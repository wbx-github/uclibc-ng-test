/* pow and its float and long double entry points against the correctly rounded
   result.

   This does not replace pow_test() in libm-test.inc: that body is 129 lines of
   which 119 are the special cases C99 lays down for pow, all of them exact
   requirements rather than accuracy measurements, and they belong where they
   are.  What was missing was any measurement of how well pow computes in
   between, which is what this adds.  See ulp-check.h.  */

#include "ulp-check.h"
#include "pow-ref.h"

/* Measured over the 999 pairs of the table: fdlibm misses the correctly rounded
   result 78 times, CORE-MATH none.  Arm's optimized-routines carries a portable
   scalar pow, so the optimized column is where it would go; until then all
   three settings are the same code.

   The budget comes from a target that rounds every operation to double.  On
   i686 the x87 hits every one of the 999 pairs, because pow splits its
   intermediate into two words and the 11 extra mantissa bits of the x87
   registers absorb exactly that -- so pow wants no help from
   x87-precision.h.  */
#define POW_BUDGET	BUDGET(85, 85, 85)

/* powf computes in double and converts (libm/float_wrappers.c), which leaves
   the double error far below the float rounding boundary.  */
#define F_BUDGET	0

/* How far the worst point may be, and this is a requirement rather than an
   observation.  It follows the implementation, not the setting -- and there is
   no variant of this function, so all three settings run the same fdlibm code
   and the accurate column is one and not nought.  fdlibm's own claim is an
   error below one ulp, so the neighbouring representable value is as far as it
   may land.  */
#define STEPS	BUDGET(1, 1, 1)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep2_d("pow", pow, pow_ref, NELEM(pow_ref),
			      POW_BUDGET, STEPS);
	fails += ulp_sweep2_f("powf", powf, powf_ref, NELEM(powf_ref),
			      F_BUDGET, STEPS);
	fails += ulp_sweep2_l("powl", powl, pow_ref, NELEM(pow_ref),
			      POW_BUDGET, STEPS);

	return fails != 0;
}
