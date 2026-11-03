/* log, log10, log2 and their float and long double entry points against the
   correctly rounded result.

   This replaces log_test(), log10_test() and log2_test() in libm-test.inc,
   which measured the same nine functions against glibc's ulp tables at about
   ten named points each.  See ulp-check.h.  */

#include "ulp-check.h"
#include "log-ref.h"

/* Measured over the 999 points of the table: fdlibm misses the correctly
   rounded result 26 times for log, 199 for log10 and 187 for log2, CORE-MATH
   none.  log10 and log2 are so much worse than log because that is all they
   are: e_log10.c and e_log2.c compute the same series and then divide by ln10
   or ln2, and the division rounds once more.

   There is no optimized or accurate variant yet.  Arm's optimized-routines does
   carry portable scalar log and log2, so the optimized column is where they
   would go; until then all three settings are the same code and share one
   number.

   These budgets come from a target that rounds every operation to double.  On
   i686 the x87 computes with 64 mantissa bits and does better -- 1, 116 and 123
   -- so unlike sin, cos and tan these three want no help from
   x87-precision.h: they do no compensated reduction that the extra width could
   break, it simply gives them room.  */
#define LOG_BUDGET	BUDGET(30, 30, 30)
#define LOG10_BUDGET	BUDGET(210, 210, 210)
#define LOG2_BUDGET	BUDGET(198, 198, 198)

/* logf, log10f and log2f compute in double and convert
   (libm/float_wrappers.c), which leaves the double error far below the float
   rounding boundary.  Measured: every setting hits every float point.  */
#define F_BUDGET	0

/* How far the worst point may be, and this is a requirement rather than an
   observation.  It follows the implementation, not the setting -- and there is
   no variant of this function, so all three settings run the same fdlibm code
   and the accurate column is one and not nought.  fdlibm's own claim is an
   error below one ulp, so the neighbouring representable value is as far as it
   may land.  */
#define LOG_STEPS	BUDGET(1, 1, 1)
#define LOG10_STEPS	BUDGET(1, 1, 1)
#define LOG2_STEPS	BUDGET(1, 1, 1)

/* The cases log_test(), log10_test() and log2_test() checked.  All three are
   -inf at zero from either side, NaN for a negative argument, and 0 at 1.  */
#define SPECIALS(fn, expect, type)					   \
do {									   \
	fails += ULP_EXPECT_EXC(expect, #fn "(0)", fn((type)0), -(type)INFINITY, ULP_DIVBYZERO);	   \
	fails += ULP_EXPECT_EXC(expect, #fn "(-0)", fn(-(type)0), -(type)INFINITY, ULP_DIVBYZERO);	   \
	fails += expect(#fn "(1)", fn((type)1), (type)0);		   \
	fails += ULP_EXPECT_EXC(expect, #fn "(-1)", fn(-(type)1), (type)NAN, ULP_INVALID);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)INFINITY);  \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
} while (0)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("log", log, log_ref, NELEM(log_ref),
			      LOG_BUDGET, LOG_STEPS);
	fails += ulp_sweep1_f("logf", logf, logf_ref, NELEM(logf_ref),
			      F_BUDGET, LOG_STEPS);
	fails += ulp_sweep1_l("logl", logl, log_ref, NELEM(log_ref),
			      LOG_BUDGET, LOG_STEPS);

	fails += ulp_sweep1_d("log10", log10, log10_ref, NELEM(log10_ref),
			      LOG10_BUDGET, LOG10_STEPS);
	fails += ulp_sweep1_f("log10f", log10f, log10f_ref, NELEM(log10f_ref),
			      F_BUDGET, LOG10_STEPS);
	fails += ulp_sweep1_l("log10l", log10l, log10_ref, NELEM(log10_ref),
			      LOG10_BUDGET, LOG10_STEPS);

	fails += ulp_sweep1_d("log2", log2, log2_ref, NELEM(log2_ref),
			      LOG2_BUDGET, LOG2_STEPS);
	fails += ulp_sweep1_f("log2f", log2f, log2f_ref, NELEM(log2f_ref),
			      F_BUDGET, LOG2_STEPS);
	fails += ulp_sweep1_l("log2l", log2l, log2_ref, NELEM(log2_ref),
			      LOG2_BUDGET, LOG2_STEPS);

	SPECIALS(log, ulp_expect_d, double);
	SPECIALS(logf, ulp_expect_f, float);
	SPECIALS(logl, ulp_expect_l, long double);

	SPECIALS(log10, ulp_expect_d, double);
	SPECIALS(log10f, ulp_expect_f, float);
	SPECIALS(log10l, ulp_expect_l, long double);

	SPECIALS(log2, ulp_expect_d, double);
	SPECIALS(log2f, ulp_expect_f, float);
	SPECIALS(log2l, ulp_expect_l, long double);

	/* The exact points those tests also checked. */
	fails += ulp_expect_d("log10(10)", log10(10.0), 1.0);
	fails += ulp_expect_d("log10(100)", log10(100.0), 2.0);
	fails += ulp_expect_d("log10(10000)", log10(10000.0), 4.0);
	fails += ulp_expect_d("log2(2)", log2(2.0), 1.0);
	fails += ulp_expect_d("log2(16)", log2(16.0), 4.0);
	fails += ulp_expect_d("log2(256)", log2(256.0), 8.0);

	ulp_report_skipped();

	return fails != 0;
}
