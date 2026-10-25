/* Testsuite runner.
 *
 * Does what uclibcng-testrunner.sh does, reading the same
 * uclibcng-testrunner.in.  Without options the output is that of the script,
 * plus one new line per test:
 *
 *   TIME <name> <milliseconds>
 *
 * -r prints a "RUN name" line before each test instead of the overwriting
 * ".... name" progress, which reads better in a serial log; -c colours the
 * result.  A test that passes but printed a SKIP line has that line shown.
 *
 * The reason for C is not elegance.  The shell version spends up to three
 * busybox execs per test -- the subshell around the test, the "env" prefix on
 * 533 of 841 lines, and "expr" for the counters -- and on a noMMU target every
 * exec re-loads busybox, some 1.1 MB, which the allocator rounds up to 2 MB and
 * cannot reuse.  That is what made riscv32 noMMU die with signal 11 partway
 * through a run, from test 132 onwards, whatever the test was.  Here a test
 * costs one vfork and one exec of the test binary itself.
 *
 * vfork, not fork: fork() does not exist without an MMU.  Linux gives the vfork
 * child its own file descriptors and its own working directory (no CLONE_FILES,
 * no CLONE_FS), so chdir and dup2 in the child are safe -- but its memory is
 * the parent's, so argv and envp are built before the vfork and the child does
 * nothing but syscalls.
 *
 * Started as PID 1 it is the system's init: it mounts what the tests need,
 * runs the suite with -r, prints the tests_start and tests_end markers and
 * then starts a shell.  See the end of the file.
 *
 * Copyright (c) 2015 Thorsten "mirabilos" Glaser <tg@mirbsd.org> for the shell
 * version this follows.
 *
 * Provided that these terms and disclaimer and all copyright notices are
 * retained or reproduced in an accompanying document, permission is granted to
 * deal in this work without restriction, including unlimited rights to use,
 * publicly perform, distribute, sell, modify, merge, give away, or sublicence.
 *
 * This work is provided "AS IS" and WITHOUT WARRANTY of any kind.
 */

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/ioctl.h>

#define MAXLINE   8192
#define MAXWORDS  64
#define MAXENV    16
#define MAXPATHL  1024

/* The buffers sized by these are static, not on the stack: a FLAT binary has
   a fixed stack with no guard page, OpenADK gives it 8 KB on lm32, and
   run_suite() alone used to need 29 KB of it.  The runner has one thread and
   no recursion, so nothing needs them to be local. */

static int opt_color;		/* -c: ANSI colours and a padded subdir column */
static int opt_runlines;	/* -r: "RUN name" instead of the \r progress   */
static int is_init;		/* running as PID 1, see init_setup()         */

static int nfail, nskip, npass;
static int kmsg_fd = -1;	/* /dev/kmsg as init, see kernel_log() */

/* ------------------------------------------------------------------ output */

/* The scripts padded the subdir to 15 columns with printf '%*s'; a negative
   width does the same and needs no second pass. */
static void
result(const char *what, const char *colour, const char *subdir,
       const char *name, const char *tail)
{
	if (opt_color)
		printf("\r\033[01;%sm%s\033[00m %-15s %s%s\n",
		       colour, what, subdir, name, tail);
	else
		printf("%s %s%s\n", what, name, tail);
}

/* Print a file with every line indented, the way "sed 's/^/\t/'" did.  Used
   for the output of a test that failed or skipped; silent if it is not there. */
static void
cat_indented(const char *path, const char *indent)
{
	FILE *f = fopen(path, "r");
	int c, bol = 1;

	if (!f)
		return;
	while ((c = getc(f)) != EOF) {
		if (bol) {
			fputs(indent, stdout);
			bol = 0;
		}
		putchar(c);
		if (c == '\n')
			bol = 1;
	}
	if (!bol)
		putchar('\n');
	fclose(f);
}

/* A test that passed may still have said it could not check something.  Its
   output goes nowhere otherwise -- only the failure paths print it -- so the
   one kind of line that has to survive a pass is surfaced: those starting
   with SKIP or containing "skipped". */
static void
surface_skips(const char *path)
{
	static char buf[MAXLINE];
	FILE *f = fopen(path, "r");

	if (!f)
		return;
	while (fgets(buf, sizeof buf, f)) {
		if (strncmp(buf, "SKIP", 4) == 0 || strstr(buf, "skipped")) {
			size_t n = strlen(buf);
			fputs("\t", stdout);
			fputs(buf, stdout);
			if (n == 0 || buf[n - 1] != '\n')
				putchar('\n');
		}
	}
	fclose(f);
}

/* What the kernel logged while a test ran -- above all the register dump
   print-fatal-signals adds when one dies of a signal.  On the console it
   came out in the middle of the runner's lines, tearing a marker or a
   result apart, so as init the console stays quiet and the messages are
   read from /dev/kmsg after each test instead: printed under a failure,
   dropped otherwise.  A record is "prio,seq,usec,flags;text"; one read
   returns one record, EAGAIN means there is no more. */
static void
kernel_log(int show)
{
	static char buf[8192];
	char *text;
	ssize_t n;

	if (kmsg_fd < 0)
		return;
	for (;;) {
		n = read(kmsg_fd, buf, sizeof buf - 1);
		if (n < 0) {
			if (errno == EPIPE)	/* overwritten, go on */
				continue;
			break;
		}
		if (n == 0)
			break;
		buf[n] = '\0';
		if (!show)
			continue;
		text = strchr(buf, ';');
		text = text ? text + 1 : buf;
		text[strcspn(text, "\n")] = '\0';	/* drop the key=value lines */
		printf("\tkernel: %s\n", text);
	}
}

/* ------------------------------------------------------------- the command */

/* The cmd field of uclibcng-testrunner.in needs no shell.  It is ./prog,
   maybe with arguments, behind the WRAPPER of its Makefile.in: an
   "env VAR=value" prefix (once env -i), a stdin redirect with <, or, in
   assert/, 'trap ":" ABRT ;'.  The only expansion is $PWD, from dlopen/, and
   the only quoting is a double quoted value.  So: honour double quotes,
   expand $PWD, and understand env, < and the trap prefix.
 *
 * Splits s into words in buf, writing pointers into w.  Not in place: $PWD is
 * four characters standing for a whole path, so the result is longer than the
 * input and would overwrite what has not been read yet.  Returns the count, or
 * -1 if it does not fit. */
static int
split(const char *s, char **w, int max, char *buf, size_t bufsz,
      const char *pwd)
{
	int n = 0, quoted;
	char *out = buf;
	char *end = buf + bufsz - 1;
	size_t pwdlen = strlen(pwd);

	while (*s) {
		while (*s == ' ' || *s == '\t')
			s++;
		if (!*s)
			break;
		if (n >= max)
			return -1;
		w[n++] = out;
		quoted = 0;
		/* The quote state has to toggle anywhere in the word, not just
		   at its start: the one quoted value in the file is
		   LD_LIBRARY_PATH="$PWD:.:", where the quote sits behind the
		   equals sign.  Treating it as a leading quote only splits the
		   word and the program name ends up wrong. */
		while (*s && (quoted || (*s != ' ' && *s != '\t'))) {
			if (*s == '"') {
				quoted = !quoted;
				s++;
			} else if (strncmp(s, "$PWD", 4) == 0) {
				/* expands inside quotes too, as in the shell */
				if (out + pwdlen >= end)
					return -1;
				memcpy(out, pwd, pwdlen);
				out += pwdlen;
				s += 4;
			} else {
				if (out >= end)
					return -1;
				*out++ = *s++;
			}
		}
		if (out >= end)
			return -1;
		*out++ = '\0';
	}
	return n;
}

/* ------------------------------------------------------------------- a run */

/* Runs one test and returns the status the shell would have reported in $?,
   that is 128 + signal when it died of one.  -1 means it could not be run. */
static int
run_one(char **w, int nw, const char *outfile, char **environ_in)
{
	char *argv[MAXWORDS];
	char *saved[MAXENV];
	/* volatile: the vfork child runs in this frame and reads it, so the
	   compiler must not keep it somewhere the child's execution clobbers.
	   gcc warns about exactly this without it. */
	const char *volatile redirect = NULL;
	int i = 0, na = 0, ne = 0, ns = 0, clear_env = 0;
	pid_t pid;
	int status;

	/* the trap prefix: 'trap ":" ABRT ;' -- the shell used it to keep quiet
	   about a test that aborts on purpose, and there is nothing here that
	   would announce it, so it just goes */
	if (i < nw && strcmp(w[i], "trap") == 0) {
		while (i < nw && strcmp(w[i], ";") != 0)
			i++;
		if (i < nw)
			i++;
	}

	if (i < nw && strcmp(w[i], "env") == 0) {
		i++;
		if (i < nw && strcmp(w[i], "-i") == 0) {
			clear_env = 1;
			i++;
		}
		while (i < nw && strchr(w[i], '=') != NULL) {
			if (ns >= MAXENV)
				return -1;
			saved[ns++] = w[i++];
		}
	}

	for (; i < nw; i++) {
		if (strcmp(w[i], "<") == 0 && i + 1 < nw) {
			redirect = w[++i];
			continue;
		}
		if (na >= MAXWORDS - 1)
			return -1;
		argv[na++] = w[i];
	}
	argv[na] = NULL;
	if (na == 0)
		return -1;

	/* Build the environment before the vfork: the child shares this memory
	   and may not write to it.  setenv() would be simpler but it edits the
	   parent's environment, and these assignments are meant for one test
	   only -- LD_LIBRARY_PATH above all, which must not leak into the next
	   one.  The array grows as needed and is kept for the next call; there
	   is one of these per test, so it settles after the first. */
	{
		static char **envbuf;
		static int envcap;
		int nold = 0, need;

		if (!clear_env)
			while (environ_in[nold])
				nold++;
		need = ns + nold + 1;
		if (need > envcap) {
			char **p = realloc(envbuf, need * sizeof *p);
			if (!p)
				return -1;
			envbuf = p;
			envcap = need;
		}
		for (i = 0; i < ns; i++)
			envbuf[ne++] = saved[i];
		for (i = 0; i < nold; i++)
			envbuf[ne++] = environ_in[i];
		envbuf[ne] = NULL;

		pid = vfork();
		if (pid == 0) {
			int fd;

			/* Syscalls only from here: this is the parent's
			   address space. */
			fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd < 0)
				_exit(127);
			dup2(fd, 1);
			dup2(fd, 2);
			if (fd > 2)
				close(fd);
			fd = open(redirect ? redirect : "/dev/null", O_RDONLY);
			if (fd < 0)
				_exit(127);
			dup2(fd, 0);
			if (fd > 2)
				close(fd);
			execve(argv[0], argv, envbuf);
			_exit(127);
		}
	}
	if (pid < 0)
		return -1;
	/* As init, every orphan of a test is ours to reap; collect them along
	   with the test so they do not pile up as zombies over the run. */
	for (;;) {
		pid_t got = waitpid(is_init ? -1 : pid, &status, 0);
		if (got == pid)
			break;
		if (got < 0 && errno != EINTR)
			return -1;
	}
	if (WIFSIGNALED(status))
		return 128 + WTERMSIG(status);
	return WEXITSTATUS(status);
}

/* --------------------------------------------------------------- .good file */

/* Byte comparison; the scripts used diff -u and only looked at whether it
   reported a difference.

   Only <binary_name>.out.good is consulted.  The script also named a second
   candidate, "$test_src_name.out", but the variable it read is tst_src_name --
   the misspelt one is always empty, so that candidate never existed.  Kept as
   it was: this is a rewrite, not a change of what gets compared. */
static int
same_file(const char *a, const char *b)
{
	FILE *fa = fopen(a, "r"), *fb = fopen(b, "r");
	int ca, cb, eq = 1;

	if (!fa || !fb) {
		if (fa) fclose(fa);
		if (fb) fclose(fb);
		return 0;
	}
	do {
		ca = getc(fa);
		cb = getc(fb);
		if (ca != cb) {
			eq = 0;
			break;
		}
	} while (ca != EOF);
	fclose(fa);
	fclose(fb);
	return eq;
}

/* The report for a failed comparison.  Not diff -u: that needed a shell, diff
   and sed in the guest, for five .good files that are a few lines each.  The
   lines are compared pairwise, so an inserted line shows as every line after it
   differing -- the limit keeps that readable, and the first difference is the
   one that matters. */
#define MAXDIFF 20

static void
show_diff(const char *out, const char *good)
{
	static char lo[MAXLINE], lg[MAXLINE];
	FILE *fo = fopen(out, "r"), *fg = fopen(good, "r");
	int n = 0, shown = 0;

	if (!fo || !fg) {
		printf("       cannot open %s\n", fo ? good : out);
		goto out;
	}
	printf("       --- expected %s\n       +++ got      %s\n", good, out);
	for (;;) {
		char *po = fgets(lo, sizeof lo, fo);
		char *pg = fgets(lg, sizeof lg, fg);

		if (!po && !pg)
			break;
		n++;
		if (po && pg && strcmp(lo, lg) == 0)
			continue;
		if (++shown > MAXDIFF) {
			puts("       ...");
			break;
		}
		printf("       @ line %d\n", n);
		if (pg)
			printf("       -%.*s\n", (int) strcspn(lg, "\n"), lg);
		if (po)
			printf("       +%.*s\n", (int) strcspn(lo, "\n"), lo);
	}
out:
	if (fo) fclose(fo);
	if (fg) fclose(fg);
}

/* ------------------------------------------------------------------- main */

extern char **environ;

static int
run_suite(const char *infile)
{
	static char line[MAXLINE], cwd[MAXPATHL], subcwd[MAXPATHL];
	static char wordbuf[MAXLINE * 2];	/* $PWD makes the result longer */
	static char outfile[MAXPATHL], goodfile[MAXPATHL], outpath[MAXPATHL];
	static char gpath[MAXPATHL];
	char *w[MAXWORDS];
	FILE *in;

	if (!getcwd(cwd, sizeof cwd)) {
		fprintf(stderr, "E: getcwd: %s\n", strerror(errno));
		return -1;
	}
	if ((in = fopen(infile, "r")) == NULL) {
		fprintf(stderr, "E: %s not found\n", infile);
		return -1;
	}

	while (fgets(line, sizeof line, in)) {
		char *p = line, *expected_s, *binary, *subdir, *rest;
		int expected, ret, nw;
		struct timespec t0, t1;
		long ms;

		p[strcspn(p, "\r\n")] = '\0';

		/* expected_ret tst_src_name binary_name subdir cmd... */
		expected_s = strtok(p, " \t");
		if (!expected_s)
			continue;
		(void) strtok(NULL, " \t");		/* tst_src_name, unused */
		binary = strtok(NULL, " \t");
		subdir = strtok(NULL, " \t");
		if (!binary || !subdir)
			continue;
		rest = strtok(NULL, "");
		if (!rest)
			continue;
		expected = atoi(expected_s);

		if (opt_runlines)
			printf("RUN %s\n", binary);
		else
			printf(".... %s\r", binary);
		fflush(stdout);

		if (chdir(subdir) != 0) {
			result("FAIL", "31", subdir, binary, " cannot chdir");
			nfail++;
			kernel_log(1);
			continue;
		}
		if (!getcwd(subcwd, sizeof subcwd))
			subcwd[0] = '\0';

		snprintf(outfile, sizeof outfile, "%s.out", binary);
		nw = split(rest, w, MAXWORDS, wordbuf, sizeof wordbuf, subcwd);

		/* CLOCK_MONOTONIC, not the wall clock: some tests set
		   the system time */
		clock_gettime(CLOCK_MONOTONIC, &t0);
		ret = nw < 0 ? -1 : run_one(w, nw, outfile, environ);
		clock_gettime(CLOCK_MONOTONIC, &t1);
		ms = (t1.tv_sec - t0.tv_sec) * 1000L
		   + (t1.tv_nsec - t0.tv_nsec) / 1000000L;

		snprintf(outpath, sizeof outpath, "%s/%s.out", subdir, binary);
		snprintf(goodfile, sizeof goodfile, "%s.out.good", binary);

		if (ret == 23) {
			if (chdir(cwd) != 0) return -1;
			result("SKIP", "33", subdir, binary, "");
			printf("TIME %s %ld\n", binary, ms);
			nskip++;
			cat_indented(outpath, "\t");
			kernel_log(0);
			continue;
		}
		if (ret != expected) {
			char tail[64];
			snprintf(tail, sizeof tail, " got %d expected %d",
				 ret, expected);
			if (chdir(cwd) != 0) return -1;
			result("FAIL", "31", subdir, binary, tail);
			printf("TIME %s %ld\n", binary, ms);
			nfail++;
			cat_indented(outpath, "\t");
			kernel_log(1);
			continue;
		}

		if (access(goodfile, F_OK) == 0) {
			int ok = same_file(outfile, goodfile);

			snprintf(gpath, sizeof gpath, "%s/%s.out.good",
				 subdir, binary);
			if (chdir(cwd) != 0) return -1;
			if (ok) {
				result("PASS", "32", subdir, binary, "");
				printf("TIME %s %ld\n", binary, ms);
				npass++;
				kernel_log(0);
			} else {
				result("FAIL", "31", subdir, binary,
				       " expected output differs");
				printf("TIME %s %ld\n", binary, ms);
				nfail++;
				show_diff(outpath, gpath);
				kernel_log(1);
			}
			continue;
		}

		if (chdir(cwd) != 0) return -1;
		result("PASS", "32", subdir, binary, "");
		printf("TIME %s %ld\n", binary, ms);
		npass++;
		surface_skips(outpath);
		kernel_log(0);
	}
	fclose(in);

	if (opt_color) {
		printf("Total skipped :\033[01;33m %d \033[00m\n", nskip);
		printf("Total failed  :\033[01;31m %d \033[00m\n", nfail);
		printf("Total passed  :\033[01;32m %d \033[00m\n", npass);
	} else {
		printf("Total skipped: %d\n", nskip);
		printf("Total failed: %d\n", nfail);
		printf("Total passed: %d\n", npass);
	}
	return nfail == 0 ? 0 : 1;	/* -1 above: the run did not finish */
}

/* ------------------------------------------------------------------- init */

/* Booted as /init, the runner does what busybox init and run_tests.sh did
   around it, so a test run execs nothing but the tests: on a noMMU target
   each busybox exec is another 1.1 MB the allocator rounds up to 2 MB.
   Busybox stays in the rootfs as /bin/sh for popen() and posix_spawn tests,
   and as the shell after the run.  Every step may fail -- a kernel
   without tmpfs, a rootfs that is ramfs -- and none of that stops the run. */

#define TESTDIR "/usr/lib/uclibc-ng-test/test"

static void
write_file(const char *path, const char *s)
{
	int fd = open(path, O_WRONLY);

	if (fd >= 0) {
		ssize_t r = write(fd, s, strlen(s));
		(void) r;
		close(fd);
	}
}

static void
init_setup(void)
{
	int fd;

	mount("devtmpfs", "/dev", "devtmpfs", 0, NULL);

	/* The kernel opens /dev/console for init only if the initramfs has
	   the node, and ours gets it from devtmpfs just now.  Opened here in
	   any case, in a session of its own, so the tests have the console as
	   controlling terminal the way busybox init gave it to run_tests.sh. */
	setsid();
	fd = open("/dev/console", O_RDWR);
	if (fd >= 0) {
		ioctl(fd, TIOCSCTTY, 0);
		dup2(fd, 0);
		dup2(fd, 1);
		dup2(fd, 2);
		if (fd > 2)
			close(fd);
	}

	mount("proc", "/proc", "proc", 0, NULL);
	mount("sysfs", "/sys", "sysfs", 0, NULL);
	/* the initramfs has no fallocate, tst-fallocate needs a tmpfs */
	mount("tmpfs", "/tmp", "tmpfs", 0, NULL);
	/* shm_open() keeps its objects here, devtmpfs does not have it */
	mkdir("/dev/shm", 01777);
	mount("tmpfs", "/dev/shm", "tmpfs", 0, NULL);
	/* the slaves /dev/ptmx hands out (ptytest) */
	mkdir("/dev/pts", 0755);
	mount("devpts", "/dev/pts", "devpts", 0, NULL);
	/* the rootfs tmpfs defaults to half the RAM; on sh4 that left 1.3 MB
	   for all .out files */
	mount("rootfs", "/", NULL, MS_REMOUNT, "size=90%");

	/* register dump on fatal signals; it is read from /dev/kmsg, the
	   console stays quiet during the run (see kernel_log()) */
	write_file("/proc/sys/kernel/print-fatal-signals", "1");
	write_file("/proc/sys/kernel/printk", "1");
	kmsg_fd = open("/dev/kmsg", O_RDONLY | O_NONBLOCK);
	if (kmsg_fd >= 0)
		lseek(kmsg_fd, 0, SEEK_END);	/* not the boot log */

	/* what busybox init exported */
	setenv("HOME", "/", 1);
	setenv("PATH", "/sbin:/usr/sbin:/bin:/usr/bin", 1);
	setenv("SHELL", "/bin/sh", 1);
	setenv("USER", "root", 1);
}

/* Markers for whatever watches the console, so it can tell a finished run
   from one that broke off. */
static void
banner(const char *text)
{
	printf("-------------------- %s %.*s\n", text,
	       (int) (35 - strlen(text)), "-----------------------------------");
}

/* init must not exit, the kernel panics.  After the run: a shell on the
   console, no login.  Without one, switch off. */
static void
init_finish(void)
{
	char *argv[] = { "sh", NULL };

	/* for the interactive look, the kernel may talk again */
	write_file("/proc/sys/kernel/printk", "8");
	puts("\nshell on the console; rerun with ./uclibcng-testrunner -c,"
	     " quit with reboot -f\n");
	fflush(stdout);
	sync();
	execve("/bin/sh", argv, environ);
	reboot(RB_POWER_OFF);
	for (;;)
		pause();
}

int
main(int argc, char **argv)
{
	const char *infile = "uclibcng-testrunner.in";
	int i;

	is_init = getpid() == 1;

	if (is_init) {
		init_setup();
		if (chdir(TESTDIR) != 0)
			fprintf(stderr, "E: chdir %s: %s\n", TESTDIR,
				strerror(errno));
		/* a fixed run: "RUN name" lines and no colour; options
		   are for starting it by hand */
		opt_runlines = 1;
		banner("tests_start");
		/* no tests_end after a run that broke off, so the harness
		   reports it as one instead of as an empty pass */
		if (run_suite(infile) >= 0)
			banner("tests_end");
		init_finish();
	}

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-c") == 0)
			opt_color = 1;
		else if (strcmp(argv[i], "-r") == 0)
			opt_runlines = 1;
		else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
			infile = argv[++i];
		else {
			fprintf(stderr, "usage: %s [-c] [-r] [-f file]\n"
				"  -c  colour the result and pad the subdir column\n"
				"  -r  print \"RUN name\" instead of overwriting progress\n",
				argv[0]);
			return 1;
		}
	}
	return run_suite(infile) == 0 ? 0 : 1;
}
