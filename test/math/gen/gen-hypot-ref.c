/* Host-side generator for the hypot reference tables -- see README.md in this
   directory for what to link against and how the budgets in the test are
   derived.

   Two tables: one for double, and one for float, because uClibc-ng computes
   hypotf in double and converts (libm/w_hypotf.c).  */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
double fd_hypot(double,double); double gl_hypot(double,double); double cr_hypot(double,double);
float cr_hypotf(float,float);   /* the reference for the float table */
static long steps(double a,double b){ union{double d;int64_t i;}u,v; long d;
	if(a==b) return 0; u.d=a; v.d=b; d=(long)(u.i-v.i); return d<0?-d:d; }
static long stepsf(float a,float b){ union{float f;int32_t i;}u,v; long d;
	if(a==b) return 0; u.f=a; v.f=b; d=(long)(u.i-v.i); return d<0?-d:d; }
int main(int argc, char **argv)
{
	int emit = argc>1;
	long nfd=0,ngl=0,mfd=0,mgl=0,tab=0;
	long ffd=0,fgl=0,fcr=0,mffd=0,mfgl=0,mfcr=0,ftab=0;
	if(emit) printf("static const struct ulp_d2 hypot_ref[] = {\n");
	/* the same sweep the measurements used; every 4th pair that only fdlibm
	   misses and every 40th clean pair go into the table */
	long seen_bad=0, seen_ok=0;
	for (int i=0;i<700;i++){
		double x = ldexp(1.0 + (i%97)/97.0, (i%300)-150);
		for (int j=0;j<60;j++){
			double y = ldexp(1.0 + (j%13)/13.0, (j%120)-60);
			double want = cr_hypot(x,y);
			if(!isfinite(want)||want==0) continue;
			long sf=steps(fd_hypot(x,y),want), sg=steps(gl_hypot(x,y),want);
			/* Pairs where glibc misses are rare, so take all of them:
			   without those the test cannot tell OPTIMIZED from
			   ACCURATE at all. */
			int take;
			if (sg != 0) take = 1;
			else if (sf != 0) take = (seen_bad++ % 4)==0;
			else take = (seen_ok++ % 40)==0;
			if(!take) continue;
			if(emit) printf("\t{ %a, %a, %a },\n", x,y,want);
			tab++; nfd+=(sf!=0); ngl+=(sg!=0);
			if(sf>mfd)mfd=sf; if(sg>mgl)mgl=sg;
		}
	}
	if(emit) printf("};\n\nstatic const struct ulp_f2 hypotf_ref[] = {\n");
	/* Float needs no bias towards hard cases: computing in double leaves the
	   error far below the float rounding boundary, so a plain sweep is what
	   there is to check.  Every 7th pair keeps the table small.  */
	long fseen=0;
	for (int i=0;i<60;i++){
		float x = (float)ldexp(1.0 + (i%37)/37.0, (i%60)-30);
		for (int j=0;j<40;j++){
			float y = (float)ldexp(1.0 + (j%11)/11.0, (j%50)-25);
			float want = cr_hypotf(x,y);
			if(!isfinite(want)||want==0) continue;
			if((fseen++ % 7)!=0) continue;
			long sf=stepsf((float)fd_hypot(x,y),want);
			long sg=stepsf((float)gl_hypot(x,y),want);
			long sc=stepsf((float)cr_hypot(x,y),want);
			if(emit) printf("\t{ %a, %a, %a },\n", (double)x,(double)y,(double)want);
			ftab++; ffd+=(sf!=0); fgl+=(sg!=0); fcr+=(sc!=0);
			if(sf>mffd)mffd=sf; if(sg>mfgl)mfgl=sg; if(sc>mfcr)mfcr=sc;
		}
	}
	if(emit) printf("};\n");
	else {
		printf("double table: %ld pairs\n  SMALL (fdlibm)    %ld off (worst %ld)\n  OPTIMIZED (glibc) %ld off (worst %ld)\n  ACCURATE          0 (the reference)\n", tab,nfd,mfd,ngl,mgl);
		printf("float table: %ld pairs (double result rounded to float)\n  SMALL (fdlibm)    %ld off (worst %ld)\n  OPTIMIZED (glibc) %ld off (worst %ld)\n  ACCURATE          %ld off (worst %ld)\n", ftab,ffd,mffd,fgl,mfgl,fcr,mfcr);
	}
	return 0;
}
