/* sched_rr_get_interval returns the round-robin time slice.  Under SCHED_RR
   the kernel reports a nonzero slice (100 ms by default), and on a 32-bit
   target with a 64-bit time_t the library has to convert the kernel's
   64-bit tv_nsec into the program's timespec.  Read into a buffer filled
   with a pattern, so a field the library never writes shows up.  Changing
   the policy needs privileges; without them the test is skipped.  */

#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(void)
{
	struct sched_param sp = { .sched_priority = 1 };
	struct timespec ts;

	if (sched_setscheduler(0, SCHED_RR, &sp) != 0) {
		puts("SKIP: cannot switch to SCHED_RR");
		return 23;
	}

	memset(&ts, 0xa5, sizeof ts);
	if (sched_rr_get_interval(0, &ts) != 0) {
		perror("sched_rr_get_interval");
		return 1;
	}

	/* Any slice between one nanosecond and ten seconds is plausible.  */
	if (ts.tv_nsec < 0 || ts.tv_nsec > 999999999 || ts.tv_sec < 0
	    || ts.tv_sec > 10 || (ts.tv_sec == 0 && ts.tv_nsec == 0)) {
		printf("SCHED_RR slice %lld.%09lld, want between 0 and 10 s\n",
		       (long long)ts.tv_sec, (long long)ts.tv_nsec);
		return 1;
	}
	return 0;
}
