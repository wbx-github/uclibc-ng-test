#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>

#define check_d1(func, param, expected) \
do { \
	int err; hex_union ur; hex_union up; \
	double result = func(param); up.f = param; ur.f = result; \
	errors += (err = (result != (expected))); \
	err \
	? printf("FAIL: %s(%g/"HEXFMT")=%g/"HEXFMT" (expected %g)\n", \
		#func, (double)(param), (long long)up.hex, result, (long long)ur.hex, (double)(expected)) \
	: printf("PASS: %s(%g)=%g\n", #func, (double)(param), result); \
} while (0)

#define check_i1(func, param, expected) \
do { \
	int err; hex_union up; \
	long long result = func(param); up.f = param; \
	errors += (err = (result != (expected))); \
	err \
	? printf("FAIL: %s(%g/"HEXFMT")=%lld/%llu (expected %llu)\n", \
		#func, (double)(param), (long long)up.hex, result, result, (long long)(expected)) \
	: printf("PASS: %s(%g)=%lld/%llu\n", #func, (double)(param), result, result); \
} while (0)

/* For the two-argument comparison macros.  No hex dump: what matters about
   these is the relation they report, and both operands are printed anyway.

   The operands go through volatile variables rather than into the macro
   directly, for two reasons.  Handed two literals, the compiler answers the
   comparison itself and the test checks the folding rather than the target's
   code.  And kvx's gcc 9.4.1 cannot expand isunordered() against a floating
   point constant at all -- "unrecognizable insn" for an unge:DI holding a
   const_double, an internal compiler error in extract_insn.  A constant in
   either position is enough; two variables are fine, and the other five
   comparison macros take a constant without complaint.  */
#define check_i2(func, p1, p2, expected) \
do { \
	int err; \
	volatile double v1 = (p1), v2 = (p2); \
	long long result = func(v1, v2); \
	errors += (err = (result != (expected))); \
	err \
	? printf("FAIL: %s(%g, %g)=%lld (expected %lld)\n", \
		#func, (double)(v1), (double)(v2), result, (long long)(expected)) \
	: printf("PASS: %s(%g, %g)=%lld\n", #func, (double)(v1), (double)(v2), result); \
} while (0)

#define HEXFMT "%08llx"
typedef union {
	double f;
	uint64_t hex;
} hex_union;

double zero = 0.0;
double minus_zero = 0.0;
double nan_value = 0.0;
int errors = 0;

int main(void)
{
	nan_value /= nan_value;
	minus_zero = copysign(zero, -1.0);

	check_i1(isfinite, 1.0, 1);
	check_i1(isfinite, 2.0, 1);
	check_i1(isfinite, 3.0, 1);
	check_i1(isfinite, DBL_MAX, 1);
	check_i1(isfinite, FLT_MAX, 1);
	check_i1(isfinite, HUGE_VAL, 0);
	check_i1(isfinite, HUGE_VALF, 0);
	check_i1(isfinite, HUGE_VALL, 0);
	check_i1(isfinite, nan_value, 0);
	check_i1(isfinite, nan_value, 0);
	check_i1(isfinite, nan_value, 0);

	check_i1(isnan, 1.0, 0);
	check_i1(isnan, 2.0, 0);
	check_i1(isnan, 3.0, 0);
	check_i1(isnan, DBL_MAX, 0);
	check_i1(isnan, FLT_MAX, 0);
	check_i1(isnan, HUGE_VAL, 0);
	check_i1(isnan, HUGE_VALF, 0);
	check_i1(isnan, HUGE_VALL, 0);
	check_i1(isnan, (float)HUGE_VALL, 0);
	check_i1(isnan, nan_value, 1);
	check_i1(isnan, nan_value, 1);
	check_i1(isnan, nan_value, 1);

	check_i1(isinf, 1.0, 0);
	check_i1(isinf, 2.0, 0);
	check_i1(isinf, 3.0, 0);
	check_i1(isinf, DBL_MAX, 0);
	check_i1(isinf, FLT_MAX, 0);
	check_i1(isinf, (float)DBL_MAX, 1);
	check_i1(isinf, HUGE_VAL, 1);
	check_i1(isinf, HUGE_VALF, 1);
	check_i1(isinf, HUGE_VALL, 1);
	check_i1(isinf, (float)HUGE_VALL, 1);
	check_i1(isinf, nan_value, 0);
	check_i1(isinf, nan_value, 0);
	check_i1(isinf, nan_value, 0);

	check_i1(fpclassify, minus_zero, FP_ZERO);
	check_i1(fpclassify, 0.0, FP_ZERO);
	check_i1(fpclassify, 1.0, FP_NORMAL);
	check_i1(fpclassify, 2.0, FP_NORMAL);
	check_i1(fpclassify, 3.0, FP_NORMAL);
	check_i1(fpclassify, DBL_MIN/1.01, FP_SUBNORMAL);
	check_i1(fpclassify, DBL_MIN, FP_NORMAL);
	check_i1(fpclassify, DBL_MAX, FP_NORMAL);
	check_i1(fpclassify, FLT_MAX, FP_NORMAL);
	check_i1(fpclassify, DBL_MAX, FP_NORMAL);
	check_i1(fpclassify, DBL_MAX*1.01, FP_INFINITE);
	check_i1(fpclassify, HUGE_VAL, FP_INFINITE);
	check_i1(fpclassify, HUGE_VALF, FP_INFINITE);
	check_i1(fpclassify, HUGE_VALL, FP_INFINITE);
	check_i1(fpclassify, (float)HUGE_VALL, FP_INFINITE);
	check_i1(fpclassify, nan_value, FP_NAN);
	check_i1(fpclassify, nan_value, FP_NAN);
	check_i1(fpclassify, nan_value, FP_NAN);

	check_i1(!!signbit, -1.0, 1);
	check_i1(!!signbit, minus_zero, 1);
	check_i1(!!signbit, 0.0, 0);
	check_i1(!!signbit, HUGE_VAL, 0);
	check_i1(!!signbit, HUGE_VALF, 0);
	check_i1(!!signbit, HUGE_VALL, 0);
	check_i1(!!signbit, -HUGE_VAL, 1);
	check_i1(!!signbit, -HUGE_VALF, 1);
	check_i1(!!signbit, -HUGE_VALL, 1);

	/* isnormal is fpclassify(x) == FP_NORMAL, so the cases that carry
	   information are the ones a naive "not zero, not NaN, not infinite"
	   would get wrong: a subnormal is none of those and is not normal
	   either.  C99 promises only a nonzero value, hence the !!.  */
	check_i1(!!isnormal, 1.0, 1);
	check_i1(!!isnormal, DBL_MIN, 1);
	check_i1(!!isnormal, DBL_MAX, 1);
	check_i1(!!isnormal, FLT_MIN, 1);
	check_i1(!!isnormal, DBL_MIN/1.01, 0);
	check_i1(!!isnormal, 0.0, 0);
	check_i1(!!isnormal, minus_zero, 0);
	check_i1(!!isnormal, HUGE_VAL, 0);
	check_i1(!!isnormal, HUGE_VALF, 0);
	check_i1(!!isnormal, HUGE_VALL, 0);
	check_i1(!!isnormal, nan_value, 0);

	/* The comparison macros.  What separates each from the operator it
	   looks like is the unordered pair: isgreater(1, NaN) and isless(1,
	   NaN) are both false, so !isless(x, y) does not mean x >= y.  Only
	   isunordered says yes to a NaN.  Every macro is checked against both
	   NaN positions and against NaN on both sides at once, because an
	   implementation that tests only one operand passes the other case.  */
	check_i2(isgreater, 2.0, 1.0, 1);
	check_i2(isgreater, 1.0, 2.0, 0);
	check_i2(isgreater, 1.0, 1.0, 0);
	check_i2(isgreater, nan_value, 1.0, 0);
	check_i2(isgreater, 1.0, nan_value, 0);
	check_i2(isgreater, nan_value, nan_value, 0);

	check_i2(isgreaterequal, 2.0, 1.0, 1);
	check_i2(isgreaterequal, 1.0, 1.0, 1);
	check_i2(isgreaterequal, 1.0, 2.0, 0);
	check_i2(isgreaterequal, nan_value, 1.0, 0);
	check_i2(isgreaterequal, 1.0, nan_value, 0);
	check_i2(isgreaterequal, nan_value, nan_value, 0);

	check_i2(isless, 1.0, 2.0, 1);
	check_i2(isless, 2.0, 1.0, 0);
	check_i2(isless, 1.0, 1.0, 0);
	check_i2(isless, nan_value, 1.0, 0);
	check_i2(isless, 1.0, nan_value, 0);
	check_i2(isless, nan_value, nan_value, 0);

	check_i2(islessequal, 1.0, 2.0, 1);
	check_i2(islessequal, 1.0, 1.0, 1);
	check_i2(islessequal, 2.0, 1.0, 0);
	check_i2(islessequal, nan_value, 1.0, 0);
	check_i2(islessequal, 1.0, nan_value, 0);
	check_i2(islessequal, nan_value, nan_value, 0);

	check_i2(islessgreater, 1.0, 2.0, 1);
	check_i2(islessgreater, 2.0, 1.0, 1);
	check_i2(islessgreater, 1.0, 1.0, 0);
	check_i2(islessgreater, nan_value, 1.0, 0);
	check_i2(islessgreater, 1.0, nan_value, 0);
	check_i2(islessgreater, nan_value, nan_value, 0);

	check_i2(isunordered, 1.0, 1.0, 0);
	check_i2(isunordered, 1.0, 2.0, 0);
	check_i2(isunordered, HUGE_VAL, 1.0, 0);
	check_i2(isunordered, nan_value, 1.0, 1);
	check_i2(isunordered, 1.0, nan_value, 1);
	check_i2(isunordered, nan_value, nan_value, 1);

	/* Zero of either sign compares equal, so the pair is ordered and not
	   greater or less either way.  An implementation that compared bit
	   patterns would report a difference here.  */
	check_i2(isunordered, 0.0, minus_zero, 0);
	check_i2(isgreaterequal, 0.0, minus_zero, 1);
	check_i2(islessequal, 0.0, minus_zero, 1);
	check_i2(isgreater, 0.0, minus_zero, 0);
	check_i2(isless, 0.0, minus_zero, 0);
	check_i2(islessgreater, 0.0, minus_zero, 0);

	/* No float or long double operands here, on purpose.  These six macros
	   expand to a compiler builtin and reach no symbol in libm, so their
	   width dispatch is the compiler's business and not this library's --
	   and check_i2 takes its operands through volatile double anyway, which
	   would narrow them before the macro ever saw them.  The width that
	   does matter is covered above by the classification macros, which do
	   call into libm: __fpclassifyl, __isinfl, __isnanl and __signbitl all
	   turn up in this test's undefined symbols.  */

	printf("Errors: %d\n", errors);
	return errors;
}
