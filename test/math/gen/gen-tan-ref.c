/* Host-side generator for the tan reference table -- see README.md in this
   directory for what to link against and how the budgets in the test are
   derived.

   There is no optimized and no accurate variant: Arm's optimized-routines has
   no portable scalar tan, and CORE-MATH's carries its argument reduction in a
   real 128-bit integer, which most of our 32-bit toolchains cannot compile.  So
   all three settings are the same fdlibm code.

   The point set is the one gen-sin-ref.c uses, for the same reason: what
   separates implementations here is the argument reduction.  A third of the
   points sit in [-2pi, 2pi], a third out to 2^20, and a third spread to 2^1023
   where fdlibm has to use __kernel_rem_pio2 with its 396-bit pi.  Inside a
   block the point is a multiple of pi/2 plus a small offset, which for tan
   means the hard cases are near its poles rather than near a zero -- the
   offsets stay clear of the pole itself, so every result is finite.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_tan(double);   /* every setting (fdlibm) */
double cr_tan(double);   /* the reference */
float cr_tanf(float);    /* the reference for the float table */

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

/* The same three blocks as gen-sin-ref.c. */
static double point(int i, int n, double hi_exp)
{
	static const double pio2 = 1.57079632679489661923;
	int third = n / 3;
	if (i < third) {
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
	long nd = 0, md = 0, nf = 0, mf = 0;

	if (gen) printf("static const struct ulp_d1 tan_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point(i, N, 1023.0);
		double want = cr_tan(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long s = steps(fd_tan(x), want);
		nd += (s != 0);
		if (s > md) md = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 tanf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float)point(i, NF, 127.0);
		float want = cr_tanf(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want);
		long s = stepsf((float)fd_tan(x), want);
		nf += (s != 0);
		if (s > mf) mf = s;
	}
	if (gen) printf("};\n");
	else {
		printf("double, of %d points, off by at least one step:\n"
		       "  tan  fdlibm %ld  (worst %ld)   CORE-MATH 0 (the reference)\n",
		       N, nd, md);
		printf("float, of %d points (double result rounded to float):\n"
		       "  tanf %ld  (worst %ld)\n", NF, nf, mf);
	}
	return 0;
}
