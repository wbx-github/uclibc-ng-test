/* erf, erfc and their float and long double entry points against the correctly
   rounded result.

   This replaces erf_test() and erfc_test() in libm-test.inc, which measured the
   same six functions against glibc's ulp tables.  See ulp-check.h.  */

#include "ulp-check.h"
#include "erf-ref.h"

/* Measured over the 1000 points of the table: fdlibm misses the correctly
   rounded result 59 times, Arm's routine 107, CORE-MATH none.  Arm being the
   worse of the first two is not a mistake -- it aims at about 0.5 ulp and buys
   speed with that, five times fdlibm's on a target with an fma instruction.  */
#define ERF_BUDGET	BUDGET(65, 115, 0)

/* erfc has no optimized variant: Arm has no scalar erfc and glibc's is
   CORE-MATH, so the small and the optimized setting are the same fdlibm code,
   and they get the same budget.  116 measured of 1000 on riscv32 -- and that
   number, not the 109 an x86-64 host reports, is the one to trust: fdlibm's
   erfc calls exp twice, so what it misses depends on the exp underneath it, and
   on a host that is glibc's rather than ours.  */
#define ERFC_BUDGET	BUDGET(120, 120, 0)

/* erff and erfcf compute in double and convert.  Measured: every variant hits
   every float point, the double error staying far below the float rounding
   boundary, so this holds the wrappers to the double implementation.  */
#define F_BUDGET	0

/* How far the worst point may be, and this is a requirement rather than an
   observation.  It follows the implementation, not the setting: the accurate
   variant is correctly rounded, so nought; the optimized ones aim below half an
   ulp and fdlibm's own claim is below one, so either may land on the
   neighbouring representable value but no further.  */
#define ERF_STEPS	BUDGET(1, 1, 0)

/* erfc gets two, because fdlibm's one-ulp claim is for erf alone: s_erf.c says
   0.84375 was chosen "to guarantee the error is less than one ulp for erf", and
   for erfc it bounds the rational approximations only, never the result.  erfc
   is 1 - erf and calls exp twice, and that composition costs the extra step.
   Measured across 38 targets, hard and soft float, both settings: erf is one
   step out everywhere, erfc two, never three.  No margin on top, so a
   regression shows on the next run.  Accurate stays at nought, where CORE-MATH
   is exact.  */
#define ERFC_STEPS	BUDGET(2, 2, 0)

/* The cases erf_test() checked, for each of the three entry points. */
#define ERF_SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)0);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), -(type)0);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)1);	   \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), -(type)1);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
	fails += expect(#fn "(27)", fn((type)27), (type)1);		   \
} while (0)

/* And the ones erfc_test() checked.  erfc(-inf) is 2, not -0: the function is
   1 - erf, so it grows to the left.  */
#define ERFC_SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)1);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), (type)1);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)0);	   \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), (type)2);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
} while (0)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("erf", erf, erf_ref, NELEM(erf_ref),
			      ERF_BUDGET, ERF_STEPS);
	fails += ulp_sweep1_f("erff", erff, erff_ref, NELEM(erff_ref),
			      F_BUDGET, ERF_STEPS);
	fails += ulp_sweep1_l("erfl", erfl, erf_ref, NELEM(erf_ref),
			      ERF_BUDGET, ERF_STEPS);

	fails += ulp_sweep1_d("erfc", erfc, erfc_ref, NELEM(erfc_ref),
			      ERFC_BUDGET, ERFC_STEPS);
	fails += ulp_sweep1_f("erfcf", erfcf, erfcf_ref, NELEM(erfcf_ref),
			      F_BUDGET, ERFC_STEPS);
	fails += ulp_sweep1_l("erfcl", erfcl, erfc_ref, NELEM(erfc_ref),
			      ERFC_BUDGET, ERFC_STEPS);

	ERF_SPECIALS(erf, ulp_expect_d, double);
	ERF_SPECIALS(erff, ulp_expect_f, float);
	ERF_SPECIALS(erfl, ulp_expect_l, long double);

	ERFC_SPECIALS(erfc, ulp_expect_d, double);
	ERFC_SPECIALS(erfcf, ulp_expect_f, float);
	ERFC_SPECIALS(erfcl, ulp_expect_l, long double);

	return fails != 0;
}
