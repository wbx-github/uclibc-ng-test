/* timer_settime and timer_gettime hand back what was set, including the
   nanoseconds, and zeros once the timer is off.  On a 32-bit target with a
   64-bit time_t the kernel's timespec has a 64-bit tv_nsec while the
   program's has a long, so the library has to convert struct itimerspec in
   both directions.  Results are read into a buffer filled with a pattern,
   so a field the library never writes shows up.  */

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static int err;

/* Is TS within LO_NS..HI_NS?  Seconds first, so the fill pattern cannot
   overflow the multiplication.  */
static void check_ts(const char *what, const struct timespec *ts,
		     long long lo_ns, long long hi_ns)
{
	if (ts->tv_nsec < 0 || ts->tv_nsec > 999999999
	    || ts->tv_sec < lo_ns / 1000000000 || ts->tv_sec > hi_ns / 1000000000
	    || (long long)ts->tv_sec * 1000000000 + ts->tv_nsec < lo_ns
	    || (long long)ts->tv_sec * 1000000000 + ts->tv_nsec > hi_ns) {
		printf("%s: %lld.%09lld, want %lld.%09lld..%lld.%09lld\n", what,
		       (long long)ts->tv_sec, (long long)ts->tv_nsec,
		       lo_ns / 1000000000, lo_ns % 1000000000,
		       hi_ns / 1000000000, hi_ns % 1000000000);
		err = 1;
	}
}

int main(void)
{
	/* A minute, so that a slow emulator does not see it expire.  */
	struct itimerspec set = { { 1, 250000000 }, { 60, 500000000 } };
	struct itimerspec zero = { { 0, 0 }, { 0, 0 } };
	struct itimerspec got;
	struct sigevent ev;
	sigset_t block;
	timer_t t;

	/* A signal timer with the signal blocked, not SIGEV_NONE: Linux 7.0
	   keeps reporting the old remaining time of a SIGEV_NONE timer after
	   it was switched off.  */
	sigemptyset(&block);
	sigaddset(&block, SIGRTMIN);
	sigprocmask(SIG_BLOCK, &block, NULL);
	memset(&ev, 0, sizeof ev);
	ev.sigev_notify = SIGEV_SIGNAL;
	ev.sigev_signo = SIGRTMIN;
	if (timer_create(CLOCK_MONOTONIC, &ev, &t) != 0) {
		perror("timer_create");
		return 1;
	}

	memset(&got, 0xa5, sizeof got);
	if (timer_gettime(t, &got) != 0) {
		perror("timer_gettime (never set)");
		err = 1;
	} else {
		check_ts("never set interval", &got.it_interval, 0, 0);
		check_ts("never set value", &got.it_value, 0, 0);
	}

	if (timer_settime(t, 0, &set, NULL) != 0) {
		perror("timer_settime");
		return 1;
	}

	memset(&got, 0xa5, sizeof got);
	if (timer_gettime(t, &got) != 0) {
		perror("timer_gettime");
		err = 1;
	} else {
		/* The interval comes back as set, the value counts down.  */
		check_ts("armed interval", &got.it_interval, 1250000000, 1250000000);
		check_ts("armed value", &got.it_value, 1, 60500000000LL);
	}

	memset(&got, 0xa5, sizeof got);
	if (timer_settime(t, 0, &zero, &got) != 0) {
		perror("timer_settime(0)");
		err = 1;
	} else {
		check_ts("old interval", &got.it_interval, 1250000000, 1250000000);
		check_ts("old value", &got.it_value, 1, 60500000000LL);
	}

	memset(&got, 0xa5, sizeof got);
	if (timer_gettime(t, &got) != 0) {
		perror("timer_gettime (disarmed)");
		err = 1;
	} else {
		check_ts("disarmed interval", &got.it_interval, 0, 0);
		check_ts("disarmed value", &got.it_value, 0, 0);
	}

	timer_delete(t);
	return err;
}
