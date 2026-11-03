/* Host-side generator for the log, log10 and log2 reference tables -- see
   README.md in this directory for what to link against and how the budgets in
   the test are derived.

   None of the three has an optimized or an accurate variant yet.  Arm's
   optimized-routines does carry portable scalar log and log2, so the optimized
   column is expected to change; until it does, all three settings are the same
   fdlibm code.

   Two regions matter.  Around 1 the logarithm cancels -- log(1+e) is e to first
   order, so the leading bits of the argument carry no information and the
   implementation has to work in the tail; fdlibm splits the argument there and
   this is where it loses its last bit.  Away from 1 the function is tame and
   what is being tested is the reduction by the exponent.  So a third of the
   points sit within 2^-20 of 1, a third in [0.5, 2], and a third spread over
   the whole exponent range from 2^-1022 to 2^1023.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_log(double), fd_log10(double), fd_log2(double);
double cr_log(double), cr_log10(double), cr_log2(double);
float cr_logf(float), cr_log10f(float), cr_log2f(float);

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

static double point(int i, int n, double lo_exp, double hi_exp)
{
	int third = n / 3;
	if (i < third)			/* within 2^-20 of 1, both sides */
		return 1.0 + (double)(i - third / 2) * 0x1p-20 / (third / 2);
	if (i < 2 * third) {		/* [0.5, 2] */
		int j = i - third;
		return 0.5 + 1.5 * j / (third - 1);
	}
	int j = i - 2 * third;		/* the exponent range */
	int m = n - 2 * third;
	double e = lo_exp + (hi_exp - lo_exp) * j / (m - 1);
	return ldexp(1.0 + (j % 11) / 11.0, (int)e);
}

#define SWEEP(nm, fdfn, crfn)						\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_d1 " #nm "_ref[] = {\n"); \
	for (int i = 0; i < N; i++) {					\
		double x = point(i, N, -1022.0, 1023.0);			\
		double want = crfn(x);					\
		if (!isfinite(want) || x <= 0) continue;		\
		if (gen) printf("\t{ %a, %a },\n", x, want);		\
		long s = steps(fdfn(x), want);				\
		wrong += (s != 0);					\
		if (s > worst) worst = s;				\
	}								\
	if (gen) printf("};\n\n");					\
	else printf("  %-6s fdlibm %ld  (worst %ld)   CORE-MATH 0 (the reference)\n", \
		    #nm, wrong, worst);					\
} while (0)

#define SWEEPF(nm, fdfn, crfn)						\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_f1 " #nm "f_ref[] = {\n"); \
	for (int i = 0; i < NF; i++) {					\
		float x = (float)point(i, NF, -126.0, 127.0);		\
		float want = crfn(x);					\
		if (!isfinite(want) || x <= 0) continue;		\
		if (gen) printf("\t{ %a, %a },\n", (double)x, (double)want); \
		long s = stepsf((float)fdfn(x), want);			\
		wrong += (s != 0);					\
		if (s > worst) worst = s;				\
	}								\
	if (gen) printf("};\n\n");					\
	else printf("  %-6s fdlibm %ld  (worst %ld)\n", #nm "f", wrong, worst); \
} while (0)

int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;

	if (!gen) printf("double, of %d points, off by at least one step:\n", N);
	SWEEP(log, fd_log, cr_log);
	SWEEP(log10, fd_log10, cr_log10);
	SWEEP(log2, fd_log2, cr_log2);
	if (!gen) printf("float, of %d points (double result rounded to float):\n", NF);
	SWEEPF(log, fd_log, cr_logf);
	SWEEPF(log10, fd_log10, cr_log10f);
	SWEEPF(log2, fd_log2, cr_log2f);
	return 0;
}
