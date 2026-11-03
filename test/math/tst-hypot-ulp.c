/* hypot, hypotf and hypotl against the correctly rounded result.

   This replaces hypot_test() in libm-test.inc, which measured the same three
   functions against glibc's ulp tables: it covers the same special cases -- the
   inf/NaN matrix, the sign symmetry, hypot(x,0) == fabs(x) -- and 1275 pairs
   instead of sixteen values, and it knows which implementation the library was
   configured with.  See ulp-check.h.

   The pairs in hypot-ref.h are deliberately biased towards hard cases, so the
   counts below say nothing about how often hypot is wrong in practice.  They
   only have to separate the variants from each other.  */

#include "ulp-check.h"
#include "hypot-ref.h"

/* Measured on x86-64 over these 1275 pairs: fdlibm misses 248, glibc's version
   6, CORE-MATH none.  Kept tight for the reason given in tst-exp-ulp.c.  */
#define D_BUDGET	BUDGET(260, 8, 0)

/* No variant lands further than one representable value from the reference. */
#define STEPS	BUDGET(1, 1, 0)

/* hypotf computes in double and converts (libm/w_hypotf.c).  Measured: every
   variant hits every one of the 343 float pairs, so this holds the wrapper to
   the double implementation rather than grading the implementation.  */
#define F_BUDGET	0

/* The cases hypot_test() checked, for each of the three entry points.  The
   symmetry and the identity are checked as such instead of against a constant,
   which is one thing more than the old test did.  */
#define SPECIALS(fn, expect, type)					     \
do {									     \
	type pinf = (type)INFINITY, qnan = (type)NAN;			     \
	type a = (type)0.7, b = (type)12.4, r = fn(a, b);		     \
	type c = (type)0.75;						     \
									     \
	fails += expect(#fn "(inf,1)", fn(pinf, (type)1), pinf);		     \
	fails += expect(#fn "(-inf,1)", fn(-pinf, (type)1), pinf);	     \
	fails += expect(#fn "(inf,nan)", fn(pinf, qnan), pinf);		     \
	fails += expect(#fn "(-inf,nan)", fn(-pinf, qnan), pinf);		     \
	fails += expect(#fn "(nan,inf)", fn(qnan, pinf), pinf);		     \
	fails += expect(#fn "(nan,-inf)", fn(qnan, -pinf), pinf);		     \
	fails += expect(#fn "(nan,nan)", fn(qnan, qnan), qnan);		     \
									     \
	/* hypot (x,y) == hypot (+-x, +-y), either order */		     \
	fails += expect(#fn "(-x,y)", fn(-a, b), r);			     \
	fails += expect(#fn "(x,-y)", fn(a, -b), r);			     \
	fails += expect(#fn "(-x,-y)", fn(-a, -b), r);			     \
	fails += expect(#fn "(y,x)", fn(b, a), r);			     \
	fails += expect(#fn "(-y,x)", fn(-b, a), r);			     \
	fails += expect(#fn "(y,-x)", fn(b, -a), r);			     \
	fails += expect(#fn "(-y,-x)", fn(-b, -a), r);			     \
									     \
	/* hypot (x,0) == fabs (x) */					     \
	fails += expect(#fn "(0.75,0)", fn(c, (type)0), c);		     \
	fails += expect(#fn "(-0.75,0)", fn(-c, (type)0), c);		     \
	fails += expect(#fn "(-5.7e7,0)", fn((type)-5.7e7, (type)0),	     \
			(type)5.7e7);					     \
} while (0)

int main(void)
{
	int fails = 0;

	fails += ulp_sweep2_d("hypot", hypot, hypot_ref, NELEM(hypot_ref),
			      D_BUDGET, STEPS);
	fails += ulp_sweep2_f("hypotf", hypotf, hypotf_ref, NELEM(hypotf_ref),
			      F_BUDGET, STEPS);
	fails += ulp_sweep2_l("hypotl", hypotl, hypot_ref, NELEM(hypot_ref),
			      D_BUDGET, STEPS);

	SPECIALS(hypot, ulp_expect_d, double);
	SPECIALS(hypotf, ulp_expect_f, float);
	SPECIALS(hypotl, ulp_expect_l, long double);

	return fails != 0;
}
