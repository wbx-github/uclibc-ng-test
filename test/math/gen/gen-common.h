/* Shared by the generators added on 2026-08-24 -- see README.md. */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

static long g_steps(double a, double b)
{
	union { double d; int64_t i; } ua = {a}, ub = {b};
	if (a == b) return 0;
	if ((a < 0) != (b < 0)) return 1 << 30;	/* straddles zero */
	long d = ua.i - ub.i;
	return d < 0 ? -d : d;
}
static long g_stepsf(float a, float b)
{
	union { float f; int32_t i; } ua = {a}, ub = {b};
	if (a == b) return 0;
	if ((a < 0) != (b < 0)) return 1 << 30;
	long d = ua.i - ub.i;
	return d < 0 ? -d : d;
}

#define GEN_SWEEP(nm, fdfn, crfn, lo, hi, tbl)				\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_d1 " #tbl "_ref[] = {\n"); \
	for (int i = 0; i < N; i++) {					\
		double x = span(i, N, lo, hi);				\
		double want = crfn(x);					\
		if (!isfinite(want)) continue;				\
		if (gen) printf("\t{ %a, %a },\n", x, want);		\
		long s = g_steps(fdfn(x), want);			\
		wrong += (s != 0);					\
		if (s > worst) worst = s;				\
	}								\
	if (gen) printf("};\n\n");					\
	else printf("  %-6s fdlibm %ld  (worst %ld)\n", #nm, wrong, worst); \
} while (0)

#define GEN_SWEEPF(nm, fdfn, crfn, lo, hi, tbl)				\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_f1 " #tbl "f_ref[] = {\n"); \
	for (int i = 0; i < NF; i++) {					\
		float x = (float) span(i, NF, lo, hi);			\
		float want = crfn(x);					\
		if (!isfinite(want)) continue;				\
		if (gen) printf("\t{ %a, %a },\n", (double) x, (double) want); \
		long s = g_stepsf((float) fdfn(x), want);		\
		wrong += (s != 0);					\
		if (s > worst) worst = s;				\
	}								\
	if (gen) printf("};\n\n");					\
	else printf("  %-6s fdlibm %ld  (worst %ld)\n", #nm "f", wrong, worst); \
} while (0)
