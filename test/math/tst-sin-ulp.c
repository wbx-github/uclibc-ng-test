/* sin, cos, sincos and their float and long double entry points against the
   correctly rounded result.

   This replaces sin_test(), cos_test() and sincos_test() in libm-test.inc,
   which measured the same nine functions against glibc's ulp tables at ten
   named points each.  See ulp-check.h.  */

/* sincos is a GNU extension, so it needs the feature macro before math.h. */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include "ulp-check.h"
#include "sin-ref.h"

/* Measured over the points of the table: fdlibm misses the correctly rounded
   result 26 times for sin and 20 for cos, CORE-MATH none.  There is no
   optimized and no accurate variant -- Arm's optimized-routines has no portable
   scalar sin or cos, and CORE-MATH's carry their argument reduction in a real
   128-bit integer, which 22 of our 34 toolchains cannot compile -- so all three
   settings are the same fdlibm code and share one number.

   Those 26 and 20 hold on every target only because libm/s_sin.c and s_cos.c
   put the x87 at 53 bits for the duration of the call: the reduction carries
   pi/2 in two words and needs each operation rounded to double.  Without that,
   i686 misses 53 and 68 -- the damage is in the reduction, 46 of the 68 in the
   block beyond 2^20 where __kernel_rem_pio2 runs.  The slack over the
   measurement is for targets where the compiler contracts a multiply and an add
   into one instruction.  */
#define SIN_BUDGET	BUDGET(30, 30, 30)
#define COS_BUDGET	BUDGET(24, 24, 24)

#define F_BUDGET	0

/* How far the worst point may be, and this is a requirement rather than an
   observation.  It follows the implementation, not the setting -- and there is
   no variant of this function, so all three settings run the same fdlibm code
   and the accurate column is one and not nought.  fdlibm's own claim is an
   error below one ulp, so the neighbouring representable value is as far as it
   may land.  */
#define STEPS	BUDGET(1, 1, 1)

/* The cases sin_test() checked, for each of the three entry points.  sin of an
   infinity is a domain error, so NaN. */
#define SIN_SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)0);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), -(type)0);		   \
	fails += ULP_EXPECT_EXC(expect, #fn "(inf)", fn((type)INFINITY), (type)NAN, ULP_INVALID);	   \
	fails += ULP_EXPECT_EXC(expect, #fn "(-inf)", fn(-(type)INFINITY), (type)NAN, ULP_INVALID);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
} while (0)

/* And the ones cos_test() checked. */
#define COS_SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)1);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), (type)1);		   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)NAN);	   \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), (type)NAN);	   \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
} while (0)

/* sincos needs no budget of its own.  libm/sincos.c is *s = sin(x), *c = cos(x),
   so its accuracy is theirs and the only thing worth checking is that it stays
   that way -- an implementation that shares the argument reduction between the
   two, which is the point of having sincos at all, would have to agree here.
   Checked over the same points as the sweeps above.  */
static int sincos_agrees(void)
{
	int i, fails = 0;

	for (i = 0; i < NELEM(sin_ref); i++) {
		double x = sin_ref[i].x, s, c;

		sincos(x, &s, &c);
		if (s != sin(x)) {
			printf("FAIL: sincos(%a) put %a in the sine, sin gives %a\n",
			       x, s, sin(x));
			fails++;
		}
		if (c != cos(x)) {
			printf("FAIL: sincos(%a) put %a in the cosine, cos gives %a\n",
			       x, c, cos(x));
			fails++;
		}
		if (fails > 4)
			break;
	}
	return fails;
}

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("sin", sin, sin_ref, NELEM(sin_ref),
			      SIN_BUDGET, STEPS);
	fails += ulp_sweep1_f("sinf", sinf, sinf_ref, NELEM(sinf_ref),
			      F_BUDGET, STEPS);
	fails += ulp_sweep1_l("sinl", sinl, sin_ref, NELEM(sin_ref),
			      SIN_BUDGET, STEPS);

	fails += ulp_sweep1_d("cos", cos, cos_ref, NELEM(cos_ref),
			      COS_BUDGET, STEPS);
	fails += ulp_sweep1_f("cosf", cosf, cosf_ref, NELEM(cosf_ref),
			      F_BUDGET, STEPS);
	fails += ulp_sweep1_l("cosl", cosl, cos_ref, NELEM(cos_ref),
			      COS_BUDGET, STEPS);

	SIN_SPECIALS(sin, ulp_expect_d, double);
	SIN_SPECIALS(sinf, ulp_expect_f, float);
	SIN_SPECIALS(sinl, ulp_expect_l, long double);

	COS_SPECIALS(cos, ulp_expect_d, double);
	COS_SPECIALS(cosf, ulp_expect_f, float);
	COS_SPECIALS(cosl, ulp_expect_l, long double);

	fails += sincos_agrees();

	ulp_report_skipped();

	return fails != 0;
}
