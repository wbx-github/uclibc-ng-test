/* Argp example #1 -- a minimal program using argp */

/* This is (probably) the smallest possible program that
   uses argp.  It won't do much except give an error
   messages and exit when there are any arguments, and print
   a (rather pointless) messages for --help.  */

#include <stdlib.h>
#if (defined(__GLIBC__) && !defined(__UCLIBC__)) || defined(__UCLIBC_HAS_ARGP__)
#include <argp.h>
#endif

int main (int argc, char **argv)
{
#if (defined(__GLIBC__) && !defined(__UCLIBC__)) || defined(__UCLIBC_HAS_ARGP__)
  argp_parse (0, argc, argv, 0, 0, 0);
  exit (0);
#else
  return 23;
#endif
}
