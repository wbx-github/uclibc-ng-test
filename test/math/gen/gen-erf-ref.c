/* Host-side generator for the erf and erfc reference tables -- see README.md in
   this directory for what to link against and how the budgets in the test are
   derived.

   Four tables: erf and erfc, each for double and float, because uClibc-ng
   computes erff and erfcf in double and converts (libm/float_wrappers.c).

   The point set covers both regimes.  erf is interesting near zero, where it is
   almost linear and the small-argument path runs, and up to 6, past which it
   rounds to 1.  erfc keeps going: it stays non-zero out to 27.25, and the
   implementations switch to an asymptotic form on the way, so the second half of
   the set spreads over the exponent range instead of the value range.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_erf(double), fd_erfc(double);   /* SMALL (fdlibm) */
double arm_erf(double);                   /* OPTIMIZED for erf (Arm) */
double cr_erf(double), cr_erfc(double);   /* ACCURATE, and the reference */
float cr_erff(float), cr_erfcf(float);    /* the reference for the float tables */
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
/* first half linear in [-lin,lin], second half by exponent up to hi_exp */
static double point(int i, int n, double lin, double lo_exp, double hi_exp)
{
	int half = n / 2;
	if (i < half)
		return -lin + 2.0 * lin * i / (half - 1);
	int j = i - half;
	int m = n - half;
	double e = lo_exp + (hi_exp - lo_exp) * j / (m - 1);
	double x = ldexp (1.0 + (j % 7) / 7.0, (int) e);
	return (j & 1) ? -x : x;
}
int main(int argc, char **argv)
{
	const int N = 1000, NF = 500;
	int gen = argc > 1;
	long e_fd = 0, e_arm = 0, e_mfd = 0, e_marm = 0;
	long c_fd = 0, c_mfd = 0;
	long ef_fd = 0, ef_cr = 0, ef_mfd = 0, ef_mcr = 0;
	long cf_fd = 0, cf_cr = 0, cf_mfd = 0, cf_mcr = 0;

	if (gen) printf("static const struct ulp_d1 erf_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point (i, N, 6.0, -60.0, 2.0);
		double want = cr_erf (x);
		if (!isfinite (want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long sf = steps (fd_erf (x), want), sa = steps (arm_erf (x), want);
		e_fd += (sf != 0); e_arm += (sa != 0);
		if (sf > e_mfd) e_mfd = sf;
		if (sa > e_marm) e_marm = sa;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 erff_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float) point (i, NF, 6.0, -30.0, 2.0);
		float want = cr_erff (x);
		if (!isfinite (want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a },\n", (double) x, (double) want);
		long sf = stepsf ((float) fd_erf (x), want);
		long sc = stepsf ((float) cr_erf (x), want);
		ef_fd += (sf != 0); ef_cr += (sc != 0);
		if (sf > ef_mfd) ef_mfd = sf;
		if (sc > ef_mcr) ef_mcr = sc;
	}
	if (gen) printf("};\n\nstatic const struct ulp_d1 erfc_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		/* erfc is non-zero out to 27.25, so the exponent half reaches 4 */
		double x = point (i, N, 6.0, -60.0, 4.0);
		if (x > 27.0) continue;
		double want = cr_erfc (x);
		if (!isfinite (want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long sf = steps (fd_erfc (x), want);
		c_fd += (sf != 0);
		if (sf > c_mfd) c_mfd = sf;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 erfcf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float) point (i, NF, 6.0, -30.0, 3.0);
		if (x > 10.0) continue;   /* erfcf underflows above 10.54 */
		float want = cr_erfcf (x);
		if (!isfinite (want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a },\n", (double) x, (double) want);
		long sf = stepsf ((float) fd_erfc (x), want);
		long sc = stepsf ((float) cr_erfc (x), want);
		cf_fd += (sf != 0); cf_cr += (sc != 0);
		if (sf > cf_mfd) cf_mfd = sf;
		if (sc > cf_mcr) cf_mcr = sc;
	}
	if (gen) printf("};\n");
	else {
		printf("erf   double: SMALL %ld off (worst %ld), OPTIMIZED (Arm) %ld (worst %ld), ACCURATE 0\n",
		       e_fd, e_mfd, e_arm, e_marm);
		printf("erf   float : SMALL %ld off (worst %ld), ACCURATE %ld (worst %ld)\n",
		       ef_fd, ef_mfd, ef_cr, ef_mcr);
		printf("erfc  double: SMALL %ld off (worst %ld), OPTIMIZED = SMALL, ACCURATE 0\n",
		       c_fd, c_mfd);
		printf("erfc  float : SMALL %ld off (worst %ld), ACCURATE %ld (worst %ld)\n",
		       cf_fd, cf_mfd, cf_cr, cf_mcr);
	}
	return 0;
}
