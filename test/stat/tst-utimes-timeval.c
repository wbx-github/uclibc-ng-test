/* utimes, lutimes and futimesat take struct timeval, utime a
   struct utimbuf of two time_t; the kernel's old system calls expect both
   with 32-bit fields.  On a 32-bit target with a 64-bit time_t the library
   therefore has to convert, or go through utimensat.
   Each call must leave the same timestamps as utimensat does for the same
   times, so the test does not depend on what the file system can store
   (nanoseconds, dates after 2038).  */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <utime.h>

static int err;

/* Distinct seconds and microseconds for both times, the second after 2038
   where time_t is 64 bits wide.  */
static const struct timeval tv[2] = {
	{ 1000000000, 250000 },
#if __SIZEOF_TIME_T__ > 4 || defined __USE_TIME_BITS64 || defined __UCLIBC_USE_TIME64__
	{ (time_t)4102444800LL, 500000 },	/* 2100-01-01 */
#else
	{ 2000000000, 500000 },
#endif
};

static int stamps(const char *path, int follow, struct timespec out[2])
{
	struct stat st;

	if ((follow ? stat(path, &st) : lstat(path, &st)) != 0)
		return -1;
	out[0] = st.st_atim;
	out[1] = st.st_mtim;
	return 0;
}

/* What utimensat makes of TV, as the reference.  */
static int reference(const char *path, int flags, struct timespec want[2])
{
	struct timespec ts[2];

	ts[0].tv_sec = tv[0].tv_sec;
	ts[0].tv_nsec = tv[0].tv_usec * 1000;
	ts[1].tv_sec = tv[1].tv_sec;
	ts[1].tv_nsec = tv[1].tv_usec * 1000;
	if (utimensat(AT_FDCWD, path, ts, flags) != 0)
		return -1;
	return stamps(path, !(flags & AT_SYMLINK_NOFOLLOW), want);
}

static void reset(const char *path, int flags)
{
	struct timespec ts[2] = { { 1, 0 }, { 1, 0 } };

	utimensat(AT_FDCWD, path, ts, flags);
}

static void compare(const char *what, const char *path, int follow,
		    const struct timespec want[2])
{
	struct timespec got[2];
	int i;

	if (stamps(path, follow, got) != 0) {
		printf("%s: stat failed\n", what);
		err = 1;
		return;
	}
	for (i = 0; i < 2; i++)
		if (got[i].tv_sec != want[i].tv_sec || got[i].tv_nsec != want[i].tv_nsec) {
			printf("%s: %s %lld.%09ld, want %lld.%09ld\n", what,
			       i ? "mtime" : "atime", (long long)got[i].tv_sec,
			       (long)got[i].tv_nsec, (long long)want[i].tv_sec,
			       (long)want[i].tv_nsec);
			err = 1;
		}
}

int main(void)
{
	char dir[] = "/tmp/tst-utimes-XXXXXX";
	char file[64], link[64];
	struct timespec want[2], lwant[2];
	int fd;

	if (!mkdtemp(dir)) {
		perror("mkdtemp");
		return 1;
	}
	snprintf(file, sizeof file, "%s/file", dir);
	snprintf(link, sizeof link, "%s/link", dir);
	fd = open(file, O_CREAT | O_WRONLY, 0644);
	if (fd < 0 || symlink("file", link) != 0) {
		perror("setup");
		return 1;
	}
	close(fd);

	if (reference(file, 0, want) != 0 || reference(link, AT_SYMLINK_NOFOLLOW, lwant) != 0) {
		perror("utimensat");
		err = 1;
		goto out;
	}

	reset(file, 0);
	if (utimes(file, tv) != 0) {
		printf("utimes: %s\n", strerror(errno));
		err = 1;
	} else
		compare("utimes", file, 1, want);

	reset(file, 0);
	if (futimesat(AT_FDCWD, file, tv) != 0) {
		printf("futimesat: %s\n", strerror(errno));
		err = 1;
	} else
		compare("futimesat", file, 1, want);

	/* utime takes whole seconds in a struct utimbuf of two time_t.  */
	{
		struct utimbuf ub = { tv[0].tv_sec, tv[1].tv_sec };
		struct timespec swant[2] = { { tv[0].tv_sec, 0 }, { tv[1].tv_sec, 0 } };

		if (utimensat(AT_FDCWD, file, swant, 0) != 0
		    || stamps(file, 1, swant) != 0) {
			perror("utimensat (whole seconds)");
			err = 1;
		} else {
			reset(file, 0);
			if (utime(file, &ub) != 0) {
				printf("utime: %s\n", strerror(errno));
				err = 1;
			} else
				compare("utime", file, 1, swant);
		}
	}

	reset(link, AT_SYMLINK_NOFOLLOW);
	if (lutimes(link, tv) != 0) {
		printf("lutimes: %s\n", strerror(errno));
		err = 1;
	} else
		compare("lutimes", link, 0, lwant);

out:
	unlink(link);
	unlink(file);
	rmdir(dir);
	return err;
}
