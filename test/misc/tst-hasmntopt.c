/* Copyright (C) 2020 by Yann Sionneau <yann@sionneau.net> */

#include <stdio.h>
#include <mntent.h>
#include <stdlib.h>
#include <string.h>

static int
do_test (void)
{
	char *res;
	struct mntent m;

	/* check that "ro" does not match "erROr" */
	m.mnt_opts = "error";
	res = hasmntopt (&m, MNTOPT_RO);
	if (res != NULL) {
		puts ("error: hasmntopt() picked up non existing option");
		exit (1);
	}

	/* check that "ro" does not match "remount-ro" */
	m.mnt_opts = "rw,relatime,errors=remount-ro";
	res = hasmntopt (&m, MNTOPT_RO);
	if (res != NULL) {
		puts ("error: hasmntopt() picked up non existing option");
		exit (1);
	}

	/* check that "ro" does match "ro" */
	m.mnt_opts = "noatime,ro";
	res = hasmntopt (&m, MNTOPT_RO);
	if (res == NULL) {
		puts ("error: hasmntopt() did not pick up an existing option");
		exit (1);
	}

	if (strncmp(res, "ro", 2) != 0) {
		puts ("error: hasmntopt() did not return a pointer to corresponding option");
		exit (1);
	}

	return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
