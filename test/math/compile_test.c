#include <math.h>
#include <stdio.h>

/* testl below calls some sixty *l entry points, which libm has only when it
   was built with UCLIBC_HAS_LONG_DOUBLE_MATH -- otherwise this does not link.
   math.h reports that as __NO_LONG_DOUBLE_MATH, so ask the header: the test
   suite never reads uClibc-ng's .config and the Makefile cannot tell.
   testf stays either way; its only long double is the second argument of
   nexttowardf, which exists regardless.  */
#ifdef __NO_LONG_DOUBLE_MATH
# define NO_LONG_DOUBLE 1
#endif

/* External, not static, and that is the point: see main.  */
int testf(float float_x, long double long_double_x, /*float complex float_complex_x,*/ int int_x, long long_x)
{
int r = 0;
r += acosf(float_x);
r += acoshf(float_x);
r += asinf(float_x);
r += asinhf(float_x);
r += atan2f(float_x, float_x);
r += atanf(float_x);
r += atanhf(float_x);
/*r += cargf(float_complex_x); - will fight with complex numbers later */
r += cbrtf(float_x);
r += ceilf(float_x);
r += copysignf(float_x, float_x);
r += cosf(float_x);
r += coshf(float_x);
r += erfcf(float_x);
r += erff(float_x);
r += exp2f(float_x);
r += expf(float_x);
r += expm1f(float_x);
r += fabsf(float_x);
r += fdimf(float_x, float_x);
r += floorf(float_x);
r += fmaf(float_x, float_x, float_x);
r += fmaxf(float_x, float_x);
r += fminf(float_x, float_x);
r += fmodf(float_x, float_x);
r += frexpf(float_x, &int_x);
r += gammaf(float_x);
r += hypotf(float_x, float_x);
r += ilogbf(float_x);
r += ldexpf(float_x, int_x);
r += lgammaf(float_x);
r += llrintf(float_x);
r += llroundf(float_x);
r += log10f(float_x);
r += log1pf(float_x);
r += log2f(float_x);
r += logbf(float_x);
r += logf(float_x);
r += lrintf(float_x);
r += lroundf(float_x);
r += modff(float_x, &float_x);
r += nearbyintf(float_x);
r += nexttowardf(float_x, long_double_x);
r += powf(float_x, float_x);
r += remainderf(float_x, float_x);
r += remquof(float_x, float_x, &int_x);
r += rintf(float_x);
r += roundf(float_x);
#ifdef __UCLIBC_SUSV3_LEGACY__
r += scalbf(float_x, float_x);
#endif
r += scalblnf(float_x, long_x);
r += scalbnf(float_x, int_x);
r += significandf(float_x);
r += sinf(float_x);
r += sinhf(float_x);
r += sqrtf(float_x);
r += tanf(float_x);
r += tanhf(float_x);
r += tgammaf(float_x);
r += truncf(float_x);
return r;
}

/* The double entry points.  This test had a float and a long double section
   and none for double -- the width every program reaches for first was the
   one it did not link.  The list is the union of the other two: the names
   only testf had (gamma, scalb, significand), the ones only testl had
   (nextafter and the five __ classification helpers the type-generic macros
   dispatch to), and everything both share.  */
int testd(double double_x, long double long_double_x, int int_x, long long_x)
{
int r = 0;
r += __finite(double_x);
r += __fpclassify(double_x);
r += __isinf(double_x);
r += __isnan(double_x);
r += __signbit(double_x);
r += acos(double_x);
r += acosh(double_x);
r += asin(double_x);
r += asinh(double_x);
r += atan2(double_x, double_x);
r += atan(double_x);
r += atanh(double_x);
r += cbrt(double_x);
r += ceil(double_x);
r += copysign(double_x, double_x);
r += cos(double_x);
r += cosh(double_x);
r += erfc(double_x);
r += erf(double_x);
r += exp2(double_x);
r += exp(double_x);
r += expm1(double_x);
r += fabs(double_x);
r += fdim(double_x, double_x);
r += floor(double_x);
r += fma(double_x, double_x, double_x);
r += fmax(double_x, double_x);
r += fmin(double_x, double_x);
r += fmod(double_x, double_x);
r += frexp(double_x, &int_x);
r += gamma(double_x);
r += hypot(double_x, double_x);
r += ilogb(double_x);
r += ldexp(double_x, int_x);
r += lgamma(double_x);
r += llrint(double_x);
r += llround(double_x);
r += log10(double_x);
r += log1p(double_x);
r += log2(double_x);
r += logb(double_x);
r += log(double_x);
r += lrint(double_x);
r += lround(double_x);
r += modf(double_x, &double_x);
r += nearbyint(double_x);
r += nextafter(double_x, double_x);
r += nexttoward(double_x, long_double_x);
r += pow(double_x, double_x);
r += remainder(double_x, double_x);
r += remquo(double_x, double_x, &int_x);
r += rint(double_x);
r += round(double_x);
r += scalb(double_x, double_x);
r += scalbln(double_x, long_x);
r += scalbn(double_x, int_x);
r += significand(double_x);
r += sin(double_x);
r += sinh(double_x);
r += sqrt(double_x);
r += tan(double_x);
r += tanh(double_x);
r += tgamma(double_x);
r += trunc(double_x);
return r;
}

#ifndef NO_LONG_DOUBLE
int testl(long double long_double_x, int int_x, long long_x)
{
int r = 0;
r += __finitel(long_double_x);
r += __fpclassifyl(long_double_x);
r += __isinfl(long_double_x);
r += __isnanl(long_double_x);
r += __signbitl(long_double_x);
r += acoshl(long_double_x);
r += acosl(long_double_x);
r += asinhl(long_double_x);
r += asinl(long_double_x);
r += atan2l(long_double_x, long_double_x);
r += atanhl(long_double_x);
r += atanl(long_double_x);
r += cbrtl(long_double_x);
r += ceill(long_double_x);
r += copysignl(long_double_x, long_double_x);
r += coshl(long_double_x);
r += cosl(long_double_x);
r += erfcl(long_double_x);
r += erfl(long_double_x);
r += exp2l(long_double_x);
r += expl(long_double_x);
r += expm1l(long_double_x);
r += fabsl(long_double_x);
r += fdiml(long_double_x, long_double_x);
r += floorl(long_double_x);
r += fmal(long_double_x, long_double_x, long_double_x);
r += fmaxl(long_double_x, long_double_x);
r += fminl(long_double_x, long_double_x);
r += fmodl(long_double_x, long_double_x);
r += frexpl(long_double_x, &int_x);
r += hypotl(long_double_x, long_double_x);
r += ilogbl(long_double_x);
r += ldexpl(long_double_x, int_x);
r += lgammal(long_double_x);
r += llrintl(long_double_x);
r += llroundl(long_double_x);
r += log10l(long_double_x);
r += log1pl(long_double_x);
r += log2l(long_double_x);
r += logbl(long_double_x);
r += logl(long_double_x);
r += lrintl(long_double_x);
r += lroundl(long_double_x);
r += modfl(long_double_x, &long_double_x);
r += nearbyintl(long_double_x);
r += nextafterl(long_double_x, long_double_x);
r += nexttowardl(long_double_x, long_double_x);
r += powl(long_double_x, long_double_x);
r += remainderl(long_double_x, long_double_x);
r += remquol(long_double_x, long_double_x, &int_x);
r += rintl(long_double_x);
r += roundl(long_double_x);
r += scalblnl(long_double_x, long_x);
r += scalbnl(long_double_x, int_x);
r += sinhl(long_double_x);
r += sinl(long_double_x);
r += sqrtl(long_double_x);
r += tanhl(long_double_x);
r += tanl(long_double_x);
r += tgammal(long_double_x);
r += truncl(long_double_x);
return r;
}
#endif

/* Nothing here calls the three functions above, and nothing needs to: this is
   a link test, and giving them external linkage is what makes it one.  The
   compiler may not discard an externally visible function, so every call in
   them has to resolve.

   It used to be "return 5 & ((long)&testf) & ((long)&testl) & 2;", with the
   comment "Always 0 but gcc hopefully won't be able to notice".  gcc does
   notice: 5 & 2 is 0 whatever the addresses are, so it folded the expression,
   the address-taking went with it, and the three static functions became
   unreferenced and were dropped.  Measured on i686 with gcc 13 at -O2, the
   binary held main alone and called nothing -- 1 undefined symbol against 184
   at -O0.  Test.mak builds every test twice, plain and at -O2, so half of
   this test was checking nothing at all.  */
int main(int argc, char **argv)
{
#ifdef NO_LONG_DOUBLE
        puts("SKIP: built without UCLIBC_HAS_LONG_DOUBLE_MATH, libm has no *l");
        return 23;	/* 23 is the runner's skip status */
#else
        return 0;
#endif
}
