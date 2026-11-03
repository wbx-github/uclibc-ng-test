/* sqrt and its float and long double entry points against the correctly
   rounded result.

   This replaces sqrt_test() in libm-test.inc, which checked fourteen named
   points.  IEEE 754 requires sqrt correctly rounded, so the budget is zero in
   every setting -- there is no accuracy to trade and nothing for a variant to
   improve.  What the thousand points are for is the ports without a sqrt
   instruction, where libm/e_sqrt.c runs a shift-and-subtract loop that the
   fourteen named points could not have vouched for.  See ulp-check.h.  */

#include <stdio.h>
#include "ulp-check.h"
#include "sqrt-ref.h"

/* Not a budget but a requirement.  Measured: libm/e_sqrt.c hits every one of
   the thousand points.  */
#define SQRT_BUDGET	BUDGET(0, 0, 0)
#define SQRT_STEPS	0

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("sqrt", sqrt, sqrt_ref, NELEM(sqrt_ref),
			      SQRT_BUDGET, SQRT_STEPS);
	fails += ulp_sweep1_l("sqrtl", sqrtl, sqrt_ref, NELEM(sqrt_ref),
			      SQRT_BUDGET, SQRT_STEPS);

	/* The cases sqrt_test() checked.  A negative argument is a domain
	   error, and e_sqrt.c answers it with (x-x)/(x-x).  */
	fails += ulp_expect_d("sqrt(0)", sqrt(0.0), 0.0);
	fails += ulp_expect_d("sqrt(-0)", sqrt(-0.0), -0.0);
	fails += ulp_expect_d("sqrt(inf)", sqrt(INFINITY), INFINITY);
	fails += ulp_expect_d("sqrt(nan)", sqrt(NAN), NAN);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "sqrt(-1)", sqrt(-1.0), NAN, ULP_INVALID);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "sqrt(-inf)", sqrt(-INFINITY), NAN, ULP_INVALID);
	fails += ulp_expect_d("sqrt(1)", sqrt(1.0), 1.0);
	fails += ulp_expect_d("sqrt(4)", sqrt(4.0), 2.0);
	fails += ulp_expect_d("sqrt(2)", sqrt(2.0), 0x1.6a09e667f3bcdp+0);

	ulp_report_skipped();

	return fails != 0;
}
