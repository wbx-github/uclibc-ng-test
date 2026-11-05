/* getitimer and setitimer hand back what was set, and zeros once the timer
   is off.  The kernel has no time64 variant of either call, so on a 32-bit
   target with a 64-bit time_t the library has to convert struct itimerval
   in both directions.  Every result is read into a buffer filled with a
   pattern, so that a field the conversion never writes shows up.  */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

static int err;

static const char *const names[] = { "ITIMER_REAL", "ITIMER_VIRTUAL", "ITIMER_PROF" };

/* Is TV within LO_US..HI_US microseconds?  The seconds are checked on their
   own first, so that a field full of the fill pattern cannot overflow the
   multiplication.  */
static void check_tv(const char *what, int which, const struct timeval *tv,
		     long long lo_us, long long hi_us)
{
	long long us;

	if (tv->tv_usec < 0 || tv->tv_usec > 999999
	    || tv->tv_sec < lo_us / 1000000 || tv->tv_sec > hi_us / 1000000) {
		printf("%s %s: %lld.%06lld, want %lld.%06lld..%lld.%06lld\n",
		       names[which], what, (long long)tv->tv_sec,
		       (long long)tv->tv_usec, lo_us / 1000000, lo_us % 1000000,
		       hi_us / 1000000, hi_us % 1000000);
		err = 1;
		return;
	}
	us = (long long)tv->tv_sec * 1000000 + tv->tv_usec;
	if (us < lo_us || us > hi_us) {
		printf("%s %s: %lld.%06lld, want %lld.%06lld..%lld.%06lld\n",
		       names[which], what, (long long)tv->tv_sec,
		       (long long)tv->tv_usec, lo_us / 1000000, lo_us % 1000000,
		       hi_us / 1000000, hi_us % 1000000);
		err = 1;
	}
}

static void check_off(const char *what, int which)
{
	struct itimerval got;

	memset(&got, 0xa5, sizeof got);
	if (getitimer(which, &got) != 0) {
		printf("%s: getitimer (%s) failed\n", names[which], what);
		err = 1;
		return;
	}
	check_tv(what, which, &got.it_interval, 0, 0);
	check_tv(what, which, &got.it_value, 0, 0);
}

static void check_timer(int which)
{
	/* A minute, so that a slow emulator does not see the timer fire before
	   it is switched off again; the kernel may round the interval up to
	   its clock granularity.  */
	struct itimerval set = { { 1, 250000 }, { 60, 500000 } };
	struct itimerval zero = { { 0, 0 }, { 0, 0 } };
	struct itimerval got;

	check_off("never set", which);

	if (setitimer(which, &set, NULL) != 0) {
		printf("%s: setitimer failed\n", names[which]);
		err = 1;
		return;
	}

	memset(&got, 0xa5, sizeof got);
	if (getitimer(which, &got) != 0) {
		printf("%s: getitimer failed\n", names[which]);
		err = 1;
	} else {
		check_tv("armed interval", which, &got.it_interval, 1250000, 1300000);
		check_tv("armed value", which, &got.it_value, 1, 60550000);
	}

	/* Switching it off returns the old setting.  */
	memset(&got, 0xa5, sizeof got);
	if (setitimer(which, &zero, &got) != 0) {
		printf("%s: setitimer(0) failed\n", names[which]);
		err = 1;
	} else {
		check_tv("old interval", which, &got.it_interval, 1250000, 1300000);
		check_tv("old value", which, &got.it_value, 1, 60550000);
	}

	check_off("disarmed", which);
}

/* With a time_t wider than the kernel's 32-bit seconds, a value that does
   not fit must fail with EOVERFLOW and leave the timer alone, not wrap
   around to a short timer.  */
static void check_overflow(void)
{
	/* 2^32 + 5 seconds: wraps to 5 when cut to 32 bits.  */
	const long long wide = 0x100000005LL;
	struct itimerval zero = { { 0, 0 }, { 0, 0 } };
	struct itimerval big[2] = {
		{ { 0, 0 }, { 0, 0 } },
		{ { 0, 0 }, { 0, 0 } },
	};
	int i;

	if (sizeof(time_t) <= sizeof(long))
		return;

	big[0].it_value.tv_sec = wide;
	big[1].it_value.tv_sec = 5;
	big[1].it_interval.tv_sec = wide;

	for (i = 0; i < 2; i++) {
		errno = 0;
		if (setitimer(ITIMER_REAL, &big[i], NULL) != -1 || errno != EOVERFLOW) {
			printf("ITIMER_REAL: setitimer with %s 2^32+5 s: "
			       "got errno %d, want EOVERFLOW\n",
			       i ? "interval" : "value", errno);
			err = 1;
		}
		check_off("after overflow", ITIMER_REAL);
		/* A wrapped timer would fire within seconds; stop it.  */
		setitimer(ITIMER_REAL, &zero, NULL);
	}
}

int main(void)
{
	check_overflow();
	check_timer(ITIMER_REAL);
	check_timer(ITIMER_VIRTUAL);
	check_timer(ITIMER_PROF);
	return err;
}
