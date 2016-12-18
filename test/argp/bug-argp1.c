#include <stdio.h>

#if (defined(__GLIBC__) && !defined(__UCLIBC__)) || defined(__UCLIBC_HAS_ARGP__)
#include <argp.h>

static const struct argp_option test_options[] =
{
  { NULL, 'a', NULL, OPTION_DOC, NULL },
  { NULL, 'b', NULL, OPTION_DOC, NULL },
  { NULL, 0, NULL, 0, NULL }
};

static struct argp test_argp =
{
  test_options
};

#endif

static int
do_test (int argc, char *argv[])
{
#if (defined(__GLIBC__) && !defined(__UCLIBC__)) || defined(__UCLIBC_HAS_ARGP__)
  int i;
  argp_parse (&test_argp, argc, argv, 0, &i, NULL);
  return 0;
#else
  return 23;
#endif
}

#define TEST_FUNCTION do_test (argc, argv)
#include "../test-skeleton.c"
