/* cbrt, cosh, log1p and their float and long double entry points against the
   correctly rounded result.

   This replaces cbrt_test(), cosh_test() and log1p_test() in libm-test.inc,
   which measured the same nine functions against glibc's ulp tables at about
   ten named points each.  See ulp-check.h.  */

#include <stdio.h>
#include "ulp-check.h"
#include "cbrt-ref.h"

/* Measured over the points of the tables: fdlibm misses the correctly rounded
   result 65 times for cbrt, 96 for cosh and 17 for log1p, never by more than
   one step.  None has an optimized or an accurate variant of its own; CORE-MATH
   has all three.

   cosh is the exception, and not because of cosh: it is built on expm1 and exp,
   and both of those have an optimized and an accurate variant, so cosh inherits
   them and reaches 104 on either setting where the small one stays at 96.  sinh
   behaves the same way and is one step better for it -- see tst-sinh-ulp.c.  */
#define CBRT_BUDGET	BUDGET(70, 70, 70)
#define COSH_BUDGET	BUDGET(102, 104, 104)
#define LOG1P_BUDGET	BUDGET(22, 22, 22)
#define F_BUDGET	0

/* How far the worst point may be.  One for all three settings, since none of
   these has a variant of its own, and one is what they deliver: measured, no
   point of any of the three tables is further out than the neighbouring
   representable value.  s_cbrt.c and s_log1p.c also claim it -- "error less
   than 0.667 ulps" and "error in log below 1ulp" -- while e_cosh.c claims
   nothing, so for cosh the one is measurement rather than promise.  */
#define STEPS		BUDGET(1, 1, 1)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep1_d("cbrt", cbrt, cbrt_ref, NELEM(cbrt_ref), CBRT_BUDGET, STEPS);
	fails += ulp_sweep1_f("cbrtf", cbrtf, cbrtf_ref, NELEM(cbrtf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("cbrtl", cbrtl, cbrt_ref, NELEM(cbrt_ref), CBRT_BUDGET, STEPS);

	fails += ulp_sweep1_d("cosh", cosh, cosh_ref, NELEM(cosh_ref), COSH_BUDGET, STEPS);
	fails += ulp_sweep1_f("coshf", coshf, coshf_ref, NELEM(coshf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("coshl", coshl, cosh_ref, NELEM(cosh_ref), COSH_BUDGET, STEPS);

	fails += ulp_sweep1_d("log1p", log1p, log1p_ref, NELEM(log1p_ref), LOG1P_BUDGET, STEPS);
	fails += ulp_sweep1_f("log1pf", log1pf, log1pf_ref, NELEM(log1pf_ref), F_BUDGET, STEPS);
	fails += ulp_sweep1_l("log1pl", log1pl, log1p_ref, NELEM(log1p_ref), LOG1P_BUDGET, STEPS);

	/* The cases the three bodies checked.  cbrt keeps the sign of its
	   argument; cosh is even and overflows; log1p is -inf at -1 and a
	   domain error below it.  */
	fails += ulp_expect_d("cbrt(0)", cbrt(0.0), 0.0);
	fails += ulp_expect_d("cbrt(-0)", cbrt(-0.0), -0.0);
	fails += ulp_expect_d("cbrt(inf)", cbrt(INFINITY), INFINITY);
	fails += ulp_expect_d("cbrt(-inf)", cbrt(-INFINITY), -INFINITY);
	fails += ulp_expect_d("cbrt(nan)", cbrt(NAN), NAN);
	fails += ulp_expect_d("cbrt(8)", cbrt(8.0), 2.0);
	fails += ulp_expect_d("cbrt(-27)", cbrt(-27.0), -3.0);

	fails += ulp_expect_d("cosh(0)", cosh(0.0), 1.0);
	fails += ulp_expect_d("cosh(-0)", cosh(-0.0), 1.0);
	fails += ulp_expect_d("cosh(inf)", cosh(INFINITY), INFINITY);
	fails += ulp_expect_d("cosh(-inf)", cosh(-INFINITY), INFINITY);
	fails += ulp_expect_d("cosh(nan)", cosh(NAN), NAN);

	fails += ulp_expect_d("log1p(0)", log1p(0.0), 0.0);
	fails += ulp_expect_d("log1p(-0)", log1p(-0.0), -0.0);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "log1p(-1)", log1p(-1.0), -INFINITY, ULP_DIVBYZERO);
	fails += ULP_EXPECT_EXC(ulp_expect_d, "log1p(-2)", log1p(-2.0), NAN, ULP_INVALID);
	fails += ulp_expect_d("log1p(inf)", log1p(INFINITY), INFINITY);
	fails += ulp_expect_d("log1p(nan)", log1p(NAN), NAN);

	ulp_report_skipped();

	return fails != 0;
}
