/* Host-side generator for the atan, atan2, asin and acos reference tables --
   see README.md in this directory.

   None of the four has an optimized or an accurate variant yet; CORE-MATH has
   all four, so the accurate column is where they would go.

   Where the points sit.  asin and acos live on [-1, 1] and are steepest at the
   ends, where the derivative is infinite and an implementation has to change
   method -- a third of their points sit within 2^-20 of +/-1 for that reason.
   atan is tame on [-1, 1] and reduces by 1/x outside it, so a third of its
   points cover the reduction boundary and a third spread to 2^60, where the
   answer approaches pi/2 and everything rides on how that constant is carried.
   atan2 takes the same arguments in pairs, with the quadrant chosen by their
   signs.  */
#include "gen-common.h"
double fd_atan(double), fd_asin(double), fd_acos(double), fd_atan2(double, double);
double cr_atan(double), cr_asin(double), cr_acos(double), cr_atan2(double, double);
float cr_atanf(float), cr_asinf(float), cr_acosf(float), cr_atan2f(float, float);

/* a third near lo, a third linear, a third out to hi by exponent */
static double span(int i, int n, double lo, double hi)
{
	int third = n / 3;

	if (i < third)			/* crowded at the near end */
		return lo + (hi - lo) * ldexp((double)(i % 21) + 1.0, -20);
	if (i < 2 * third) {
		int j = i - third;
		return lo + (hi - lo) * j / (third - 1);
	}
	{
		int j = i - 2 * third;
		int m = n - 2 * third;
		double e = -30.0 + 90.0 * j / (m - 1);
		double x = ldexp(1.0 + (j % 11) / 11.0, (int) e);
		return (j & 1) ? -x : x;
	}
}

int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;

	if (!gen) printf("of %d points, off by at least one step:\n", N);
	GEN_SWEEP(asin, fd_asin, cr_asin, -1.0, 1.0, asin);
	GEN_SWEEP(acos, fd_acos, cr_acos, -1.0, 1.0, acos);
	GEN_SWEEP(atan, fd_atan, cr_atan, -1.0, 1.0, atan);
	GEN_SWEEPF(asin, fd_asin, cr_asinf, -1.0, 1.0, asin);
	GEN_SWEEPF(acos, fd_acos, cr_acosf, -1.0, 1.0, acos);
	GEN_SWEEPF(atan, fd_atan, cr_atanf, -1.0, 1.0, atan);

	{	/* atan2 in pairs */
		long wrong = 0, worst = 0;
		if (gen) printf("static const struct ulp_d2 atan2_ref[] = {\n");
		for (int i = 0; i < N; i++) {
			double y = span(i, N, -1.0, 1.0);
			double x = span((i * 7 + 3) % N, N, -1.0, 1.0);
			double want = cr_atan2(y, x);
			if (!isfinite(want) || (y == 0 && x == 0)) continue;
			if (gen) printf("\t{ %a, %a, %a },\n", y, x, want);
			long s = g_steps(fd_atan2(y, x), want);
			wrong += (s != 0);
			if (s > worst) worst = s;
		}
		if (gen) printf("};\n\n");
		else printf("  atan2  fdlibm %ld  (worst %ld)\n", wrong, worst);
	}
	{
		long wrong = 0, worst = 0;
		if (gen) printf("static const struct ulp_f2 atan2f_ref[] = {\n");
		for (int i = 0; i < NF; i++) {
			float y = (float) span(i, NF, -1.0, 1.0);
			float x = (float) span((i * 7 + 3) % NF, NF, -1.0, 1.0);
			float want = cr_atan2f(y, x);
			if (!isfinite(want) || (y == 0 && x == 0)) continue;
			if (gen) printf("\t{ %a, %a, %a },\n", (double) y, (double) x, (double) want);
			long s = g_stepsf((float) fd_atan2(y, x), want);
			wrong += (s != 0);
			if (s > worst) worst = s;
		}
		if (gen) printf("};\n");
		else printf("  atan2f fdlibm %ld  (worst %ld)\n", wrong, worst);
	}
	return 0;
}
