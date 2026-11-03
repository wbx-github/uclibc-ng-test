/* expm1, expm1f and expm1l against the correctly rounded result.

   This replaces expm1_test() in libm-test.inc, which measured the same three
   functions against glibc's ulp tables: it covers the same special cases and
   1000 points instead of two values.  See ulp-check.h.  */

#include "ulp-check.h"
#include "expm1-ref.h"

/* Measured on x86-64 over these 1000 points: fdlibm misses the correctly
   rounded result 61 times, glibc's version also 61 -- and 60 of them are the
   same points, because glibc's is fdlibm with corrections that do not change
   the accuracy.  What the optimized variant buys here is 13 percent of the run
   time at identical code size, not precision; the accurate one misses none, at
   5.5 times the code and four times the run time.  One budget for both of the
   first two is therefore not sloppiness, it is the measurement.  */
#define D_BUDGET	BUDGET(65, 65, 0)

/* No variant lands further than one representable value from the reference. */
#define STEPS	BUDGET(1, 1, 0)

/* expm1f computes in double and converts (libm/float_wrappers.c).  Measured:
   every variant hits every one of the 500 float points, the double error
   staying far below the float rounding boundary, so this holds the wrapper to
   the double implementation rather than grading it.  */
#define F_BUDGET	0

/* The cases expm1_test() checked, for each of the three entry points.  Note
   expm1(-0) is -0, unlike exp(-0) == 1: the sign of zero has to survive.  */
#define SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)0);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), -(type)0);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)INFINITY);  \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), -(type)1);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
} while (0)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("expm1", expm1, expm1_ref, NELEM(expm1_ref),
			      D_BUDGET, STEPS);
	fails += ulp_sweep1_f("expm1f", expm1f, expm1f_ref,
			      NELEM(expm1f_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("expm1l", expm1l, expm1_ref, NELEM(expm1_ref),
			      D_BUDGET, STEPS);

	SPECIALS(expm1, ulp_expect_d, double);
	SPECIALS(expm1f, ulp_expect_f, float);
	SPECIALS(expm1l, ulp_expect_l, long double);

	return fails != 0;
}
