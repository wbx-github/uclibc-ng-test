/* stime sets the clock from a time_t.  The kernel's stime system call
   reads a 32-bit value, so on a 32-bit target with a 64-bit time_t the
   library has to convert or go through settimeofday; passing the program's
   time_t through reads its upper half on big-endian targets and asks for
   1970, which the kernel refuses or applies.

   The test sets the clock to what it reads anyway, checks that it did not
   jump, and puts it back from a monotonic reference either way.  A guest
   without a real-time clock starts near 1970, below its own uptime, and
   the kernel refuses any time below that; there the test sets a time just
   above the uptime instead and cannot put the clock back.  It needs the
   privilege to set the clock and skips without it.  */

#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(void)
{
	struct timespec real, mono0, mono1;
	time_t now, after;
	int r, e, err = 0, restore = 1;

	clock_gettime(CLOCK_REALTIME, &real);
	clock_gettime(CLOCK_MONOTONIC, &mono0);
	now = real.tv_sec;
	if (now < mono0.tv_sec + 2) {
		now = mono0.tv_sec + 10;
		restore = 0;
	}

	r = stime(&now);
	e = errno;
	after = time(NULL);

	if (r != 0 && e == EPERM) {
		puts("SKIP: no privilege to set the clock");
		return 23;
	}
	if (r != 0) {
		printf("stime(%lld) failed: %s\n", (long long)now, strerror(e));
		err = 1;
	} else if (after < now || after > now + 5) {
		printf("stime(%lld) left the clock at %lld\n",
		       (long long)now, (long long)after);
		err = 1;
	}

	if (!restore)
		return err;

	/* Put it back as it would be without the test.  */
	clock_gettime(CLOCK_MONOTONIC, &mono1);
	real.tv_sec += mono1.tv_sec - mono0.tv_sec;
	real.tv_nsec += mono1.tv_nsec - mono0.tv_nsec;
	if (real.tv_nsec < 0) {
		real.tv_nsec += 1000000000;
		real.tv_sec--;
	} else if (real.tv_nsec >= 1000000000) {
		real.tv_nsec -= 1000000000;
		real.tv_sec++;
	}
	if (clock_settime(CLOCK_REALTIME, &real) != 0) {
		printf("clock_settime(%lld.%09ld) to restore failed: %s\n",
		       (long long)real.tv_sec, (long)real.tv_nsec, strerror(errno));
		err = 1;
	}

	return err;
}
