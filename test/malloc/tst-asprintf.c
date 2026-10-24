#include <features.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>

/* malloc_stats() lives in libc/stdlib/malloc-standard only, so without that
   option this does not link.  It is a diagnostic here -- asprintf() does not
   need it -- so only the call goes, not the test. */
static void my_stats(void)
{
#if !defined __UCLIBC__ || defined __MALLOC_STANDARD__
	malloc_stats();
	fprintf(stderr, "\n");
#endif
}

static const char expect[] = "asdsadasd AAAAsdf\n";

static int
check(const char *what, char *s, int n)
{
	if (n != (int) strlen(expect)) {
		printf("FAIL: %s returned %d, expected %d\n",
		       what, n, (int) strlen(expect));
		return 1;
	}
	if (s == NULL) {
		printf("FAIL: %s left the pointer at NULL\n", what);
		return 1;
	}
	if (strcmp(s, expect) != 0) {
		printf("FAIL: %s produced \"%s\", expected \"%s\"\n",
		       what, s, expect);
		return 1;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char *a = NULL, *b = NULL;
	int errors = 0;
	int n;

	my_stats();

	/* Two allocations before either is freed, which is what this test is
	   about: the second must not hand back the first one's buffer. */
	n = asprintf(&b, "asdsadasd %ssdf\n", "AAAA");
	errors += check("first asprintf", b, n);
	my_stats();

	n = asprintf(&a, "asdsadasd %ssdf\n", "AAAA");
	errors += check("second asprintf", a, n);
	my_stats();

	if (a != NULL && a == b) {
		puts("FAIL: both calls returned the same buffer");
		errors++;
	}

	free(a);
	free(b);
	my_stats();

	return errors != 0;
}
