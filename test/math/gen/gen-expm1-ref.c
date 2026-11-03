/* Host-side generator for the expm1 reference tables -- see README.md in this
   directory for what to link against and how the budgets in the test are
   derived.

   Two tables, as for exp: one for double, one for float, because uClibc-ng
   computes expm1f in double and converts (libm/float_wrappers.c).

   The point set has to cover both halves of the function.  For small |x| the
   implementations use a polynomial, and that is the whole reason expm1 exists
   -- exp(x)-1 would cancel there.  For large x it is exp with a correction.
   Half the points therefore sit in [-1,1] and half spread over the exponent
   range up to the overflow threshold.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_expm1(double);   /* SMALL (fdlibm) */
double gl_expm1(double);   /* OPTIMIZED (glibc) */
double cr_expm1(double);   /* ACCURATE, and the reference */
float cr_expm1f(float);    /* the reference for the float table */
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
/* x for index i of n: first half linear in [-1,1], second half by exponent */
static double point(int i, int n, double lo_exp, double hi_exp)
{
	int half = n / 2;
	if (i < half)
		return -1.0 + 2.0 * i / (half - 1);
	int j = i - half;
	int m = n - half;
	double e = lo_exp + (hi_exp - lo_exp) * j / (m - 1);
	double x = ldexp(1.0 + (j % 7) / 7.0, (int)e);
	return (j & 1) ? -x : x;
}
int main(int argc, char **argv)
{
	const int N = 1000, NF = 500;
	int gen = argc > 1;
	long nfd = 0, ngl = 0, maxfd = 0, maxgl = 0;
	long ffd = 0, fgl = 0, fcr = 0, mffd = 0, mfgl = 0, mfcr = 0;
	if (gen) printf("static const struct ulp_d1 expm1_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		/* exponents -60..9: down to where expm1(x) == x, up to below the
		   overflow threshold at 709.78 */
		double x = point(i, N, -60.0, 9.0);
		double want = cr_expm1(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long sf = steps(fd_expm1(x), want), sg = steps(gl_expm1(x), want);
		nfd += (sf != 0); ngl += (sg != 0);
		if (sf > maxfd) maxfd = sf;
		if (sg > maxgl) maxgl = sg;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 expm1f_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		/* the same shape in float range: overflow sits at 88.72 */
		float x = (float)point(i, NF, -30.0, 6.0);
		float want = cr_expm1f(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want);
		long sf = stepsf((float)fd_expm1(x), want);
		long sg = stepsf((float)gl_expm1(x), want);
		long sc = stepsf((float)cr_expm1(x), want);
		ffd += (sf != 0); fgl += (sg != 0); fcr += (sc != 0);
		if (sf > mffd) mffd = sf;
		if (sg > mfgl) mfgl = sg;
		if (sc > mfcr) mfcr = sc;
	}
	if (gen) printf("};\n");
	else {
		printf("double, of %d points, off by at least one step:\n  SMALL (fdlibm)    %ld  (worst %ld)\n  OPTIMIZED (glibc) %ld  (worst %ld)\n  ACCURATE          0  (the reference)\n",
		       N, nfd, maxfd, ngl, maxgl);
		printf("float, of %d points (double result rounded to float):\n  SMALL (fdlibm)    %ld  (worst %ld)\n  OPTIMIZED (glibc) %ld  (worst %ld)\n  ACCURATE          %ld  (worst %ld)\n",
		       NF, ffd, mffd, fgl, mfgl, fcr, mfcr);
	}
	return 0;
}
