/* Host-side generator for the exp reference tables -- see README.md in this
   directory for what to link against and how the budgets in the test are
   derived.  Emits the sample points with their correctly rounded results
   (CORE-MATH) and counts how often each of the three variants misses them.

   Two tables: one for double, and one for float, because uClibc-ng computes
   expf in double and converts (libm/w_expf.c).  That rounds twice, so the
   float budget is not the double one.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_exp(double);   /* SMALL */
double exp(double);      /* OPTIMIZED (ARM) */
double cr_exp(double);   /* ACCURATE, and the reference */
float cr_expf(float);    /* the reference for the float table */
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
int main(int argc, char **argv)
{
	const int N = 1000, NF = 500;
	int gen = argc > 1;
	long nfd = 0, narm = 0, maxfd = 0, maxarm = 0;
	long ffd = 0, farm = 0, fcr = 0, mffd = 0, mfarm = 0, mfcr = 0;
	if (gen) printf("static const struct ulp_d1 exp_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		/* cover -745..708: subnormal results at one end, no overflow at the other */
		double x = -745.0 + 1453.0 * i / (N - 1);
		double want = cr_exp(x);
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long sf = steps(fd_exp(x), want), sa = steps(exp(x), want);
		nfd += (sf != 0); narm += (sa != 0);
		if (sf > maxfd) maxfd = sf;
		if (sa > maxarm) maxarm = sa;
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 expf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		/* -103.9..88.7: subnormal float results at one end, no overflow at the other */
		float x = (float)(-103.9 + 192.6 * i / (NF - 1));
		float want = cr_expf(x);
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want);
		long sf = stepsf((float)fd_exp(x), want);
		long sa = stepsf((float)exp(x), want);
		long sc = stepsf((float)cr_exp(x), want);
		ffd += (sf != 0); farm += (sa != 0); fcr += (sc != 0);
		if (sf > mffd) mffd = sf;
		if (sa > mfarm) mfarm = sa;
		if (sc > mfcr) mfcr = sc;
	}
	if (gen) printf("};\n");
	else {
		printf("double, of %d points, off by at least one step:\n  SMALL (fdlibm)  %ld  (worst %ld)\n  OPTIMIZED (ARM) %ld  (worst %ld)\n  ACCURATE        0  (the reference)\n",
		       N, nfd, maxfd, narm, maxarm);
		printf("float, of %d points (double result rounded to float):\n  SMALL (fdlibm)  %ld  (worst %ld)\n  OPTIMIZED (ARM) %ld  (worst %ld)\n  ACCURATE        %ld  (worst %ld)\n",
		       NF, ffd, mffd, farm, mfarm, fcr, mfcr);
	}
	return 0;
}
