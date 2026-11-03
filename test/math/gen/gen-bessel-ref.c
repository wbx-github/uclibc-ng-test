/* Host-side generator for the Bessel reference tables -- see README.md in this
   directory for what to link against and how the budgets in the test are
   derived.

   This one is the exception among the generators: its reference does not come
   from CORE-MATH, which has no Bessel functions, but from MPFR.  Nor is there
   anything else to compare against -- glibc and musl both ship the same fdlibm
   code we do -- so mpfr_j0 and its siblings at 300 bits, rounded to double, are
   the only way to say what the right answer is.  MPFR is needed to regenerate
   the tables, not to build or run the test.

   Where the points sit.  These functions oscillate and their zeros are where an
   implementation loses the most: near a zero the result is a difference of
   nearly equal terms, and fdlibm switches from its rational approximation to an
   asymptotic form at |x| = 8 and again where it starts using the phase.  So half
   the points cover [0, 60] densely, which holds about nineteen zeros of j0, and
   half spread out to 2^60, where everything rides on the argument reduction of
   the phase.  y0 and y1 are -inf at zero and undefined to the left of it, so
   their points start just above zero.  jn and yn get their own tables with the
   order beside the argument; orders stay small because fdlibm's recurrence is
   what is being tested, not overflow.

   Nothing is dropped.  Measuring these functions in ulp of their own result
   would be meaningless near a zero -- at the first zero of j0 the correctly
   rounded value is -6.1e-17 and fdlibm is four hundred million million
   representable values away, as would be anything that does not carry extra
   precision there -- so the test measures against the local scale instead, the
   envelope below.  That keeps every point and still shows a real defect: jn
   normalises with j0 (e_jn.c, b = t * __ieee754_j0(x) / b) and inherits its
   relative error, which near a zero of j0 costs it eight digits.
   */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <mpfr.h>

double fd_j0(double), fd_j1(double), fd_y0(double), fd_y1(double);
double fd_jn(int, double), fd_yn(int, double);

#define PREC 300

/* |J0(x)| and friends decay like sqrt(2/(pi x)); below 1 the functions are of
   order one, except y0 and y1, which grow to -inf and need no envelope. */
static double envelope (double x)
{
	x = fabs (x);
	return x < 1.0 ? 1.0 : sqrt (2.0 / (M_PI * x));
}

/* the same measure the test uses -- see ulp_scaled_steps_d in ulp-check.h */
static long steps(double got, double want, double x)
{
	double s = fabs (want) > envelope (x) ? fabs (want) : envelope (x);
	double u, d;
	int e;

	if (got == want) return 0;
	frexp (s, &e);
	u = ldexp (1.0, e - 53);
	d = fabs (got - want) / u;
	return d > 1e18 ? (long) 1e18 : (long) (d + 0.5);
}

static double ref1(int (*f)(mpfr_t, const mpfr_t, mpfr_rnd_t), double x)
{
	mpfr_t mx, mr;
	double r;
	mpfr_init2 (mx, PREC); mpfr_init2 (mr, PREC);
	mpfr_set_d (mx, x, MPFR_RNDN);
	f (mr, mx, MPFR_RNDN);
	r = mpfr_get_d (mr, MPFR_RNDN);
	mpfr_clear (mx); mpfr_clear (mr);
	return r;
}

static double refn(int (*f)(mpfr_t, long, const mpfr_t, mpfr_rnd_t), int n, double x)
{
	mpfr_t mx, mr;
	double r;
	mpfr_init2 (mx, PREC); mpfr_init2 (mr, PREC);
	mpfr_set_d (mx, x, MPFR_RNDN);
	f (mr, (long) n, mx, MPFR_RNDN);
	r = mpfr_get_d (mr, MPFR_RNDN);
	mpfr_clear (mx); mpfr_clear (mr);
	return r;
}

/* The zeros of j0 and j1.  jn and yn are built by recurrence from those two,
   so an error in either shows there and nowhere else -- a point set that pairs
   orders and arguments by arithmetic alone walks straight past it, as this one
   did before the third block below was added.  */
static const double j0_zero[] = {
	2.404825557695773, 5.520078110286311, 8.653727912911013,
	11.791534439014281, 14.930917708487787, 18.071063967910924,
	21.211636629879259, 24.352471530749302,
};
static const double j1_zero[] = {
	3.831705970207512, 7.015586669815619, 10.173468135062722,
	13.323691936314223, 16.470630050877634, 19.615858510468243,
	22.760084380592772, 25.903672087618382,
};
#define NZERO ((int) (sizeof j0_zero / sizeof *j0_zero))

/* half of the points in [lo, 60], half out to 2^60 */
static double point(int i, int n, double lo)
{
	int half = n / 2;
	if (i < half)
		return lo + (60.0 - lo) * i / (half - 1);
	int j = i - half;
	int m = n - half;
	double e = 6.0 + 54.0 * j / (m - 1);
	return ldexp (1.0 + (j % 13) / 13.0, (int) e);
}

#define SWEEP(nm, fdfn, mpfn, lo)					\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_d1 " #nm "_ref[] = {\n"); \
	for (int i = 0; i < N; i++) {					\
		double x = point(i, N, lo);				\
		double want = ref1(mpfn, x);				\
		if (!isfinite(want) || want == 0) continue;		\
		if (gen) printf("\t{ %a, %a },\n", x, want);		\
		long s = steps(fdfn(x), want, x);				\
		wrong += (s != 0);					\
		if (s > worst) worst = s;				\
	}								\
	if (gen) printf("};\n\n");					\
	else printf("  %-4s fdlibm %ld  (worst %ld)\n", #nm, wrong, worst); \
} while (0)

/* jn and yn: a third dense, a third on the zeros of j0 and j1 for every order,
   a third over the exponent range. */
static void pointn(int i, int n, double lo, int *order, double *x)
{
	int third = n / 3;

	if (i < third) {
		*order = i % 11;
		*x = lo + (60.0 - lo) * i / (third - 1);
		return;
	}
	if (i < 2 * third) {
		int j = i - third;
		const double *z = (j & 1) ? j1_zero : j0_zero;
		double base = z[(j / 2) % NZERO];
		/* on the zero and a few steps to either side of it */
		static const double off[] = { 0.0, 0x1p-20, -0x1p-20, 0x1p-10,
					      -0x1p-10, 0.01, -0.01 };
		*order = (j / 14) % 11;
		*x = base + off[j % 7];
		if (lo < 0 && (j & 2))
			*x = -*x;
		return;
	}
	{
		int j = i - 2 * third;
		int m = n - 2 * third;
		double e = 6.0 + 54.0 * j / (m - 1);
		*order = j % 11;
		*x = ldexp (1.0 + (j % 13) / 13.0, (int) e);
	}
}

#define SWEEPN(nm, fdfn, mpfn, lo)					\
do {									\
	long wrong = 0, worst = 0;					\
	if (gen) printf("static const struct ulp_n1 " #nm "_ref[] = {\n"); \
	for (int i = 0; i < NN; i++) {					\
		int order;						\
		double x;						\
		pointn(i, NN, lo, &order, &x);				\
		double want = refn(mpfn, order, x);			\
		if (!isfinite(want) || want == 0) continue;		\
		if (gen) printf("\t{ %d, %a, %a },\n", order, x, want);	\
		long s = steps(fdfn(order, x), want, x);			\
		wrong += (s != 0);					\
		if (s > worst) worst = s;				\
	}								\
	if (gen) printf("};\n\n");					\
	else printf("  %-4s fdlibm %ld  (worst %ld)\n", #nm, wrong, worst); \
} while (0)

int main(int argc, char **argv)
{
	const int N = 999, NN = 501;
	int gen = argc > 1;

	if (!gen) printf("of %d points (%d for jn and yn), off by at least one step:\n", N, NN);
	SWEEP(j0, fd_j0, mpfr_j0, -60.0);
	SWEEP(j1, fd_j1, mpfr_j1, -60.0);
	SWEEP(y0, fd_y0, mpfr_y0, 0x1p-40);
	SWEEP(y1, fd_y1, mpfr_y1, 0x1p-40);
	SWEEPN(jn, fd_jn, mpfr_jn, -60.0);
	SWEEPN(yn, fd_yn, mpfr_yn, 0x1p-40);
	return 0;
}
