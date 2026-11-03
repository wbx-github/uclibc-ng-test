/* Host-side generator for the sinh and tanh reference tables -- see README.md in
   this directory for what to link against and how the budgets in the test are
   derived.

   Neither has an optimized variant: Arm's optimized-routines has no portable
   scalar sinh or tanh, and glibc's are the same fdlibm code we ship.  So the
   small and the optimized setting share a budget.

   Both functions are built out of exp, and that is what the point set has to
   reach.  Half the points sit in [-1,1], where fdlibm uses expm1 and a
   polynomial and where cancellation matters; the other half spread over the
   exponent range.  sinh runs up to 2^9, just under its overflow at 710.48;
   tanh only to 2^5, because past |x| = 22 it is exactly +/-1 and every
   implementation agrees.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_sinh(double), fd_tanh(double);  /* SMALL and OPTIMIZED (fdlibm) */
double cr_sinh(double), cr_tanh(double);  /* ACCURATE, and the reference */
float cr_sinhf(float), cr_tanhf(float);   /* the reference for the float tables */

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
	long ns = 0, nt = 0, ms = 0, mt = 0;
	long fs = 0, ft = 0, mfs = 0, mft = 0;

	if (gen) printf("static const struct ulp_d1 sinh_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point(i, N, -60.0, 9.0);
		double want = cr_sinh(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long s = steps(fd_sinh(x), want);
		ns += (s != 0);
		if (s > ms) ms = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_d1 tanh_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point(i, N, -60.0, 5.0);
		double want = cr_tanh(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long s = steps(fd_tanh(x), want);
		nt += (s != 0);
		if (s > mt) mt = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 sinhf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float)point(i, NF, -30.0, 6.0);
		float want = cr_sinhf(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want);
		long s = stepsf((float)fd_sinh(x), want);
		fs += (s != 0);
		if (s > mfs) mfs = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 tanhf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float)point(i, NF, -30.0, 4.0);
		float want = cr_tanhf(x);
		if (!isfinite(want)) continue;
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want);
		long s = stepsf((float)fd_tanh(x), want);
		ft += (s != 0);
		if (s > mft) mft = s;
	}
	if (gen) printf("};\n");
	else {
		printf("double, of %d points, off by at least one step:\n"
		       "  sinh SMALL/OPTIMIZED (fdlibm) %ld  (worst %ld)   ACCURATE 0 (the reference)\n"
		       "  tanh SMALL/OPTIMIZED (fdlibm) %ld  (worst %ld)   ACCURATE 0 (the reference)\n",
		       N, ns, ms, nt, mt);
		printf("float, of %d points (double result rounded to float):\n"
		       "  sinhf %ld  (worst %ld)\n  tanhf %ld  (worst %ld)\n",
		       NF, fs, mfs, ft, mft);
	}
	return 0;
}
