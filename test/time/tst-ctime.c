/* vi: set sw=4 ts=4: */
/* testcase for ctime(3) with large time
 * Copyright (C) 2010 David A Ramos <daramos@gustav.stanford.edu>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_POSITIVE(type)  (~0 & ~((type) 1 << (sizeof(type)*8 - 1)))

int do_test(int argc, char **argv) {
	char *correct = 0, *s;
	struct tm *tm;
	int status;

	/* need a very high positive number (e.g., max - 1024) */
	time_t test = MAX_POSITIVE(time_t) - 1024;

	/* With a 64-bit time_t this year does not fit an int tm_year, so
	 * localtime() reports the failure and there is no string to compare.
	 * Handing its NULL to asctime() would fault -- ctime() has the same
	 * two cases inside, and that equivalence is what is under test. */
	tm = localtime(&test);
	if (tm) {
		s = asctime(tm);
		if (s) {
			// copy static buffer to heap
			correct = malloc(strlen(s)+1);
			strcpy(correct, s);
		}
	}

	s = ctime(&test);

	printf("ANSI:\t%suClibc:\t%s",
	       correct ? correct : "(none)\n", s ? s : "(none)\n");

	if (correct == NULL || s == NULL)
		status = (correct == NULL && s == NULL) ? EXIT_SUCCESS : EXIT_FAILURE;
	else if (s != correct && strcmp(correct, s))
		status = EXIT_FAILURE;
	else
		status = EXIT_SUCCESS;

	if (correct)
		free(correct);

    return status;
}

#include <test-skeleton.c>
