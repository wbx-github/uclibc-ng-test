/* tan and its float and long double entry points against the correctly rounded
   result.

   This replaces tan_test() in libm-test.inc, which measured the same three
   functions against glibc's ulp tables at seven named points.  See
   ulp-check.h.  */

#include "ulp-check.h"
#include "tan-ref.h"

/* Measured over the 999 points of the table: fdlibm misses the correctly
   rounded result 19 times, CORE-MATH none.  There is no optimized and no
   accurate variant -- Arm's optimized-routines has no portable scalar tan, and
   CORE-MATH's carries its argument reduction in a real 128-bit integer, which
   22 of our 34 toolchains cannot compile -- so all three settings are the same
   fdlibm code and share one number.

   That 19 holds on every target only because libm/s_tan.c puts the x87 at 53
   bits for the duration of the call: the reduction carries pi/2 in two words
   and needs each operation rounded to double.  Without that, i686 misses 135,
   99 of them in the block beyond 2^20 where __kernel_rem_pio2 runs.  The slack
   over the measurement is for targets where the compiler contracts a multiply
   and an add into one instruction.  */
#define TAN_BUDGET	BUDGET(23, 23, 23)

/* tanf computes in double and converts (libm/float_wrappers.c), which leaves
   the double error far below the float rounding boundary.  Measured: every
   setting hits every float point, so this holds the wrapper to the double
   implementation.  */
#define F_BUDGET	0

/* How far the worst point may be, and this is a requirement rather than an
   observation.  It follows the implementation, not the setting -- and there is
   no variant of this function, so all three settings run the same fdlibm code
   and the accurate column is one and not nought.  fdlibm's own claim is an
   error below one ulp, so the neighbouring representable value is as far as it
   may land.  */
#define STEPS	BUDGET(1, 1, 1)

/* The cases tan_test() checked, for each of the three entry points.  tan of an
   infinity is a domain error, so NaN.  */
#define SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)0);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), -(type)0);		   \
	fails += ULP_EXPECT_EXC(expect, #fn "(inf)", fn((type)INFINITY), (type)NAN, ULP_INVALID);	   \
	fails += ULP_EXPECT_EXC(expect, #fn "(-inf)", fn(-(type)INFINITY), (type)NAN, ULP_INVALID);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
} while (0)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("tan", tan, tan_ref, NELEM(tan_ref),
			      TAN_BUDGET, STEPS);
	fails += ulp_sweep1_f("tanf", tanf, tanf_ref, NELEM(tanf_ref),
			      F_BUDGET, STEPS);
	fails += ulp_sweep1_l("tanl", tanl, tan_ref, NELEM(tan_ref),
			      TAN_BUDGET, STEPS);

	SPECIALS(tan, ulp_expect_d, double);
	SPECIALS(tanf, ulp_expect_f, float);
	SPECIALS(tanl, ulp_expect_l, long double);

	ulp_report_skipped();

	return fails != 0;
}
