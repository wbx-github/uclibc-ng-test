/* acosh, asinh, atanh and their float and long double entry points against the
   correctly rounded result.

   This replaces acosh_test(), asinh_test() and atanh_test() in libm-test.inc,
   which measured the same nine functions against glibc's ulp tables at about
   ten named points each.  See ulp-check.h.  */

#include <stdio.h>
#include "ulp-check.h"
#include "acosh-ref.h"

/* Measured over the points of the tables: fdlibm misses the correctly rounded
   result 177 times for acosh, 242 for asinh and 108 for atanh, never by more
   than one step.  The counts are high because all three are built out of log
   and log1p and inherit their error.  None has an optimized or an accurate
   variant yet; CORE-MATH has all three.  */
#define ACOSH_BUDGET	BUDGET(185, 185, 185)
#define ASINH_BUDGET	BUDGET(252, 252, 252)
#define ATANH_BUDGET	BUDGET(115, 115, 115)
#define F_BUDGET	0

/* How far the worst point may be, and this is a requirement rather than an
   observation.  It follows the implementation, not the setting -- and there is
   no variant of this function, so all three settings run the same fdlibm code
   and the accurate column is one and not nought.  fdlibm's own claim is an
   error below one ulp, so the neighbouring representable value is as far as it
   may land.  */
#define STEPS		BUDGET(1, 1, 1)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("acosh", acosh, acosh_ref, NELEM(acosh_ref), ACOSH_BUDGET, STEPS);
	fails += ulp_sweep1_f("acoshf", acoshf, acoshf_ref, NELEM(acoshf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("acoshl", acoshl, acosh_ref, NELEM(acosh_ref), ACOSH_BUDGET, STEPS);

	fails += ulp_sweep1_d("asinh", asinh, asinh_ref, NELEM(asinh_ref), ASINH_BUDGET, STEPS);
	fails += ulp_sweep1_f("asinhf", asinhf, asinhf_ref, NELEM(asinhf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("asinhl", asinhl, asinh_ref, NELEM(asinh_ref), ASINH_BUDGET, STEPS);

	fails += ulp_sweep1_d("atanh", atanh, atanh_ref, NELEM(atanh_ref), ATANH_BUDGET, STEPS);
	fails += ulp_sweep1_f("atanhf", atanhf, atanhf_ref, NELEM(atanhf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("atanhl", atanhl, atanh_ref, NELEM(atanh_ref), ATANH_BUDGET, STEPS);

	/* The cases the three bodies checked.  acosh is a domain error below 1,
	   atanh outside [-1, 1], and atanh is infinite at either end.  */
	fails += ulp_expect_d("acosh(1)", acosh(1.0), 0.0);
	fails += ulp_expect_d("acosh(inf)", acosh(INFINITY), INFINITY);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "acosh(-inf)", acosh(-INFINITY), NAN, ULP_INVALID);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "acosh(-1.125)", acosh(-1.125), NAN, ULP_INVALID);
	fails += ulp_expect_d("acosh(nan)", acosh(NAN), NAN);

	fails += ulp_expect_d("asinh(0)", asinh(0.0), 0.0);
	fails += ulp_expect_d("asinh(-0)", asinh(-0.0), -0.0);
	fails += ulp_expect_d("asinh(inf)", asinh(INFINITY), INFINITY);
	fails += ulp_expect_d("asinh(-inf)", asinh(-INFINITY), -INFINITY);
	fails += ulp_expect_d("asinh(nan)", asinh(NAN), NAN);

	fails += ulp_expect_d("atanh(0)", atanh(0.0), 0.0);
	fails += ulp_expect_d("atanh(-0)", atanh(-0.0), -0.0);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "atanh(1)", atanh(1.0), INFINITY, ULP_DIVBYZERO);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "atanh(-1)", atanh(-1.0), -INFINITY, ULP_DIVBYZERO);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "atanh(1.125)", atanh(1.125), NAN, ULP_INVALID);
	fails += ulp_expect_d("atanh(nan)", atanh(NAN), NAN);

	ulp_report_skipped();

	return fails != 0;
}
