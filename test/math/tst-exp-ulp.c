/* exp, expf and expl against the correctly rounded result.

   This replaces exp_test() in libm-test.inc, which measured the same three
   functions against glibc's ulp tables: it covers the same special cases and
   1000 points instead of six values, and it knows which implementation the
   library was configured with.  See ulp-check.h.  */

#include "ulp-check.h"
#include "exp-ref.h"

/* Measured on x86-64 over these 1000 points: fdlibm misses the correctly
   rounded result 101 times, Arm's routines twice, CORE-MATH never.  The budgets
   sit just above that on purpose -- a target that scores worse is the
   interesting case and should be looked at, not accommodated.  Which input
   falls on the wrong side of a rounding boundary does move with the compiler and
   with fma contraction, so some arch will trip these; when one does, find out
   why before raising the number, and write down what came out.  */
#define D_BUDGET	BUDGET(110, 4, 0)

/* No variant lands further than one representable value from the reference. */
#define STEPS	BUDGET(1, 1, 0)

/* expf computes in double and converts (libm/w_expf.c), which rounds twice.
   Measured, and not what one would guess: every variant hits every one of the
   500 float points, because the double error stays far below the float rounding
   boundary.  Hence zero for all three -- this holds the wrapper to the double
   implementation, it does not grade the implementation.  */
#define F_BUDGET	0

/* The cases exp_test() checked, for each of the three entry points. */
#define SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)1);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), (type)1);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)INFINITY);  \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), (type)0);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
} while (0)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("exp", exp, exp_ref, NELEM(exp_ref), D_BUDGET, STEPS);
	fails += ulp_sweep1_f("expf", expf, expf_ref, NELEM(expf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("expl", expl, exp_ref, NELEM(exp_ref), D_BUDGET, STEPS);

	SPECIALS(exp, ulp_expect_d, double);
	SPECIALS(expf, ulp_expect_f, float);
	SPECIALS(expl, ulp_expect_l, long double);

	return fails != 0;
}
