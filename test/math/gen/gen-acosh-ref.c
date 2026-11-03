/* Host-side generator for the acosh, asinh and atanh reference tables -- see
   README.md in this directory.

   None of the three has an optimized or an accurate variant yet; CORE-MATH has
   all three.

   Where the points sit.  Each has one hard end.  acosh is defined from 1 up and
   its derivative is infinite there, so a third of its points sit within 2^-20
   of 1 and the rest spread to 2^500, where it becomes log(2x).  atanh runs the
   other way: it is tame at 0 and goes to infinity at +/-1, so a third of its
   points crowd the ends.  asinh is smooth everywhere, and what it needs is the
   range where log1p takes over from the series, so its points spread by
   exponent from 2^-30 to 2^500.  */
#include "gen-common.h"
double fd_acosh(double), fd_asinh(double), fd_atanh(double);
double cr_acosh(double), cr_asinh(double), cr_atanh(double);
float cr_acoshf(float), cr_asinhf(float), cr_atanhf(float);

/* lo and hi bracket the interesting end; a third crowd it, a third are linear
   between, a third spread by exponent */
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
		double e = -30.0 + 530.0 * j / (m - 1);
		double x = ldexp(1.0 + (j % 11) / 11.0, (int) e);
		return (lo < 0 && (j & 1)) ? -x : x;
	}
}

int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;

	if (!gen) printf("of %d points, off by at least one step:\n", N);
	GEN_SWEEP(acosh, fd_acosh, cr_acosh, 1.0, 3.0, acosh);
	GEN_SWEEP(asinh, fd_asinh, cr_asinh, -2.0, 2.0, asinh);
	GEN_SWEEP(atanh, fd_atanh, cr_atanh, 1.0, -1.0, atanh);
	GEN_SWEEPF(acosh, fd_acosh, cr_acoshf, 1.0, 3.0, acosh);
	GEN_SWEEPF(asinh, fd_asinh, cr_asinhf, -2.0, 2.0, asinh);
	GEN_SWEEPF(atanh, fd_atanh, cr_atanhf, 1.0, -1.0, atanh);
	return 0;
}
