/* timerfd_settime and timerfd_gettime hand back what was set, including the
   nanoseconds, and zeros once the timer is off.  tst-timerfd only ever uses
   whole seconds, which hides a tv_nsec that the library fails to convert
   between the program's timespec and the kernel's 64-bit one.  Results are
   read into a buffer filled with a pattern, so a field the library never
   writes shows up.  */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/timerfd.h>

static int err;

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
	struct itimerspec set = { { 1, 250000000 }, { 60, 500000000 } };
	struct itimerspec zero = { { 0, 0 }, { 0, 0 } };
	struct itimerspec got;
	int fd;

	fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (fd < 0) {
		perror("timerfd_create");
		return 1;
	}

	memset(&got, 0xa5, sizeof got);
	if (timerfd_gettime(fd, &got) != 0) {
		perror("timerfd_gettime (never set)");
		err = 1;
	} else {
		check_ts("never set interval", &got.it_interval, 0, 0);
		check_ts("never set value", &got.it_value, 0, 0);
	}

	if (timerfd_settime(fd, 0, &set, NULL) != 0) {
		perror("timerfd_settime");
		return 1;
	}

	memset(&got, 0xa5, sizeof got);
	if (timerfd_gettime(fd, &got) != 0) {
		perror("timerfd_gettime");
		err = 1;
	} else {
		check_ts("armed interval", &got.it_interval, 1250000000, 1250000000);
		check_ts("armed value", &got.it_value, 1, 60500000000LL);
	}

	memset(&got, 0xa5, sizeof got);
	if (timerfd_settime(fd, 0, &zero, &got) != 0) {
		perror("timerfd_settime(0)");
		err = 1;
	} else {
		check_ts("old interval", &got.it_interval, 1250000000, 1250000000);
		check_ts("old value", &got.it_value, 1, 60500000000LL);
	}

	memset(&got, 0xa5, sizeof got);
	if (timerfd_gettime(fd, &got) != 0) {
		perror("timerfd_gettime (disarmed)");
		err = 1;
	} else {
		check_ts("disarmed interval", &got.it_interval, 0, 0);
		check_ts("disarmed value", &got.it_value, 0, 0);
	}

	close(fd);
	return err;
}
