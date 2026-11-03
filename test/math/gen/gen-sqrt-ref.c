/* Host-side generator for the sqrt reference table -- see README.md in this
   directory for what to link against.

   sqrt is one of the five operations IEEE 754 requires correctly rounded, so
   there is nothing to choose between implementations and the budget is zero in
   every setting.  What this test is for is the ports that compute it in
   software: where the hardware has a sqrt instruction the compiler emits it and
   the answer is right by construction, but libm/e_sqrt.c is a shift-and-subtract
   loop, and whether it rounds correctly at every point is not something the ten
   named cases in sqrt_test could show.

   Half the points cover [0.25, 4], where the mantissa work happens, and a third
   of those sit just above and below a perfect square, which is where a
   shift-and-subtract loop is most likely to pick the wrong last bit.  The rest
   spread over the exponent range, down into the subnormals and up to the
   largest finite double.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <mpfr.h>

double fd_sqrt(double);

static long steps(double a, double b)
{
	union { double d; int64_t i; } ua = {a}, ub = {b};
	if (a == b) return 0;
	long d = ua.i - ub.i;
	return d < 0 ? -d : d;
}

static double ref(double x)
{
	mpfr_t a, b;
	double r;
	mpfr_init2 (a, 300); mpfr_init2 (b, 300);
	mpfr_set_d (a, x, MPFR_RNDN);
	mpfr_sqrt (b, a, MPFR_RNDN);
	r = mpfr_get_d (b, MPFR_RNDN);
	mpfr_clear (a); mpfr_clear (b);
	return r;
}

static double point(int i, int n)
{
	int half = n / 2;

	if (i < half) {
		if (i % 3) {			/* dense in [0.25, 4] */
			double t = (double)(i / 3) / (half / 3 - 1);
			return 0.25 + 3.75 * t;
		}
		/* just off a perfect square */
		double k = 1.0 + (i / 3) % 64;
		double sq = k * k;
		return nextafter (sq, (i & 4) ? 0.0 : 2.0 * sq);
	}
	{
		int j = i - half;
		int m = n - half;
		double e = -1074.0 + (1023.0 + 1074.0) * j / (m - 1);
		return ldexp (1.0 + (j % 17) / 17.0, (int) e);
	}
}

int main(int argc, char **argv)
{
	const int N = 1000;
	int gen = argc > 1;
	long wrong = 0, worst = 0;

	if (gen) printf("static const struct ulp_d1 sqrt_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point(i, N);
		double want;
		if (!(x > 0) || !isfinite(x)) continue;
		want = ref(x);
		if (!isfinite(want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
		long s = steps(fd_sqrt(x), want);
		wrong += (s != 0);
		if (s > worst) worst = s;
	}
	if (gen) printf("};\n");
	else printf("of %d points: libm/e_sqrt.c off at %ld (worst %ld)\n",
		    N, wrong, worst);
	return 0;
}
