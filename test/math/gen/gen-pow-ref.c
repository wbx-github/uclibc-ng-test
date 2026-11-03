/* Host-side generator for the pow reference table -- see README.md in this
   directory for what to link against and how the budget in the test is derived.

   No optimized or accurate variant yet; Arm's optimized-routines carries a
   portable scalar pow, so the optimized column is expected to change.

   pow is the one function here with two arguments, and its hard cases come in
   three shapes, one block each.  Small integer exponents with the base near 1,
   where the result is a short product and the implementation should be exact.
   A base within 2^-20 of 1 with a large exponent, where log(x) cancels and the
   whole result rides on the tail bits -- this is the block that separates
   implementations.  And a spread over the exponent range with a small
   non-integer exponent, which exercises the reduction.  Bases are kept positive
   throughout: a negative base with a non-integer exponent is a domain error,
   and the special cases the test checks cover that side.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_pow(double, double);
double cr_pow(double, double);
float cr_powf(float, float);

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

static void point(int i, int n, double *px, double *py)
{
	int third = n / 3;
	if (i < third) {			/* small integer exponents */
		*px = 0.5 + 1.5 * (i % 31) / 30.0;
		*py = (double)(i % 41 - 20);
		return;
	}
	if (i < 2 * third) {			/* base near 1, large exponent */
		int j = i - third;
		*px = 1.0 + (double)(j % 63 - 31) * 0x1p-20;
		*py = ldexp(1.0 + (j % 7) / 7.0, j % 21);
		if (j & 1) *py = -*py;
		return;
	}
	int j = i - 2 * third;			/* exponent range, small y */
	int m = n - 2 * third;
	double e = -1000.0 + 2000.0 * j / (m - 1);
	*px = ldexp(1.0 + (j % 11) / 11.0, (int)e);
	*py = 0.25 + 1.5 * (j % 17) / 17.0;
	if (j & 1) *py = -*py;
}

int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;
	long wrong = 0, worst = 0, fw = 0, fworst = 0;

	if (gen) printf("static const struct ulp_d2 pow_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x, y, want;
		point(i, N, &x, &y);
		want = cr_pow(x, y);
		if (!isfinite(want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a, %a },\n", x, y, want);
		long s = steps(fd_pow(x, y), want);
		wrong += (s != 0);
		if (s > worst) worst = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f2 powf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		double dx, dy;
		point(i, NF, &dx, &dy);
		float x = (float)dx, y = (float)dy;
		float want = cr_powf(x, y);
		if (!isfinite(want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a, %a },\n", (double)x, (double)y, (double)want);
		long s = stepsf((float)fd_pow(x, y), want);
		fw += (s != 0);
		if (s > fworst) fworst = s;
	}
	if (gen) printf("};\n");
	else {
		printf("double, of %d points, off by at least one step:\n"
		       "  pow  fdlibm %ld  (worst %ld)   CORE-MATH 0 (the reference)\n",
		       N, wrong, worst);
		printf("float, of %d points (double result rounded to float):\n"
		       "  powf fdlibm %ld  (worst %ld)\n", NF, fw, fworst);
	}
	return 0;
}
