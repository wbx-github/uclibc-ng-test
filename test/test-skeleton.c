/* Skeleton for test programs.
   Copyright (C) 1998,2000-2004, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <malloc.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <time.h>
#include <features.h>
#include "test-skeleton.h"

	 
/* The test function is normally called `do_test' and it is called
   with argc and argv as the arguments.  We nevertheless provide the
   possibility to overwrite this name.  */
#ifndef TEST_FUNCTION
# define TEST_FUNCTION do_test (argc, argv)
#endif

#ifndef TEST_DATA_LIMIT
# define TEST_DATA_LIMIT (64 << 20) /* Data limit (bytes) to run with.  */
#endif

/* PID of the test itself.  */
static pid_t pid;

/* Directory to place temporary files in.  */
static const char *test_dir;

/* List of temporary files.  */
struct temp_name_list
{
  struct qelem q;
  char *name;
} *temp_name_list;

/* Add temporary files in list.  */
static void
__attribute__ ((unused))
add_temp_file (const char *name)
{
  struct temp_name_list *newp
    = (struct temp_name_list *) calloc (sizeof (*newp), 1);
  char *newname = strdup (name);
  if (newp != NULL && newname != NULL)
    {
      newp->name = newname;
      if (temp_name_list == NULL)
	temp_name_list = (struct temp_name_list *) &newp->q;
      else
	insque (newp, temp_name_list);
    }
  else
    free (newp);
}

/* Delete all temporary files.  */
static void
delete_temp_files (void)
{
  while (temp_name_list != NULL)
    {
      remove (temp_name_list->name);
      free (temp_name_list->name);

      struct temp_name_list *next
	= (struct temp_name_list *) temp_name_list->q.q_forw;
      free (temp_name_list);
      temp_name_list = next;
    }
}

/* Create a temporary file.  Return the opened file descriptor on
   success, or -1 on failure.  Write the file name to *FILENAME if
   FILENAME is not NULL.  In this case, the caller is expected to free
   *FILENAME.  */
static int
__attribute__ ((unused))
create_temp_file (const char *base, char **filename)
{
  char *fname;
  int _fd;

  fname = (char *) malloc (strlen (test_dir) + 1 + strlen (base)
			   + sizeof ("XXXXXX"));
  if (fname == NULL)
    {
      puts ("out of memory");
      return -1;
    }
  strcpy (stpcpy (stpcpy (stpcpy (fname, test_dir), "/"), base), "XXXXXX");

  _fd = mkstemp (fname);
  if (_fd == -1)
    {
      printf ("cannot open temporary file '%s': %s\n", fname, strerror(errno));
      free (fname);
      return -1;
    }

  add_temp_file (fname);
  if (filename != NULL)
    *filename = fname;
  else
    free (fname);

  return _fd;
}

/* Timeout handler.  We kill the child and exit with an error.  */
static void
__attribute__ ((noreturn))
signal_handler (int sig __attribute__ ((unused)))
{
  int killed = 0;
  int status;
  int i;

  assert (pid > 1);
  /* Kill the whole process group.  */
  kill (-pid, SIGKILL);
  /* In case setpgid failed in the child, kill it individually too.  */
  kill (pid, SIGKILL);

  /* Wait for it to terminate.  */
  for (i = 0; i < 5; ++i)
    {
#ifdef __UCLIBC_HAS_REALTIME__
      struct timespec ts;
#endif
      killed = waitpid (pid, &status, WNOHANG|WUNTRACED);
      if (killed != 0)
	break;

      /* Delay, give the system time to process the kill.  If the
	 nanosleep() call return prematurely, all the better.  We
	 won't restart it since this probably means the child process
	 finally died.  */
#ifdef __UCLIBC_HAS_REALTIME__
      ts.tv_sec = 0;
      ts.tv_nsec = 100000000;
      nanosleep (&ts, NULL);
#else
	  /* No nanosleep, just sleep 1s instead of 0.1s */
	  sleep(1);
#endif
    }
  if (killed != 0 && killed != pid)
    {
      perror ("Failed to kill test process");
      exit (1);
    }

#ifdef CLEANUP_HANDLER
  CLEANUP_HANDLER;
#endif

  if (sig == SIGINT)
    {
      signal (sig, SIG_DFL);
      raise (sig);
    }

  /* If we expected this signal: good!  */
#ifdef EXPECTED_SIGNAL
  if (EXPECTED_SIGNAL == SIGALRM)
    exit (0);
#endif

  if (killed == 0 || (WIFSIGNALED (status) && WTERMSIG (status) == SIGKILL))
    fputs ("Timed out: killed the child process\n", stderr);
  else if (WIFSTOPPED (status))
    fprintf (stderr, "Timed out: the child process was %s\n",
	     strsignal (WSTOPSIG (status)));
  else if (WIFSIGNALED (status))
    fprintf (stderr, "Timed out: the child process got signal %s\n",
	     strsignal (WTERMSIG (status)));
  else
    fprintf (stderr, "Timed out: killed the child process but it exited %d\n",
	     WEXITSTATUS (status));

  /* Exit with an error.  */
  exit (1);
}

#ifdef __XXX_HANDLE_CTRL_C
static void
__attribute__ ((noreturn))
handler_killpid(int sig)
{
	kill(pid, SIGKILL); /* kill test */
	signal(sig, SIG_DFL);
	raise(sig); /* kill ourself */
	_exit(128 + sig); /* paranoia */
}
#endif

/* We provide the entry point here.  */
int
main (int argc, char *argv[], char *envp[])
{
  int direct = 0;	/* Directly call the test function?  */
  int status;
  int opt;
  unsigned int timeoutfactor = 1;
  pid_t termpid;
  char *envstr_timeoutfactor;
  char **argv1;

  /* Make uses of freed and uninitialized memory known.  */
#ifdef __MALLOC_STANDARD__
#ifndef M_PERTURB
# define M_PERTURB -6
#endif
  mallopt (M_PERTURB, 42);
#endif

#ifdef STDOUT_UNBUFFERED
  setbuf (stdout, NULL);
#endif

# ifndef CMDLINE_OPTIONS
#  define CMDLINE_OPTIONS ""
# endif
  while ((opt = getopt (argc, argv, "+dt:" CMDLINE_OPTIONS)) >= 0)
    switch (opt)
      {
      case '?':
	exit (1);
      case 'd':
	direct = 1;
	break;
      case 't':
	test_dir = optarg;
	break;
#ifdef CMDLINE_PROCESS
	CMDLINE_PROCESS
#endif
      }

  /* If set, read the test TIMEOUTFACTOR value from the environment.
     This value is used to scale the default test timeout values. */
  envstr_timeoutfactor = getenv ("TIMEOUTFACTOR");
  if (envstr_timeoutfactor != NULL)
    {
      char *envstr_conv = envstr_timeoutfactor;
      unsigned long int env_fact;

      env_fact = strtoul (envstr_timeoutfactor, &envstr_conv, 0);
      if (*envstr_conv == '\0' && envstr_conv != envstr_timeoutfactor)
	timeoutfactor = MAX (env_fact, 1);
    }

  /* Set TMPDIR to specified test directory.  */
  if (test_dir != NULL)
    {
      setenv ("TMPDIR", test_dir, 1);

      if (chdir (test_dir) < 0)
	{
	  perror ("chdir");
	  exit (1);
	}
    }
  else
    {
      test_dir = getenv ("TMPDIR");
      if (test_dir == NULL || test_dir[0] == '\0')
	test_dir = "/tmp";
    }

  /* Make sure we see all message, even those on stdout.  */
  setvbuf (stdout, NULL, _IONBF, 0);

  /* make sure temporary files are deleted.  */
  atexit (delete_temp_files);

  /* If we are not expected to fork run the function immediately.  */
  if (direct)
    {
      /* Correct for the possible parameters.  */
      argv[optind - 1] = argv[0];
      argv += optind - 1;
      argc -= optind - 1;

      /* Call the initializing function, if one is available.  */
#ifdef PREPARE
      PREPARE (argc, argv);
#endif

      return TEST_FUNCTION;
    }

  /* Set up the test environment:
     - prevent core dumps
     - set up the timer
     - fork and execute the function.  */

#if defined __ARCH_USE_MMU__ || ! defined __UCLIBC__
  pid = fork ();
  if (pid == 0)
    {
      /* This is the child.  */
#ifdef RLIMIT_CORE
      /* Try to avoid dumping core.  */
      struct rlimit core_limit;
      core_limit.rlim_cur = 0;
      core_limit.rlim_max = 0;
      setrlimit (RLIMIT_CORE, &core_limit);
#endif

      /* We put the test process in its own pgrp so that if it bogusly
	 generates any job control signals, they won't hit the whole build.  */
      if (setpgid (0, 0) != 0)
	printf ("Failed to set the process group ID: %m\n");

      /* Correct for the possible parameters.  */
      argv[optind - 1] = argv[0];
      argv += optind - 1;
      argc -= optind - 1;

      /* Call the initializing function, if one is available.  */
#ifdef PREPARE
      PREPARE (argc, argv);
#endif

      /* Execute the test function and exit with the return value.   */
      exit (TEST_FUNCTION);
    }
  else if (pid < 0)
    {
      perror ("Cannot fork test program");
      exit (1);
    }
#else
  argv1 = malloc ((argc + 2) * sizeof(void *));
  argv1[0] = argv[0];
  argv1[1] = "-d";
  memcpy(argv1 + 2, argv + 1, argc * sizeof(void *));

  pid = vfork ();
  if (pid == 0)
    {
      /* This is the child.  */
      /* We put the test process in its own pgrp so that if it bogusly
	 generates any job control signals, they won't hit the whole build.  */
      if (setpgid (0, 0) != 0)
	printf ("Failed to set the process group ID: %m\n");

      if (execve (argv1[0], argv1, envp) < 0)
	{
	  perror ("Cannot exec test program");
	  _exit (1);
	}
    }
  else if (pid < 0)
    {
      perror ("Cannot vfork test program");
      exit (1);
    }
#endif

#ifdef __XXX_HANDLE_CTRL_C
  signal (SIGTERM, handler_killpid);
  signal (SIGINT, handler_killpid);
  signal (SIGQUIT, handler_killpid);
#endif

  /* Set timeout.  */
#ifndef TIMEOUT
  /* Default timeout is two seconds.  */
# define TIMEOUT 2
#endif
  signal (SIGALRM, signal_handler);
  alarm (TIMEOUT * timeoutfactor);

  /* Make sure we clean up if the wrapper gets interrupted.  */
  signal (SIGINT, signal_handler);

  /* Wait for the regular termination.  */
  termpid = TEMP_FAILURE_RETRY (waitpid (pid, &status, 0));
  if (termpid == -1)
    {
      printf ("Waiting for test program failed: %s\n", strerror(errno));
      exit (1);
    }
  if (termpid != pid)
    {
      printf ("Oops, wrong test program terminated: expected %ld, got %ld\n",
	      (long int) pid, (long int) termpid);
      exit (1);
    }

#ifndef EXPECTED_SIGNAL
  /* We don't expect any signal.  */
# define EXPECTED_SIGNAL 0
#endif
  if (WTERMSIG (status) != EXPECTED_SIGNAL)
    {
      if (EXPECTED_SIGNAL != 0)
	{
	  if (WTERMSIG (status) == 0)
	    fprintf (stderr,
		     "Expected signal '%s' from child, got none\n",
		     strsignal (EXPECTED_SIGNAL));
	  else
	    fprintf (stderr,
		     "Incorrect signal from child: got `%s', need `%s'\n",
		     strsignal (WTERMSIG (status)),
		     strsignal (EXPECTED_SIGNAL));
	}
      else
	fprintf (stderr, "Didn't expect signal from child: got `%s'\n",
		 strsignal (WTERMSIG (status)));
      exit (1);
    }

  /* Simply exit with the return value of the test.  */
#ifndef EXPECTED_STATUS
  return WEXITSTATUS (status);
#else
  if (WEXITSTATUS (status) != EXPECTED_STATUS)
    {
      fprintf (stderr, "Expected status %d, got %d\n",
	       EXPECTED_STATUS, WEXITSTATUS (status));
      exit (1);
    }

  return 0;
#endif
}
