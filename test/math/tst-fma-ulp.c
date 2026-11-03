/* fma against the correctly rounded result.

   IEEE 754 requires fma correctly rounded, so the budget is zero in every
   setting.  The thousand triples are there because this is the function where a
   wrong implementation hid longest: libm/s_fma.c was glibc's 1997 placeholder,
   return (x * y) + z, which is not a fused operation at all, and fma_test asked
   only for fma(1,2,3) and a few NaNs.  Against these triples that placeholder
   misses 707.  Where the hardware has the instruction the compiler emits it and
   nothing here is being tested; on the sixteen ports without one, every triple
   goes through the integer path in s_fma.c.  See ulp-check.h.  */

#include <stdio.h>
#include "ulp-check.h"
#include "fma-ref.h"

/* Not a budget but a requirement. */
#define FMA_BUDGET	BUDGET(0, 0, 0)
#define FMA_STEPS	0

int main(void)
{
	int fails = 0;

	fails += ulp_sweep3_d("fma", fma, fma_ref, NELEM(fma_ref),
			      FMA_BUDGET, FMA_STEPS);

	/* The cases fma_test() checked.  IEEE 754 fixes every one of them: the
	   result is NaN when a NaN reaches it, and inf times zero is invalid
	   whatever z is.  */
	fails += ulp_expect_d("fma(1,2,3)", fma(1.0, 2.0, 3.0), 5.0);
	fails += ulp_expect_d("fma(nan,2,3)", fma(NAN, 2.0, 3.0), NAN);
	fails += ulp_expect_d("fma(1,nan,3)", fma(1.0, NAN, 3.0), NAN);
	fails += ulp_expect_d("fma(1,2,nan)", fma(1.0, 2.0, NAN), NAN);
	fails += ulp_expect_d("fma(inf,0,nan)", fma(INFINITY, 0.0, NAN), NAN);
	fails += ulp_expect_d("fma(-inf,0,nan)", fma(-INFINITY, 0.0, NAN), NAN);
	fails += ulp_expect_d("fma(0,inf,nan)", fma(0.0, INFINITY, NAN), NAN);
	fails += ulp_expect_d("fma(0,-inf,nan)", fma(0.0, -INFINITY, NAN), NAN);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "fma(inf,0,1)", fma(INFINITY, 0.0, 1.0), NAN, ULP_INVALID);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "fma(0,inf,1)", fma(0.0, INFINITY, 1.0), NAN, ULP_INVALID);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "fma(inf,1,-inf)", fma(INFINITY, 1.0, -INFINITY), NAN, ULP_INVALID);
	fails += ulp_expect_d("fma(inf,1,inf)", fma(INFINITY, 1.0, INFINITY), INFINITY);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "fma(1,inf,-inf)", fma(1.0, INFINITY, -INFINITY), NAN, ULP_INVALID);
	fails += ulp_expect_d("fma(0,0,0)", fma(0.0, 0.0, 0.0), 0.0);
	fails += ulp_expect_d("fma(-0,0,0)", fma(-0.0, 0.0, 0.0), 0.0);
	fails += ulp_expect_d("fma(0,0,-0)", fma(0.0, 0.0, -0.0), 0.0);
	fails += ulp_expect_d("fma(-0,-0,-0)", fma(-0.0, -0.0, -0.0), 0.0);

	ulp_report_skipped();

	return fails != 0;
}
