/* Host-side generator for the sin and cos reference tables -- see README.md in
   this directory for what to link against and how the budgets in the test are
   derived.

   There is no optimized variant of either function: Arm's optimized-routines
   has no portable scalar sin or cos, and glibc's is the IBM Accurate
   Mathematical Library, which aims at accuracy rather than speed.  So the small
   and the optimized setting are the same fdlibm code and share a budget.

   The point set has to exercise the argument reduction, which is where these
   functions actually differ.  A third of the points sit in [-2pi, 2pi], where
   fdlibm reduces with a two-word pi/2; a third in [2pi, 2^20], where it still
   uses that path; and a third spread over the exponent range up to 2^1023,
   where it has to fall back on __kernel_rem_pio2 with its 396-bit pi.  Points
   near a multiple of pi/2 are the hard ones -- the result is then a difference
   of nearly equal numbers -- so each block walks a small offset around those
   multiples instead of sampling evenly.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_sin(double), fd_cos(double);   /* SMALL and OPTIMIZED (fdlibm) */
double cr_sin(double), cr_cos(double);   /* ACCURATE, and the reference */
float cr_sinf(float), cr_cosf(float);    /* the reference for the float tables */

static long steps(double a, double b)
{
	union { double d; int64_t i; } ua = {a}, ub = {b};
	if (a == b) return 0;
	long d = ua.i - ub.i;
	return d < 0 ? -d : d;
}
static long stepsf(float a, float b)
{
	union { float f; int32_t i; } ua = {a}, ub = {b};
	if (a == b) return 0;
	long d = ua.i - ub.i;
	return d < 0 ? -d : d;
}

/* Three blocks, as described above.  Inside a block the point is a multiple of
   pi/2 plus a small offset, so that the reduction has to work for its result. */
static double point(int i, int n, double hi_exp)
{
	static const double pio2 = 1.57079632679489661923;
	int third = n / 3;
	if (i < third) {
		/* [-2pi, 2pi]: k*pi/2 + offset, k = -4..4 */
		int k = -4 + (i % 9);
		double off = (i % 37 - 18) * 0x1p-6;
		return k * pio2 + off;
	}
	if (i < 2 * third) {
		int j = i - third;
		double k = 1.0 + (double)j * 1000.0;
		double off = (j % 41 - 20) * 0x1p-9;
		return (j & 1 ? -1.0 : 1.0) * (k * pio2 + off);
	}
	int j = i - 2 * third;
	int m = n - 2 * third;
	double e = 20.0 + (hi_exp - 20.0) * j / (m - 1);
	double x = ldexp(1.0 + (j % 13) / 13.0, (int)e);
	return (j & 1) ? -x : x;
}

int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;
	long nsin = 0, ncos = 0, msin = 0, mcos = 0;
	long fsin = 0, fcos = 0, mfsin = 0, mfcos = 0;

	if (gen) printf("static const struct ulp_d1 sin_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point(i, N, 1023.0);
		double want = cr_sin(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long s = steps(fd_sin(x), want);
		nsin += (s != 0);
		if (s > msin) msin = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_d1 cos_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point(i, N, 1023.0);
		double want = cr_cos(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long s = steps(fd_cos(x), want);
		ncos += (s != 0);
		if (s > mcos) mcos = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 sinf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float)point(i, NF, 127.0);
		float want = cr_sinf(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want);
		long s = stepsf((float)fd_sin(x), want);
		fsin += (s != 0);
		if (s > mfsin) mfsin = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 cosf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float)point(i, NF, 127.0);
		float want = cr_cosf(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want);
		long s = stepsf((float)fd_cos(x), want);
		fcos += (s != 0);
		if (s > mfcos) mfcos = s;
	}
	if (gen) printf("};\n");
	else {
		printf("double, of %d points, off by at least one step:\n"
		       "  sin  SMALL/OPTIMIZED (fdlibm) %ld  (worst %ld)   ACCURATE 0 (the reference)\n"
		       "  cos  SMALL/OPTIMIZED (fdlibm) %ld  (worst %ld)   ACCURATE 0 (the reference)\n",
		       N, nsin, msin, ncos, mcos);
		printf("float, of %d points (double result rounded to float):\n"
		       "  sinf %ld  (worst %ld)\n  cosf %ld  (worst %ld)\n",
		       NF, fsin, mfsin, fcos, mfcos);
	}
	return 0;
}
