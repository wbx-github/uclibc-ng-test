#include <features.h>

#if defined __UCLIBC__ && !defined __HAVE_SHARED__

/* The test calls into libtest.so, which a libc without shared support cannot
   link at all. */
#include <stdio.h>
int
main (void)
{
	puts("needs HAVE_SHARED");
	return 23;			/* SKIP */
}

#else

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

extern int dltest(const char *s);

int main(int argc, char **argv)
{
	dltest("hello world!");
	return EXIT_SUCCESS;
}

#endif
