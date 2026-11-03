/* tgamma, tgammaf and tgammal against the correctly rounded result.

   This replaces tgamma_test() in libm-test.inc.  See ulp-check.h.

   Two implementations here: musl's Lanczos approximation on the small and the
   optimized setting, CORE-MATH on the accurate one.  Until uClibc-ng moved the
   Lanczos file into libm/e_tgamma.c the small setting computed exp(lgamma(x))
   and was not a gamma function at all.  */

#include "ulp-check.h"
#include "tgamma-ref.h"

/* Measured over the 999 points of the table.  The small and the optimized
   setting run the same Lanczos file and differ only in the exp and pow beneath
   it, which moves the count by a couple of points and leaves the worst case
   alone -- musl halves the exponent before calling pow and squares afterwards,
   so pow's accuracy barely reaches the result.

		misses	worst
      small	   721	     6 steps
      optimized	   718	     6
      accurate	     0	     0

   The count is high for both because Lanczos misses the last bit nearly as
   often as anything else does; the worst case is what matters.  For comparison,
   the exp(lgamma(x)) this replaced was 852 steps out on aarch64 and 1269 on
   csky -- exp scales the error of its argument by |lgamma(x)|, so the number
   grew with the argument and differed from target to target.

   Six, then, and nought where CORE-MATH runs.  */
#define D_BUDGET	BUDGET(760, 730, 0)
#define STEPS	BUDGET(6, 6, 0)

/* tgammaf computes in double and converts, and every variant hits every one of
   the 501 float points -- even the small one, whose double error disappears in
   the conversion.  */
#define F_BUDGET	0

/* The cases tgamma_test() checked.  Note the poles differ in sign -- tgamma(+0)
   is +inf and tgamma(-0) is -inf -- and that a negative integer is a domain
   error giving NaN, not a pole.  */
#define SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)INFINITY);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), -(type)INFINITY);	   \
	fails += expect(#fn "(-2)", fn(-(type)2), (type)NAN);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)INFINITY);  \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), (type)NAN);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
	fails += expect(#fn "(1)", fn((type)1), (type)1);		   \
	fails += expect(#fn "(4)", fn((type)4), (type)6);		   \
} while (0)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("tgamma", tgamma, tgamma_ref, NELEM(tgamma_ref),
			      D_BUDGET, STEPS);
	fails += ulp_sweep1_f("tgammaf", tgammaf, tgammaf_ref,
			      NELEM(tgammaf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("tgammal", tgammal, tgamma_ref, NELEM(tgamma_ref),
			      D_BUDGET, STEPS);

	SPECIALS(tgamma, ulp_expect_d, double);
	SPECIALS(tgammaf, ulp_expect_f, float);
	SPECIALS(tgammal, ulp_expect_l, long double);

	return fails != 0;
}
