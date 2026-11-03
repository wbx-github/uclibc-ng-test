/* Host-side generator for the exp2 and exp10 reference tables -- see README.md
   in this directory for what to link against and how the budgets in the test
   are derived.

   Neither has an optimized or an accurate variant yet; Arm's optimized-routines
   carries portable scalar exp2 and exp10, so the optimized column is expected
   to change.

   exp2 is the odd one: uClibc-ng has no implementation of it at all.
   libm/w_exp2.c computes pow(2.0, x), so what this measures is pow at base two,
   and that is why its count is the highest here.

   Half the points sit in [-1,1], where the implementations use a polynomial;
   half spread over the exponent range up to just below the overflow threshold,
   which is 1024 for exp2 and 308.25 for exp10.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_exp2(double), fd_exp10(double);
double cr_exp2(double), cr_exp10(double);
float cr_exp2f(float), cr_exp10f(float);

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

/* first half linear in [-1,1], second half linear out to the limit */
static double point(int i, int n, double lim)
{
	int half = n / 2;
	if (i < half)
		return -1.0 + 2.0 * i / (half - 1);
	int j = i - half;
	int m = n - half;
	double x = 1.0 + (lim - 1.0) * j / (m - 1);
	return (j & 1) ? -x : x;
}

#define SWEEP(nm, fdfn, crfn, lim)					\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_d1 " #nm "_ref[] = {\n"); \
	for (int i = 0; i < N; i++) {					\
		double x = point(i, N, lim);				\
		double want = crfn(x);					\
		if (!isfinite(want) || want == 0) continue;		\
		if (gen) printf("\t{ %a, %a },\n", x, want);		\
		long s = steps(fdfn(x), want);				\
		wrong += (s != 0);					\
		if (s > worst) worst = s;				\
	}								\
	if (gen) printf("};\n\n");					\
	else printf("  %-6s fdlibm %ld  (worst %ld)   CORE-MATH 0 (the reference)\n", \
		    #nm, wrong, worst);					\
} while (0)

#define SWEEPF(nm, fdfn, crfn, lim)					\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_f1 " #nm "f_ref[] = {\n"); \
	for (int i = 0; i < NF; i++) {					\
		float x = (float)point(i, NF, lim);			\
		float want = crfn(x);					\
		if (!isfinite(want) || want == 0) continue;		\
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
	const int N = 1000, NF = 500;
	int gen = argc > 1;

	if (!gen) printf("double, of %d points, off by at least one step:\n", N);
	SWEEP(exp2, fd_exp2, cr_exp2, 1023.0);
	SWEEP(exp10, fd_exp10, cr_exp10, 308.0);
	if (!gen) printf("float, of %d points (double result rounded to float):\n", NF);
	SWEEPF(exp2, fd_exp2, cr_exp2f, 127.0);
	SWEEPF(exp10, fd_exp10, cr_exp10f, 38.0);
	return 0;
}
