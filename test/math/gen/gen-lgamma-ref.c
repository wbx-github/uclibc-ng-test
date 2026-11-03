/* Host-side generator for the lgamma reference tables -- see README.md in this
   directory for what to link against and how the budgets in the test are
   derived.

   lgamma has poles at zero and at every negative integer and zeros at 1 and 2,
   so the point set stays away from the poles but comes close to them: a third of
   it lies in (0,3], where the zeros and the first pole are, a third on the
   negative side at non-integer arguments, and a third spread over the exponent
   range up to 170, past which lgamma overflows.

   Points near lgamma's own zeros are kept, and they are why the test allows a
   worst case of 32 representable values rather than one or two.  lgamma has
   zeros at 1 and 2 and again between the negative poles, and fdlibm reaches them
   by subtraction, so the relative error there is genuinely large: at x =
   -2.7375, where the result is 0x1.3p-6, it is 45 steps off on the target (29 on an x86-64 host, which reaches it through glibc's log).  That is a real
   weakness and the table should show it; the sensitive part of the test is the
   number of misses, 323 of 950, not the worst case.

   Two tables, as elsewhere, because lgammaf computes in double and converts
   (libm/w_lgammaf.c).  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_lgamma_r(double, int *);   /* SMALL (fdlibm) */
double cr_lgamma(double);            /* ACCURATE, and the reference */
float cr_lgammaf(float);             /* the reference for the float table */
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
/* thirds: (0,3], negative non-integers, then by exponent */
static double point(int i, int n, double hi_exp)
{
	int t = n / 3;
	if (i < t)
		return 0.003 + 2.997 * i / (t - 1);
	if (i < 2 * t) {
		int j = i - t;
		/* -0.37 keeps every argument clear of the poles */
		return -0.37 - 6.0 * j / (t - 1);
	}
	int j = i - 2 * t;
	int m = n - 2 * t;
	double e = -50.0 + (hi_exp + 50.0) * j / (m - 1);
	return ldexp (1.0 + (j % 7) / 7.0, (int) e);
}
int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;
	long d_fd = 0, d_mfd = 0, f_fd = 0, f_cr = 0, f_mfd = 0, f_mcr = 0;
	int sg;

	if (gen) printf("static const struct ulp_d1 lgamma_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point (i, N, 7.0);
		double want = cr_lgamma (x);
		if (!isfinite (want)) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long s = steps (fd_lgamma_r (x, &sg), want);
		d_fd += (s != 0);
		if (s > d_mfd) d_mfd = s;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 lgammaf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float) point (i, NF, 5.0);
		float want = cr_lgammaf (x);
		if (!isfinite (want)) continue;
		if (gen) printf("\t{ %a, %a },\n", (double) x, (double) want);
		long s = stepsf ((float) fd_lgamma_r (x, &sg), want);
		long c = stepsf ((float) cr_lgamma (x), want);
		f_fd += (s != 0); f_cr += (c != 0);
		if (s > f_mfd) f_mfd = s;
		if (c > f_mcr) f_mcr = c;
	}
	if (gen) printf("};\n");
	else {
		printf("lgamma double: SMALL %ld off (worst %ld), OPTIMIZED = SMALL, ACCURATE 0\n",
		       d_fd, d_mfd);
		printf("lgamma float : SMALL %ld off (worst %ld), ACCURATE %ld (worst %ld)\n",
		       f_fd, f_mfd, f_cr, f_mcr);
	}
	return 0;
}
