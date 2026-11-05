/* A null output pointer fails with EFAULT, as the Linux manual pages
   document for these calls.  POSIX leaves the behaviour undefined; the
   point here is that a library which converts between its own time
   structures and the kernel's (64-bit time_t on a 32-bit target) keeps the
   system call's behaviour instead of writing the result through the null
   pointer itself.  Each call runs in a child, so a crash counts as a
   failure instead of ending the test.

   Without an MMU the kernel cannot check a user pointer: it writes through
   the null pointer itself and panics.  There is no EFAULT to expect, so
   the test is skipped there.  */

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <sys/wait.h>

enum { GETITIMER, TIMER_GETTIME, TIMERFD_GETTIME, SCHED_RR_GET_INTERVAL };

static const char *const names[] = {
	"getitimer", "timer_gettime", "timerfd_gettime", "sched_rr_get_interval"
};

/* Child: arm whatever the call reads, so that a library which writes its
   result itself would have something to write, then call with NULL.  */
static int child(int which)
{
	struct itimerspec its = { { 1, 0 }, { 60, 0 } };
	struct itimerval itv = { { 1, 0 }, { 60, 0 } };
	struct sigevent ev;
	sigset_t block;
	timer_t t;
	int fd, r = 0;

	errno = 0;
	switch (which) {
	case GETITIMER:
		setitimer(ITIMER_VIRTUAL, &itv, NULL);
		r = getitimer(ITIMER_VIRTUAL, NULL);
		break;
	case TIMER_GETTIME:
		sigemptyset(&block);
		sigaddset(&block, SIGRTMIN);
		sigprocmask(SIG_BLOCK, &block, NULL);
		memset(&ev, 0, sizeof ev);
		ev.sigev_notify = SIGEV_SIGNAL;
		ev.sigev_signo = SIGRTMIN;
		if (timer_create(CLOCK_MONOTONIC, &ev, &t) != 0)
			return 2;
		timer_settime(t, 0, &its, NULL);
		r = timer_gettime(t, NULL);
		break;
	case TIMERFD_GETTIME:
		fd = timerfd_create(CLOCK_MONOTONIC, 0);
		if (fd < 0)
			return 2;
		timerfd_settime(fd, 0, &its, NULL);
		r = timerfd_gettime(fd, NULL);
		break;
	case SCHED_RR_GET_INTERVAL:
		r = sched_rr_get_interval(0, NULL);
		break;
	}
	return r == -1 && errno == EFAULT ? 0 : 1;
}

int main(void)
{
	int which, err = 0;

#if defined __UCLIBC__ && !defined __ARCH_USE_MMU__
	(void) which;
	(void) child;
	(void) err;
	(void) names;
	puts("SKIP: no MMU, the kernel cannot report EFAULT");
	return 23;
#else
	for (which = 0; which < 4; which++) {
		int status;
		pid_t pid = fork();

		if (pid < 0) {
			perror("fork");
			return 1;
		}
		if (pid == 0)
			_exit(child(which));
		if (waitpid(pid, &status, 0) != pid) {
			perror("waitpid");
			return 1;
		}
		if (WIFSIGNALED(status)) {
			printf("%s(NULL): killed by signal %d, want -1/EFAULT\n",
			       names[which], WTERMSIG(status));
			err = 1;
		} else if (WEXITSTATUS(status) == 2) {
			printf("%s: setup failed\n", names[which]);
			err = 1;
		} else if (WEXITSTATUS(status) != 0) {
			printf("%s(NULL): did not fail with EFAULT\n", names[which]);
			err = 1;
		}
	}
	return err;
#endif
}
