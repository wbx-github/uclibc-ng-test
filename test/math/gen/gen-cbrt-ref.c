/* Host-side generator for the cbrt, cosh and log1p reference tables -- see
   README.md in this directory.

   None of the three has an optimized or an accurate variant yet; CORE-MATH has
   all three.

   Where the points sit.  cbrt is smooth and what it needs is the exponent
   range, since it divides the exponent by three and the remainder decides which
   of three cases the mantissa takes.  cosh grows like exp/2 and overflows at
   710.48, so its points spread up to just below that.  log1p is the one with a
   hard end: it exists to be accurate for small x, where log(1+x) would cancel,
   so a third of its points sit within 2^-20 of zero and a third go down by
   exponent to 2^-1000.  */
#include "gen-common.h"
double fd_cbrt(double), fd_cosh(double), fd_log1p(double);
double cr_cbrt(double), cr_cosh(double), cr_log1p(double);
float cr_cbrtf(float), cr_coshf(float), cr_log1pf(float);

static double span(int i, int n, double lo, double hi)
{
	int third = n / 3;

	if (i < third)
		return lo + (hi - lo) * ldexp((double)(i % 21) + 1.0, -20);
	if (i < 2 * third) {
		int j = i - third;
		return lo + (hi - lo) * j / (third - 1);
	}
	{
		int j = i - 2 * third;
		int m = n - 2 * third;
		double e = -1000.0 + 1009.0 * j / (m - 1);
		double x = ldexp(1.0 + (j % 11) / 11.0, (int) e);
		return (lo < 0 && (j & 1)) ? -x : x;
	}
}

int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;

	if (!gen) printf("of %d points, off by at least one step:\n", N);
	GEN_SWEEP(cbrt, fd_cbrt, cr_cbrt, -8.0, 8.0, cbrt);
	GEN_SWEEP(cosh, fd_cosh, cr_cosh, -1.0, 1.0, cosh);
	GEN_SWEEP(log1p, fd_log1p, cr_log1p, 0.0, 1.0, log1p);
	GEN_SWEEPF(cbrt, fd_cbrt, cr_cbrtf, -8.0, 8.0, cbrt);
	GEN_SWEEPF(cosh, fd_cosh, cr_coshf, -1.0, 1.0, cosh);
	GEN_SWEEPF(log1p, fd_log1p, cr_log1pf, 0.0, 1.0, log1p);
	return 0;
}
