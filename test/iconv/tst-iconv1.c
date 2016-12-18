/* Test case by yaoz@nih.gov.  */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined(__GLIBC__) && !defined(__UCLIBC__)
#include <iconv.h>
#endif

static int
do_test (void)
{
#if defined(__GLIBC__) && !defined(__UCLIBC__)
  char utf8[5];
  wchar_t ucs4[5];
  iconv_t cd;
  char *inbuf;
  char *outbuf;
  size_t inbytes;
  size_t outbytes;
  size_t n;

  strcpy (utf8, "abcd");

  /* From UTF8 to UCS4. */
  cd = iconv_open ("UCS4", "UTF8");
  if (cd == (iconv_t) -1)
    {
      perror ("iconv_open");
      return 1;
    }

  inbuf = utf8;
  inbytes = 4;
  outbuf = (char *) ucs4;
  outbytes = 4 * sizeof (wchar_t);    /* "Argument list too long" error. */
  n = iconv (cd, &inbuf, &inbytes, &outbuf, &outbytes);
  if (n == (size_t) -1)
    {
      printf ("iconv: %m\n");
      iconv_close (cd);
      return 1;
    }
  iconv_close (cd);

  return 0;
#else
  return 23;
#endif
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
