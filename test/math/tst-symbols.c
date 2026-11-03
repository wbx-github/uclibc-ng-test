/* Every function bits/mathcalls.h declares has a symbol, and the aliases mean
 * what they say.
 *
 * This is the cheapest test in the directory and the one that was missing
 * longest.  hypotl had no public symbol from 2017 until August 2026 -- commit
 * ea38f4d89 moved it out of ldouble_wrappers.c and left the libm_hidden_def
 * behind, so only __GI_hypotl existed and every program that called hypotl
 * failed to link.  Nine years, because nothing ever took its address.  The same
 * class of fault is live today: w_j0f.c and w_j0l.c hide their entry points
 * behind #ifndef __DO_XSI_MATH__, so j0f and j0l do not exist in the one
 * configuration where the Bessel functions are built.
 *
 * A missing symbol is reported, not linked against.  Taking the address the
 * ordinary way would make it a link error, and a link error stops make and with
 * it every other test in this directory -- so the names are declared weak, and
 * a weak reference the linker cannot satisfy is a null pointer instead.  One
 * run then names every missing symbol at once, which a link error cannot do.
 *
 * The cost is that this works only where the test is linked against a shared
 * libm: in a static link a weak reference does not pull the archive member in,
 * so every name would come out null whether it exists or not.  A statically
 * linked build therefore says so and checks only the aliases, which are called
 * rather than pointed at.  _DYNAMIC tells the two apart -- it is absent from a
 * static binary -- and it is declared weak here for the same reason.
 *
 * Nothing in this file calls a weakly declared function.  A weak declaration
 * applies to the whole translation unit, so a call would bind weakly too, the
 * linker would drop libm as unneeded and the call would jump to zero.
 *
 * The guards mirror libm/Makefile.in, which is what decides whether a file is
 * compiled -- not bits/mathcalls.h, which declares by feature macro.  */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <math.h>
#include <features.h>

/* Weak, every one of them, so that a name the linker cannot satisfy is a null
   pointer rather than an error.  Declared here at file scope because that is
   where #pragma weak belongs; the guards further down decide which are looked
   at, and a pragma for a name nobody looks at costs nothing.

   The names aliases() compares are weak as well, and it reaches them through
   pointers it has checked first -- a weak declaration would otherwise make a
   direct call bind weakly and jump to zero when the symbol is absent, which is
   the case the test exists to report.  signgam is the strong reference that
   keeps libm from being dropped as unneeded; without one, every weak name would
   come out null.  */
#pragma weak acos
#pragma weak acosf
#pragma weak acosl
#pragma weak asin
#pragma weak asinf
#pragma weak asinl
#pragma weak atan
#pragma weak atanf
#pragma weak atanl
#pragma weak cos
#pragma weak cosf
#pragma weak cosl
#pragma weak sin
#pragma weak sinf
#pragma weak sinl
#pragma weak tan
#pragma weak tanf
#pragma weak tanl
#pragma weak cosh
#pragma weak coshf
#pragma weak coshl
#pragma weak sinh
#pragma weak sinhf
#pragma weak sinhl
#pragma weak tanh
#pragma weak tanhf
#pragma weak tanhl
#pragma weak exp
#pragma weak expf
#pragma weak expl
#pragma weak log
#pragma weak logf
#pragma weak logl
#pragma weak log10
#pragma weak log10f
#pragma weak log10l
#pragma weak sqrt
#pragma weak sqrtf
#pragma weak sqrtl
#pragma weak ceil
#pragma weak ceilf
#pragma weak ceill
#pragma weak fabs
#pragma weak fabsf
#pragma weak fabsl
#pragma weak floor
#pragma weak floorf
#pragma weak floorl
#pragma weak acosh
#pragma weak acoshf
#pragma weak acoshl
#pragma weak asinh
#pragma weak asinhf
#pragma weak asinhl
#pragma weak atanh
#pragma weak atanhf
#pragma weak atanhl
#pragma weak expm1
#pragma weak expm1f
#pragma weak expm1l
#pragma weak log1p
#pragma weak log1pf
#pragma weak log1pl
#pragma weak logb
#pragma weak logbf
#pragma weak logbl
#pragma weak cbrt
#pragma weak cbrtf
#pragma weak cbrtl
#pragma weak rint
#pragma weak rintf
#pragma weak rintl
#pragma weak erf
#pragma weak erff
#pragma weak erfl
#pragma weak erfc
#pragma weak erfcf
#pragma weak erfcl
#pragma weak significand
#pragma weak significandf
#pragma weak significandl
#pragma weak atan2
#pragma weak atan2f
#pragma weak atan2l
#pragma weak pow
#pragma weak powf
#pragma weak powl
#pragma weak fmod
#pragma weak fmodf
#pragma weak fmodl
#pragma weak hypot
#pragma weak hypotf
#pragma weak hypotl
#pragma weak copysign
#pragma weak copysignf
#pragma weak copysignl
#pragma weak nextafter
#pragma weak nextafterf
#pragma weak nextafterl
#pragma weak scalb
#pragma weak scalbf
#pragma weak scalbl
#pragma weak frexp
#pragma weak frexpf
#pragma weak frexpl
#pragma weak ldexp
#pragma weak ldexpf
#pragma weak ldexpl
#pragma weak modf
#pragma weak modff
#pragma weak modfl
#pragma weak scalbn
#pragma weak scalbnf
#pragma weak scalbnl
#pragma weak ilogb
#pragma weak ilogbf
#pragma weak ilogbl
#pragma weak nexttoward
#pragma weak nexttowardf
#pragma weak nexttowardl
#pragma weak exp2
#pragma weak exp2f
#pragma weak exp2l
#pragma weak log2
#pragma weak log2f
#pragma weak log2l
#pragma weak tgamma
#pragma weak tgammaf
#pragma weak tgammal
#pragma weak nearbyint
#pragma weak nearbyintf
#pragma weak nearbyintl
#pragma weak round
#pragma weak roundf
#pragma weak roundl
#pragma weak trunc
#pragma weak truncf
#pragma weak truncl
#pragma weak fdim
#pragma weak fdimf
#pragma weak fdiml
#pragma weak fmax
#pragma weak fmaxf
#pragma weak fmaxl
#pragma weak fmin
#pragma weak fminf
#pragma weak fminl
#pragma weak fma
#pragma weak fmaf
#pragma weak fmal
#pragma weak scalbln
#pragma weak scalblnf
#pragma weak scalblnl
#pragma weak remquo
#pragma weak remquof
#pragma weak remquol
#pragma weak lrint
#pragma weak lrintf
#pragma weak lrintl
#pragma weak llrint
#pragma weak llrintf
#pragma weak llrintl
#pragma weak lround
#pragma weak lroundf
#pragma weak lroundl
#pragma weak llround
#pragma weak llroundf
#pragma weak llroundl
#pragma weak nan
#pragma weak nanf
#pragma weak nanl
#pragma weak sincos
#pragma weak sincosf
#pragma weak sincosl
#pragma weak j0
#pragma weak j1
#pragma weak jn
#pragma weak y0
#pragma weak y1
#pragma weak yn
#pragma weak drem
#pragma weak remainder
#pragma weak exp10
#pragma weak pow10
#pragma weak gamma
#pragma weak lgamma
#pragma weak _DYNAMIC

extern void *_DYNAMIC;
extern int signgam;		/* strong, so libm stays linked */

static int missing;
static int checked;
static int no_weak;		/* set by surface(), which main runs first */
static const char *gone[64];	/* a real gap is one name, not a hundred */

#define CHECK(fn)							\
	do {								\
		checked++;						\
		if (!(void *) (fn)) {					\
			if (missing < (int) (sizeof gone / sizeof gone[0])) \
				gone[missing] = #fn;			\
			missing++;					\
		}							\
	} while (0)

#ifdef __UCLIBC_HAS_LONG_DOUBLE_MATH__
# define CHECK_FL(fn)	do { CHECK (fn##f); CHECK (fn##l); } while (0)
#else
# define CHECK_FL(fn)	CHECK (fn##f)
#endif

#define CHECK_ALL(fn)	do { CHECK (fn); CHECK_FL (fn); } while (0)

static void surface(void)
{
	int k;	/* declared here, not in the for: the suite sets no -std= and
		   avr32's gcc 4.4.7 defaults to gnu89, which rejects it */

	if (!&_DYNAMIC) {
		puts("SKIP: statically linked -- a weak reference does not pull an\n"
		     "      archive member, so the symbol check cannot run here");
		no_weak = 1;
		return;
	}

	CHECK_ALL (acos);
	CHECK_ALL (asin);
	CHECK_ALL (atan);
	CHECK_ALL (cos);
	CHECK_ALL (sin);
	CHECK_ALL (tan);
	CHECK_ALL (cosh);
	CHECK_ALL (sinh);
	CHECK_ALL (tanh);
	CHECK_ALL (exp);
	CHECK_ALL (log);
	CHECK_ALL (log10);
	CHECK_ALL (sqrt);
	CHECK_ALL (ceil);
	CHECK_ALL (fabs);
	CHECK_ALL (floor);
	CHECK_ALL (acosh);
	CHECK_ALL (asinh);
	CHECK_ALL (atanh);
	CHECK_ALL (expm1);
	CHECK_ALL (log1p);
	CHECK_ALL (logb);
	CHECK_ALL (cbrt);
	CHECK_ALL (rint);
	CHECK_ALL (erf);
	CHECK_ALL (erfc);
	CHECK_ALL (significand);
	CHECK_ALL (atan2);
	CHECK_ALL (pow);
	CHECK_ALL (fmod);
	CHECK_ALL (hypot);
	CHECK_ALL (copysign);
	CHECK_ALL (nextafter);
	CHECK_ALL (scalb);
	CHECK_ALL (frexp);
	CHECK_ALL (ldexp);
	CHECK_ALL (modf);
	CHECK_ALL (scalbn);
	CHECK_ALL (ilogb);
	CHECK_ALL (nexttoward);

#ifdef __DO_C99_MATH__
	CHECK_ALL (exp2);
	CHECK_ALL (log2);
	CHECK_ALL (tgamma);
	CHECK_ALL (nearbyint);
	CHECK_ALL (round);
	CHECK_ALL (trunc);
	CHECK_ALL (fdim);
	CHECK_ALL (fmax);
	CHECK_ALL (fmin);
	CHECK_ALL (fma);
	CHECK_ALL (scalbln);
	CHECK_ALL (remquo);
	CHECK_ALL (lrint);
	CHECK_ALL (llrint);
	CHECK_ALL (lround);
	CHECK_ALL (llround);
	CHECK_ALL (nan);
#else
	puts("SKIP: the C99 functions are not built without DO_C99_MATH");
#endif

	CHECK_ALL (sincos);
#ifdef __UCLIBC__
	/* glibc dropped pow10 in 2.27; uClibc-ng still has it, a strong_alias
	   in w_exp10.c.  */
#endif

#ifdef __DO_XSI_MATH__
	/* Double only, on purpose.  w_j0f.c and w_j0l.c compile their entry
	   points only when __DO_XSI_MATH__ is *not* defined, so j0f and j0l do
	   not exist here -- which is the fault, not the rule, and it is written
	   down in tst-bessel-ulp.c.  Checking them would report a fault already
	   known and say nothing new.  */
	CHECK (j0); CHECK (j1); CHECK (jn);
	CHECK (y0); CHECK (y1); CHECK (yn);
#else
	puts("SKIP: the Bessel functions are not built without DO_XSI_MATH");
#endif
	/* Every name null means the weak references never reached a definition,
	   not that the library is empty: in a static link the member is simply
	   never pulled in.  _DYNAMIC catches the plain static case above, but a
	   static PIE carries a .dynamic section of its own for self-relocation
	   and slips past it -- riscv32 noMMU ELF is such a target, and reported
	   all 744 names missing while the other tests in the same run called
	   those very functions and got right answers.  So treat all-null as the
	   same skip, and say the count out loud.  */
	if (checked && missing == checked) {
		printf("SKIP: all %d names read null -- linked against an archive,\n"
		       "      where a weak reference does not pull the member in\n",
		       checked);
		missing = 0;
		no_weak = 1;
		return;
	}

	for (k = 0; k < missing && k < (int) (sizeof gone / sizeof gone[0]); k++)
		printf("MISSING: %s\n", gone[k]);
	if (missing > (int) (sizeof gone / sizeof gone[0]))
		printf("MISSING: and %d more\n",
		       missing - (int) (sizeof gone / sizeof gone[0]));

}

/* The aliases.  Each is a second name for a function checked elsewhere, and the
   only thing to ask is whether it is the same function.  These are called
   rather than pointed at, so they are a link check of their own and work in a
   static build too.  */
static int aliases(void)
{
	static const double t[] = { 0.5, 1.0, 2.5, 7.0, 100.0 };
	double (*p_drem)(double, double) = drem;
	double (*p_rem)(double, double) = remainder;
	double (*p_exp10)(double) = exp10;
#if defined __UCLIBC__ && defined __DO_C99_MATH__
	double (*p_pow10)(double) = pow10;	/* glibc dropped it in 2.27 */
#else
	double (*p_pow10)(double) = 0;
#endif
	double (*p_gamma)(double) = gamma;
	double (*p_lgamma)(double) = lgamma;
	int fails = 0;
	unsigned i;

	if (no_weak)
		return 0;	/* see surface() */

	if (!p_drem) { puts("MISSING: drem"); fails++; }
	if (!p_rem) { puts("MISSING: remainder"); fails++; }
#ifdef __DO_C99_MATH__
	if (!p_exp10) { puts("MISSING: exp10"); fails++; }
#endif
#if defined __UCLIBC__ && defined __DO_C99_MATH__
	if (!p_pow10) { puts("MISSING: pow10"); fails++; }
#endif
#ifdef __DO_XSI_MATH__
	if (!p_gamma) { puts("MISSING: gamma"); fails++; }
#endif
	if (!p_lgamma) { puts("MISSING: lgamma"); fails++; }

	for (i = 0; i < sizeof t / sizeof *t; i++) {
		volatile double a, b;

		if (p_drem && p_rem) {
			a = p_drem(t[i], 3.0); b = p_rem(t[i], 3.0);
			if (a != b) {
				printf("FAIL: drem(%g,3) is %a, remainder gives %a\n",
				       t[i], a, b);
				fails++;
			}
		}
		if (p_pow10 && p_exp10) {
			a = p_pow10(t[i]); b = p_exp10(t[i]);
			if (a != b) {
				printf("FAIL: pow10(%g) is %a, exp10 gives %a\n",
				       t[i], a, b);
				fails++;
			}
		}
		if (p_gamma && p_lgamma) {
			/* Through memory, both: on i386 a double comes back in
			   st(0) and need not be narrowed, so comparing two
			   calls directly compares a rounded copy against an
			   80-bit one.  */
			a = p_gamma(t[i]); b = p_lgamma(t[i]);
			if (a != b) {
				printf("FAIL: gamma(%g) is %a, lgamma gives %a\n",
				       t[i], a, b);
				fails++;
			}
		}
	}
	return fails;
}

/* The legacy predicates.  These check behaviour, not linkage: math.h gives them
   as macros over compiler builtins, so nothing in libm is called.  */
static int predicates(void)
{
	int fails = 0;
	static const struct { const char *n; double x; int inf, nan, fin; } t[] = {
		{ "1",		1.0,		0, 0, 1 },
		{ "inf",	INFINITY,	1, 0, 0 },
		{ "-inf",	-INFINITY,	1, 0, 0 },
		{ "nan",	NAN,		0, 1, 0 },
		{ "0",		0.0,		0, 0, 1 },
	};
	unsigned i;

	for (i = 0; i < sizeof t / sizeof *t; i++) {
		if (!!isinf(t[i].x) != t[i].inf) {
			printf("FAIL: isinf(%s) is %d, expected %d\n",
			       t[i].n, !!isinf(t[i].x), t[i].inf);
			fails++;
		}
		if (!!isnan(t[i].x) != t[i].nan) {
			printf("FAIL: isnan(%s) is %d, expected %d\n",
			       t[i].n, !!isnan(t[i].x), t[i].nan);
			fails++;
		}
		if (!!finite(t[i].x) != t[i].fin) {
			printf("FAIL: finite(%s) is %d, expected %d\n",
			       t[i].n, !!finite(t[i].x), t[i].fin);
			fails++;
		}
	}
	return fails;
}

int main(void)
{
	static volatile int anchor;
	int fails = 0;

	anchor = signgam;	/* the strong reference; see the head of the file */
	(void) anchor;
	surface();
	fails += missing;
	fails += aliases();
	fails += predicates();
	if (!fails)
		puts("every declared function has a symbol and the aliases agree");
	return fails != 0;
}
