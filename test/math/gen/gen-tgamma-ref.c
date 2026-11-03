/* Host-side generator for the tgamma reference tables -- see README.md in this
   directory for what to link against.

   Unlike the other generators this one does not print the variants' miss counts,
   because they would be wrong here: the small variant computes exp(lgamma(x)),
   so its result depends on the exp and the log underneath it, and on a host
   those are glibc's rather than ours.  The budgets in tst-tgamma-ulp.c come from
   a run on the target, which is the only place the whole chain is ours.

   tgamma has poles at zero and at every negative integer and overflows above
   171.6.  A third of each point set lies in (0,10], a third on the negative side
   at non-integer arguments, and a third spreads over the rest of the range.  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double cr_tgamma(double);   /* the reference */
float cr_tgammaf(float);    /* the reference for the float table */
/* thirds: (0,10], negative non-integers, then towards the overflow threshold */
static double point(int i, int n, double top)
{
	int t = n / 3;
	if (i < t)
		return 0.01 + 9.99 * i / (t - 1);
	if (i < 2 * t) {
		int j = i - t;
		return -0.37 - 7.0 * j / (t - 1);
	}
	int j = i - 2 * t;
	int m = n - 2 * t;
	return 10.0 + (top - 10.0) * j / (m - 1);
}
int main(int argc, char **argv)
{
	const int N = 999, NF = 501;
	int gen = argc > 1;

	if (gen) printf("static const struct ulp_d1 tgamma_ref[] = {\n");
	for (int i = 0; i < N; i++) {
		double x = point (i, N, 171.0);
		double want = cr_tgamma (x);
		if (!isfinite (want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a },\n", x, want);
	}
	if (gen) printf("};\n\nstatic const struct ulp_f1 tgammaf_ref[] = {\n");
	for (int i = 0; i < NF; i++) {
		float x = (float) point (i, NF, 34.0);   /* tgammaf overflows above 35.04 */
		float want = cr_tgammaf (x);
		if (!isfinite (want) || want == 0) continue;
		if (gen) printf("\t{ %a, %a },\n", (double) x, (double) want);
	}
	if (gen) printf("};\n");
	else printf("run with an argument to print the tables; the budgets are measured on the target\n");
	return 0;
}
