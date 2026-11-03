/* asin, acos, atan, atan2 and their float and long double entry points against
   the correctly rounded result.

   This replaces asin_test(), acos_test(), atan_test() and atan2_test() in
   libm-test.inc, which measured the same twelve functions against glibc's ulp
   tables at about ten named points each.  See ulp-check.h.  */

#include <stdio.h>
#include "ulp-check.h"
#include "atan-ref.h"

/* Measured over the points of the tables: fdlibm misses the correctly rounded
   result 21 times for asin, 17 for acos, 20 for atan and 172 for atan2, and
   never by more than one step.  None of the four has an optimized or an
   accurate variant yet -- CORE-MATH has all four, so the accurate column is
   where they belong -- so the three settings share one number.  */
#define ASIN_BUDGET	BUDGET(26, 26, 26)
#define ACOS_BUDGET	BUDGET(22, 22, 22)
#define ATAN_BUDGET	BUDGET(25, 25, 25)
#define ATAN2_BUDGET	BUDGET(180, 180, 180)

/* The float entry points compute in double and convert, which leaves the error
   far below the float rounding boundary: every setting hits every point.  */
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

	fails += ulp_sweep1_d("asin", asin, asin_ref, NELEM(asin_ref), ASIN_BUDGET, STEPS);
	fails += ulp_sweep1_f("asinf", asinf, asinf_ref, NELEM(asinf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("asinl", asinl, asin_ref, NELEM(asin_ref), ASIN_BUDGET, STEPS);

	fails += ulp_sweep1_d("acos", acos, acos_ref, NELEM(acos_ref), ACOS_BUDGET, STEPS);
	fails += ulp_sweep1_f("acosf", acosf, acosf_ref, NELEM(acosf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("acosl", acosl, acos_ref, NELEM(acos_ref), ACOS_BUDGET, STEPS);

	fails += ulp_sweep1_d("atan", atan, atan_ref, NELEM(atan_ref), ATAN_BUDGET, STEPS);
	fails += ulp_sweep1_f("atanf", atanf, atanf_ref, NELEM(atanf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("atanl", atanl, atan_ref, NELEM(atan_ref), ATAN_BUDGET, STEPS);

	fails += ulp_sweep2_d("atan2", atan2, atan2_ref, NELEM(atan2_ref), ATAN2_BUDGET, STEPS);
	fails += ulp_sweep2_f("atan2f", atan2f, atan2f_ref, NELEM(atan2f_ref), F_BUDGET, STEPS);
	fails += ulp_sweep2_l("atan2l", atan2l, atan2_ref, NELEM(atan2_ref), ATAN2_BUDGET, STEPS);

	/* The cases the four bodies checked.  asin and acos are a domain error
	   outside [-1, 1]; atan2 fixes a value for every combination of signs
	   and zeros, which is the whole reason it exists beside atan.  */
	fails += ulp_expect_d("asin(0)", asin(0.0), 0.0);
	fails += ulp_expect_d("asin(-0)", asin(-0.0), -0.0);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "asin(inf)", asin(INFINITY), NAN, ULP_INVALID);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "asin(-inf)", asin(-INFINITY), NAN, ULP_INVALID);
	fails += ulp_expect_d("asin(nan)", asin(NAN), NAN);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "asin(1.125)", asin(1.125), NAN, ULP_INVALID);
	fails += ulp_expect_d("asin(1)", asin(1.0), M_PI_2);

	fails += ulp_expect_d("acos(1)", acos(1.0), 0.0);
	fails += ulp_expect_d("acos(0)", acos(0.0), M_PI_2);
	fails += ulp_expect_d("acos(-0)", acos(-0.0), M_PI_2);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "acos(inf)", acos(INFINITY), NAN, ULP_INVALID);
	fails += ulp_expect_d("acos(nan)", acos(NAN), NAN);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "acos(1.125)", acos(1.125), NAN, ULP_INVALID);

	fails += ulp_expect_d("atan(0)", atan(0.0), 0.0);
	fails += ulp_expect_d("atan(-0)", atan(-0.0), -0.0);
	fails += ulp_expect_d("atan(inf)", atan(INFINITY), M_PI_2);
	fails += ulp_expect_d("atan(-inf)", atan(-INFINITY), -M_PI_2);
	fails += ulp_expect_d("atan(nan)", atan(NAN), NAN);

	fails += ulp_expect_d("atan2(0,1)", atan2(0.0, 1.0), 0.0);
	fails += ulp_expect_d("atan2(-0,1)", atan2(-0.0, 1.0), -0.0);
	fails += ulp_expect_d("atan2(0,0)", atan2(0.0, 0.0), 0.0);
	fails += ulp_expect_d("atan2(-0,0)", atan2(-0.0, 0.0), -0.0);
	fails += ulp_expect_d("atan2(0,-1)", atan2(0.0, -1.0), M_PI);
	fails += ulp_expect_d("atan2(-0,-1)", atan2(-0.0, -1.0), -M_PI);
	fails += ulp_expect_d("atan2(0,-0)", atan2(0.0, -0.0), M_PI);
	fails += ulp_expect_d("atan2(nan,1)", atan2(NAN, 1.0), NAN);
	fails += ulp_expect_d("atan2(1,nan)", atan2(1.0, NAN), NAN);

	ulp_report_skipped();

	return fails != 0;
}
