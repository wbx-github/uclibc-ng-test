/* lgamma, lgammaf and lgammal against the correctly rounded result, and the
   sign they report through signgam.

   This replaces lgamma_test() and gamma_test() in libm-test.inc -- gamma is the
   old name of the same function, and the alias is checked here too.  See
   ulp-check.h.  */

#include <errno.h>
#include "ulp-check.h"
#include "lgamma-ref.h"

/* Measured over the 999 points of the table on riscv32: fdlibm misses the
   correctly rounded result 327 times, CORE-MATH none.  An x86-64 host reports
   323, and the target number is the one that counts: fdlibm's lgamma calls log
   and sin internally, so its result depends on those, and on a host they are
   glibc's rather than ours.  There is no optimized variant --
   until 2025 glibc and musl shipped the same fdlibm routine and Arm has no
   scalar lgamma at all -- so the small and the optimized setting share the code
   and the budget.  */
#define D_BUDGET	BUDGET(335, 335, 0)

/* lgammaf computes in double and converts, and the double error disappears in
   that conversion: measured, no variant misses a single float point.  */
#define F_BUDGET	0

/* Why this one is 45 and not 1: lgamma has zeros at 1, at 2 and between the
   negative poles, fdlibm reaches them by subtraction, and the relative error
   there is genuinely large -- 45 representable values at x = -2.7375, where the
   result is 0x1.3p-6.  Those points are in the table on purpose, since that
   weakness is worth showing; the sensitive half of this test is the count
   above.  The accurate variant is held to nought all the same: CORE-MATH hits
   that same point exactly, so the loss is fdlibm's formula, not the problem.  */
#define STEPS	BUDGET(45, 45, 0)

/* The cases lgamma_test() checked.  lgamma is even about its poles: it is +inf
   at zero and at every negative integer, and +inf at both infinities.  */
#define SPECIALS(fn, expect, type)					   \
do {									   \
	fails += expect(#fn "(0)", fn((type)0), (type)INFINITY);		   \
	fails += expect(#fn "(-0)", fn(-(type)0), (type)INFINITY);	   \
	fails += expect(#fn "(-3)", fn(-(type)3), (type)INFINITY);	   \
	fails += expect(#fn "(inf)", fn((type)INFINITY), (type)INFINITY);  \
	fails += expect(#fn "(-inf)", fn(-(type)INFINITY), (type)INFINITY); \
	fails += expect(#fn "(nan)", fn((type)NAN), (type)NAN);		   \
	fails += expect(#fn "(1)", fn((type)1), (type)0);		   \
	fails += expect(#fn "(2)", fn((type)2), (type)0);		   \
} while (0)

/* signgam is what tells lgamma's caller the sign of gamma itself, and it is the
   only way to get it, so it is worth checking: positive throughout x > 0 and on
   (-2,-1), negative on (-1,0) and (-3,-2).  */
static int check_signgam(void)
{
	static const struct { double x; int want; } t[] = {
		{  0.5, 1 }, {  1.5, 1 }, {  3.0, 1 }, { 170.0, 1 },
		{ -0.5, -1 }, { -1.5, 1 }, { -2.5, -1 }, { -3.5, 1 },
	};
	int fails = 0, i;

	for (i = 0; i < NELEM(t); i++) {
		int sign;

		lgamma_r(t[i].x, &sign);
		if (sign != t[i].want) {
			printf("FAIL: lgamma_r(%a) reported signgam %d, expected %d\n",
			       t[i].x, sign, t[i].want);
			fails++;
		}
		signgam = 0;
		lgamma(t[i].x);
		if (signgam != t[i].want) {
			printf("FAIL: lgamma(%a) left signgam at %d, expected %d\n",
			       t[i].x, signgam, t[i].want);
			fails++;
		}
	}
	return fails;
}

/* gamma is the old name of lgamma; nothing else checks that the alias is there
   and means the same thing. */
static int check_gamma_alias(void)
{
	static const double t[] = { 0.5, 0.7, 1.2, 3.0, -0.5, -2.5 };
	int fails = 0, i;

	/* Through memory, both of them: the i386 calling convention returns a
	   double in st(0) and does not require it rounded to double, so
	   comparing two calls directly compares a rounded copy against an
	   80-bit one and never finds them equal.  */
	for (i = 0; i < NELEM(t); i++) {
		volatile double g = gamma(t[i]), l = lgamma(t[i]);

		if (g != l) {
			printf("FAIL: gamma(%a) is %a, lgamma is %a\n",
			       t[i], g, l);
			fails++;
		}
	}
	return fails;
}

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("lgamma", lgamma, lgamma_ref, NELEM(lgamma_ref),
			      D_BUDGET, STEPS);
	fails += ulp_sweep1_f("lgammaf", lgammaf, lgammaf_ref,
			      NELEM(lgammaf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("lgammal", lgammal, lgamma_ref, NELEM(lgamma_ref),
			      D_BUDGET, STEPS);

	SPECIALS(lgamma, ulp_expect_d, double);
	SPECIALS(lgammaf, ulp_expect_f, float);
	SPECIALS(lgammal, ulp_expect_l, long double);

	fails += check_signgam();
	fails += check_gamma_alias();

	return fails != 0;
}
