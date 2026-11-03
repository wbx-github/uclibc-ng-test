/* Shared harness for the tst-*-ulp tests.

   Each of those checks one function against correctly rounded results that were
   computed on the host (see gen/), because the guest has no reference of its
   own: uClibc-ng's long double is a wrapper around double on most ports, so
   expl is the very function under test.

   How far a result may stray from that reference depends on which
   implementation the library was configured with -- UCLIBC_LIBM_SMALL,
   _OPTIMIZED or _ACCURATE.  A test therefore states three budgets and lets
   BUDGET() pick the one in force.

   The float and long double entry points are not separate implementations:
   libm/w_*f.c and libm/ldouble_wrappers.c compute in double and convert.  So
   they are checked against the same setting, and against a table of their own
   only where the conversion can round a second time.  */

#ifndef ULP_CHECK_H
#define ULP_CHECK_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <features.h>

/* The real header where the port has fenv, and stand-ins where it does not.
   Not a local fenv.h: there used to be one in this directory, and because the
   include in libm-test.inc was quoted it shadowed the real header on every
   target, so no exception was ever checked anywhere.  */
#ifdef __UCLIBC_HAS_FENV__
# include <fenv.h>
#else
# define feclearexcept(x)	((void) 0)
# define fetestexcept(x)	(0)
#endif

#if defined __UCLIBC_LIBM_ACCURATE__
# define TIER				"accurate"
# define BUDGET(small, optimized, accurate)	(accurate)
#elif defined __UCLIBC_LIBM_OPTIMIZED__
# define TIER				"optimized"
# define BUDGET(small, optimized, accurate)	(optimized)
#else
# define TIER				"small"
# define BUDGET(small, optimized, accurate)	(small)
#endif

/* How far the worst point may be is stated per function, not once for all of
   them: exp, hypot and expm1 never miss by more than one representable value in
   any variant, but fdlibm's erfc reaches two, and three where the compiler
   contracts a multiply and an add.  A variant that lands further away than its
   own measurement did is broken in a different way than one that merely misses
   more often, which is why the two limits are separate.  */

#define NELEM(a)	((int)(sizeof (a) / sizeof (a)[0]))

/* Generated tables.  Results in them are positive, which is what makes the
   integer subtraction in ulp_steps_* a distance.  */
struct ulp_d1 { double x, want; };
struct ulp_d2 { double x, y, want; };
struct ulp_f1 { float x, want; };
struct ulp_f2 { float x, y, want; };
/* jn and yn take the order first, so they need a table of their own. */
struct ulp_n1 { int n; double x, want; };
/* fma takes three. */
struct ulp_d3 { double x, y, z, want; };

/* Distance in representable values, not in ulp: an ulp changes size at every
   power of two, this does not, and the budgets are counted in these.  */
static inline long ulp_steps_d(double got, double want)
{
	union { double d; int64_t i; } a, b;
	long d;

	if (got == want)
		return 0;
	a.d = got;
	b.d = want;
	d = (long)(a.i - b.i);
	return d < 0 ? -d : d;
}

/* Distance in representable values of the function's own scale, for functions
   that oscillate through zero.  Where the result is well above the scale this
   is the number ulp_steps_d gives; near a zero of the function it is the only
   number that means anything.  At the first zero of j0 the correctly rounded
   value is -6.1e-17, so an absolute error of one ulp of the order-one terms it
   was built from is four hundred million million steps of the result -- and
   every implementation that does not carry extra precision there does that.
   Against the scale the same error is three steps, which is what fdlibm
   delivers.  A real defect still shows: jn normalises with j0 (e_jn.c, b = t *
   __ieee754_j0(x) / b) and so inherits its relative error, which this measures
   as thousands of steps.

   Unlike ulp_steps_d this needs no agreement of sign, so it also holds where
   the two values straddle zero.  */
static inline long ulp_scaled_steps_d(double got, double want, double scale)
{
	double s = fabs(want) > scale ? fabs(want) : scale;
	double u, d;
	int e;

	if (got == want)
		return 0;
	frexp(s, &e);
	u = ldexp(1.0, e - 53);		/* the spacing at s */
	d = fabs(got - want) / u;
	return d > 1e18 ? (long)1e18 : (long)(d + 0.5);
}

static inline long ulp_steps_f(float got, float want)
{
	union { float f; int32_t i; } a, b;
	long d;

	if (got == want)
		return 0;
	a.f = got;
	b.f = want;
	d = (long)(a.i - b.i);
	return d < 0 ? -d : d;
}

/* The long double entry points return the double result widened, so compare
   there: the reference table stays the double one.  */
static inline long ulp_steps_l(long double got, double want)
{
	return ulp_steps_d((double)got, want);
}

static inline int ulp_verdict(const char *name, int wrong, int n, long worst,
			      int max_wrong, int max_steps, double wx,
			      double wy, int arity)
{
	if (arity == 2)
		printf("%s variant, %s: %d of %d off, worst %ld step(s) at (%a,%a)\n",
		       TIER, name, wrong, n, worst, wx, wy);
	else
		printf("%s variant, %s: %d of %d off, worst %ld step(s) at %a\n",
		       TIER, name, wrong, n, worst, wx);

	if (worst > max_steps) {
		printf("FAIL: %s is %ld step(s) from the correctly rounded value, %d allowed\n",
		       name, worst, max_steps);
		return 1;
	}
	if (wrong > max_wrong) {
		printf("FAIL: %s off at %d of %d points, %s allows %d\n",
		       name, wrong, n, TIER, max_wrong);
		return 1;
	}
	return 0;
}

/* One sweep per (type, arity).  Bodies are identical apart from the types, so
   they are written once here rather than once per test.  */
#define ULP_DEFINE_SWEEP1(sfx, type, table, steps)			\
static inline int ulp_sweep1_##sfx(const char *name, type (*fn)(type),	\
				   const struct table *t, int n,	\
				   int max_wrong, int max_steps)	\
{									\
	int i, wrong = 0;						\
	long worst = 0;							\
	double wx = 0;							\
									\
	for (i = 0; i < n; i++) {					\
		long s = steps(fn(t[i].x), t[i].want);			\
									\
		if (s != 0)						\
			wrong++;					\
		if (s > worst) {					\
			worst = s;					\
			wx = t[i].x;					\
		}							\
	}								\
	return ulp_verdict(name, wrong, n, worst, max_wrong, max_steps,	\
			   wx, 0, 1);					\
}

#define ULP_DEFINE_SWEEP2(sfx, type, table, steps)			\
static inline int ulp_sweep2_##sfx(const char *name,			\
				   type (*fn)(type, type),		\
				   const struct table *t, int n,	\
				   int max_wrong, int max_steps)	\
{									\
	int i, wrong = 0;						\
	long worst = 0;							\
	double wx = 0, wy = 0;						\
									\
	for (i = 0; i < n; i++) {					\
		long s = steps(fn(t[i].x, t[i].y), t[i].want);		\
									\
		if (s != 0)						\
			wrong++;					\
		if (s > worst) {					\
			worst = s;					\
			wx = t[i].x;					\
			wy = t[i].y;					\
		}							\
	}								\
	return ulp_verdict(name, wrong, n, worst, max_wrong, max_steps,	\
			   wx, wy, 2);					\
}

ULP_DEFINE_SWEEP1(d, double, ulp_d1, ulp_steps_d)
ULP_DEFINE_SWEEP1(f, float, ulp_f1, ulp_steps_f)
ULP_DEFINE_SWEEP1(l, long double, ulp_d1, ulp_steps_l)
ULP_DEFINE_SWEEP2(d, double, ulp_d2, ulp_steps_d)
ULP_DEFINE_SWEEP2(f, float, ulp_f2, ulp_steps_f)
/* Three arguments, for fma. */
static inline int ulp_sweep3_d(const char *name,
			       double (*fn)(double, double, double),
			       const struct ulp_d3 *t, int n,
			       int max_wrong, int max_steps)
{
	int i, wrong = 0;
	long worst = 0;
	double wx = 0, wy = 0;

	for (i = 0; i < n; i++) {
		long s = ulp_steps_d(fn(t[i].x, t[i].y, t[i].z), t[i].want);

		if (s != 0)
			wrong++;
		if (s > worst) {
			worst = s;
			wx = t[i].x;
			wy = t[i].y;
		}
	}
	return ulp_verdict(name, wrong, n, worst, max_wrong, max_steps,
			   wx, wy, 2);
}


/* Sweeps for the oscillating functions: scale(x) gives the size the function
   works at, so that a point near one of its zeros is measured against that and
   not against its own near-zero result.  */
static inline int ulp_sweep_scaled_d(const char *name, double (*fn)(double),
				     double (*scale)(double),
				     const struct ulp_d1 *t, int n,
				     int max_wrong, int max_steps)
{
	int i, wrong = 0;
	long worst = 0;
	double wx = 0;

	for (i = 0; i < n; i++) {
		long s = ulp_scaled_steps_d(fn(t[i].x), t[i].want,
					    scale(t[i].x));

		if (s != 0)
			wrong++;
		if (s > worst) {
			worst = s;
			wx = t[i].x;
		}
	}
	return ulp_verdict(name, wrong, n, worst, max_wrong, max_steps,
			   wx, 0, 1);
}

static inline int ulp_sweep_scaled_n(const char *name,
				     double (*fn)(int, double),
				     double (*scale)(double),
				     const struct ulp_n1 *t, int n,
				     int max_wrong, int max_steps)
{
	int i, wrong = 0;
	long worst = 0;
	double wx = 0, wn = 0;

	for (i = 0; i < n; i++) {
		long s = ulp_scaled_steps_d(fn(t[i].n, t[i].x), t[i].want,
					    scale(t[i].x));

		if (s != 0)
			wrong++;
		if (s > worst) {
			worst = s;
			wx = t[i].x;
			wn = t[i].n;
		}
	}
	return ulp_verdict(name, wrong, n, worst, max_wrong, max_steps,
			   wn, wx, 2);
}

ULP_DEFINE_SWEEP2(l, long double, ulp_d2, ulp_steps_l)

/* What a special case must raise, beside what it must return.  The two flags
   the C standard fixes for these functions and libm-test.inc knew about; the
   rest of FE_* is not something fdlibm sets on purpose.  */
#define ULP_NONE	0x0
#define ULP_INVALID	0x1
#define ULP_DIVBYZERO	0x2

/* Expectations that could not be checked because the port has no fenv.  Printed
   at the end of a test rather than passed over, so that a run says how much of
   the standard it left alone -- the summary in libm-test.inc claimed hundreds of
   exception tests for years while checking none.  */
static int ulp_exc_skipped;

static inline void ulp_clear_exc(void)
{
	feclearexcept (FE_ALL_EXCEPT);
}

/* Read the flags.  Called before anything else in ulp_expect_exc_*, so that it
   sees what the function under test left and not what printf did.  */
static inline int ulp_check_exc(const char *what, int want)
{
#ifdef __UCLIBC_HAS_FENV__
	int fails = 0;

# ifdef FE_INVALID
	if (!!fetestexcept (FE_INVALID) != !!(want & ULP_INVALID)) {
		printf("FAIL: %s %s invalid\n", what,
		       (want & ULP_INVALID) ? "did not raise" : "raised");
		fails++;
	}
# endif
# ifdef FE_DIVBYZERO
	if (!!fetestexcept (FE_DIVBYZERO) != !!(want & ULP_DIVBYZERO)) {
		printf("FAIL: %s %s divide-by-zero\n", what,
		       (want & ULP_DIVBYZERO) ? "did not raise" : "raised");
		fails++;
	}
# endif
	feclearexcept (FE_ALL_EXCEPT);
	return fails;
#else
	(void) what;
	if (want != ULP_NONE)
		ulp_exc_skipped++;
	return 0;
#endif
}

static inline void ulp_report_skipped(void)
{
	if (ulp_exc_skipped)
		printf("%d exception expectations skipped -- no fenv on this port\n",
		       ulp_exc_skipped);
}

/* Special cases are exact: no budget, and +0 is not -0.
 *
 * cmp is the type the comparison is made in, which for the long double entry
 * points is double: every *l function in this library computes in double and
 * widens the result (libm/ldouble_wrappers.c), so double is what it promises.
 * On x87 that matters, because the widening does not round -- the i386 ABI
 * returns a double in st(0) and does not require it narrowed to 53 bits, so the
 * extra register bits survive the cast and exp10l(3) comes out as 1000 plus a
 * remainder instead of 1000.  ulp_steps_l narrows for the same reason.  */
#define ULP_DEFINE_EXPECT(sfx, type, cmp)				\
static inline int ulp_expect_##sfx(const char *what, type got, type want) \
{									\
	int ok;								\
									\
	if (isnan(want))						\
		ok = isnan(got);					\
	else if (want == 0)						\
		ok = got == 0 && signbit(got) == signbit(want);		\
	else								\
		ok = (cmp) got == (cmp) want;				\
									\
	if (!ok)							\
		printf("FAIL: %s gave %a, expected %a\n", what,		\
		       (double)got, (double)want);			\
	return !ok;							\
}

ULP_DEFINE_EXPECT(d, double, double)
ULP_DEFINE_EXPECT(f, float, float)
ULP_DEFINE_EXPECT(l, long double, double)

/* Value and flags in one.  A statement expression, because the order matters:
   clear, then call, then read -- two arguments of a function call would not be
   sequenced, and printf inside the value check can raise flags of its own.
   expfn is the plain checker to use, so this works inside a macro that does not
   know which of the three it is dealing with.  */
#define ULP_EXPECT_EXC(expfn, what, call, want, exc) __extension__ ({	\
	__typeof (call) __v;						\
	int __f;							\
									\
	ulp_clear_exc ();						\
	__v = (call);							\
	__f = ulp_check_exc (what, exc);				\
	__f + expfn (what, __v, want); })

#endif /* ULP_CHECK_H */
