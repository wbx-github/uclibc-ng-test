/* Host-side generator for the fma reference table -- see README.md in this
   directory for what to link against.

   fma is required correctly rounded, so the budget is zero in every setting and
   the point of the test is the ports that compute it in software.  That matters
   here more than anywhere: libm/s_fma.c used to be glibc's 1997 placeholder,
   return (x * y) + z, which is not a fused operation at all, and it went
   unnoticed for years because fma_test asked for fma(1,2,3) and a few NaNs.
   Where the hardware has the instruction the compiler emits it and nothing is
   being tested; where it does not, every one of these points goes through the
   integer path.

   The point set is built to separate a fused operation from a rounded one:

   - a third have z = -(x * y as a double), so the true result is exactly the
     rounding error of the product, and anything that rounds first returns zero;
   - a third cancel partially, z = -(x * y) * (1 +/- 2^-k);
   - the rest are a plain spread, plus the two cases where the product leaves
     the double range while the result does not -- an intermediate that
     overflows to infinity or underflows to zero cannot be recovered by an
     implementation that computes it separately.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <mpfr.h>

double fd_fma(double, double, double);

static long steps(double a, double b)
{
	union { double d; int64_t i; } ua = {a}, ub = {b};
	if (a == b) return 0;
	long d = ua.i - ub.i;
	return d < 0 ? -d : d;
}

static double ref(double x, double y, double z)
{
	mpfr_t a, b, c;
	double r;
	mpfr_init2 (a, 300); mpfr_init2 (b, 300); mpfr_init2 (c, 300);
	mpfr_set_d (a, x, MPFR_RNDN);
	mpfr_set_d (b, y, MPFR_RNDN);
	mpfr_set_d (c, z, MPFR_RNDN);
	mpfr_fma (a, a, b, c, MPFR_RNDN);
	r = mpfr_get_d (a, MPFR_RNDN);
	mpfr_clear (a); mpfr_clear (b); mpfr_clear (c);
	return r;
}

/* the two cases the product leaves the range: x*y overflows, and x*y underflows */
static const double edge[][3] = {
	{ 0x1.0000000000001p+512, 0x1p+512, -0x1.fffffffffffffp+1023 },
	{ -0x1.0000000000001p+512, 0x1p+512, 0x1.fffffffffffffp+1023 },
	{ 0x1.8p+600, 0x1.8p+600, -0x1.fffffffffffffp+1023 },
	{ 0x1.8p-537, 0x1.8p-537, -0x1p-1073 },
	{ 0x1p-600, 0x1p-600, 0x1p-1070 },
	{ 0x1.fffffffffffffp-1022, 0x1.fffffffffffffp-1022, 0x1p-1074 },
};
#define NEDGE ((int) (sizeof edge / sizeof *edge))

static void point(int i, int n, double *px, double *py, double *pz)
{
	int third = n / 3;
	double x, y;

	if (i < NEDGE) {
		*px = edge[i][0]; *py = edge[i][1]; *pz = edge[i][2];
		return;
	}
	x = 1.0 + (double)((i * 7) % 4093) / 4093.0;
	y = 1.0 + (double)((i * 13) % 3571) / 3571.0;
	x = ldexp (x, (i % 41) - 20);
	y = ldexp (y, (i % 37) - 18);
	if (i & 1) x = -x;
	if (i & 2) y = -y;

	if (i < third) {			/* full cancellation */
		*px = x; *py = y; *pz = -(x * y);
		return;
	}
	if (i < 2 * third) {			/* partial */
		int k = 8 + (i % 40);
		*px = x; *py = y;
		*pz = -(x * y) * (1.0 + ldexp (1.0, -k));
		return;
	}
	*px = x; *py = y;			/* plain */
	*pz = ldexp (1.0 + (double)(i % 97) / 97.0, (i % 23) - 11);
	if (i & 4) *pz = -*pz;
}

int main(int argc, char **argv)
{
	const int N = 999;
	int gen = argc > 1;
	long wrong = 0, worst = 0;

	if (gen) printf("static const struct ulp_d3 fma_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x, y, z, want;
		point (i, N, &x, &y, &z);
		want = ref (x, y, z);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a, %a, %a },\n", x, y, z, want);
		long s = steps (fd_fma(x, y, z), want);
		wrong += (s != 0);
		if (s > worst) worst = s;
	}
	if (gen) printf("};\n");
	else printf("of %d triples: libm/s_fma.c off at %ld (worst %ld)\n",
		    N, wrong, worst);
	return 0;
}
