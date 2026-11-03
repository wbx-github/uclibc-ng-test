/* exp2, exp10 and their float and long double entry points against the
   correctly rounded result.

   This replaces exp2_test() and exp10_test() in libm-test.inc, which measured
   the same six functions against glibc's ulp tables at ten named points each.
   See ulp-check.h.  */

#include "ulp-check.h"
#include "exp2-ref.h"

/* exp2 has no implementation in this library: libm/w_exp2.c computes
   pow(2.0, x), so the 109 misses of 1000 measured here are pow's at base two.
   Arm's optimized-routines carries a portable scalar exp2 that reuses the same
   __exp_data table libm/optimized/e_exp.c already ships, so this is the place
   where the optimized column should one day be much lower.  */
#define EXP2_BUDGET	BUDGET(118, 118, 118)

/* exp10 was the worst function in this library until libm/e_exp10.c was fixed,
   and its own author had said so: it was Ulrich Drepper's 1998 placeholder,
   whose comment read "This is a very stupid and inprecise implementation.  It'll
   get replaced sometime (soon?)".  It computed exp(M_LN10 * x); the product
   rounds to double, so exp amplified an argument error of |x| * ln10 * 2^-53 --
   792 of these 1000 points were off, the worst by 845 representable values, and
   exp10(1) was not 10.

   It now carries ln(10) in two words and hands exp the tail as a relative
   correction, which costs ten more operations and no table: 272 points off,
   none by more than one step.  Arm's optimized-routines has a table-driven
   exp10 that would go a good deal lower still, and reuses the __exp_data that
   libm/optimized/e_exp.c already ships, so the optimized column is where that
   belongs.  */
#define EXP10_BUDGET	BUDGET(285, 285, 285)

/* exp2f and exp10f compute in double and convert (libm/float_wrappers.c).  Even
   both stay below the float rounding boundary, so they hit every float point.  */
#define F_BUDGET	0

/* How far the worst point may be, and this is a requirement rather than an
   observation.  It follows the implementation, not the setting -- and there is
   no variant of this function, so all three settings run the same fdlibm code
   and the accurate column is one and not nought.  fdlibm's own claim is an
   error below one ulp, so the neighbouring representable value is as far as it
   may land.  */
#define EXP2_STEPS	BUDGET(1, 1, 1)
#define EXP10_STEPS	BUDGET(1, 1, 1)

/* The cases exp2_test() and exp10_test() checked, for each entry point. */
#define SPECIALS(fn, expect, type, big)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)1);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), (type)1);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)INFINITY);   \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), (type)0);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
	fails += expect(#fn "(1e6)", fn((type)1e6), (type)INFINITY);	   \
	fails += expect(#fn "(-1e6)", fn(-(type)1e6), (type)0);		   \
	fails += expect(#fn "(big)", fn((type)big[0]), (type)big[1]);	   \
} while (0)

int main(void)
{
	int fails = 0;
	/* exp2(10) == 1024 and exp10(3) == 1000, both exact. */
	static const double e2[2] = { 10.0, 1024.0 }, e10[2] = { 3.0, 1000.0 };

	fails += ulp_sweep1_d("exp2", exp2, exp2_ref, NELEM(exp2_ref),
			      EXP2_BUDGET, EXP2_STEPS);
	fails += ulp_sweep1_f("exp2f", exp2f, exp2f_ref, NELEM(exp2f_ref),
			      F_BUDGET, EXP2_STEPS);
	fails += ulp_sweep1_l("exp2l", exp2l, exp2_ref, NELEM(exp2_ref),
			      EXP2_BUDGET, EXP2_STEPS);

	fails += ulp_sweep1_d("exp10", exp10, exp10_ref, NELEM(exp10_ref),
			      EXP10_BUDGET, EXP10_STEPS);
	fails += ulp_sweep1_f("exp10f", exp10f, exp10f_ref, NELEM(exp10f_ref),
			      F_BUDGET, EXP10_STEPS);
	fails += ulp_sweep1_l("exp10l", exp10l, exp10_ref, NELEM(exp10_ref),
			      EXP10_BUDGET, EXP10_STEPS);

	SPECIALS(exp2, ulp_expect_d, double, e2);
	SPECIALS(exp2f, ulp_expect_f, float, e2);
	SPECIALS(exp2l, ulp_expect_l, long double, e2);

	SPECIALS(exp10, ulp_expect_d, double, e10);
	SPECIALS(exp10f, ulp_expect_f, float, e10);
	SPECIALS(exp10l, ulp_expect_l, long double, e10);

	fails += ulp_expect_d("exp2(-1)", exp2(-1.0), 0.5);

	return fails != 0;
}
