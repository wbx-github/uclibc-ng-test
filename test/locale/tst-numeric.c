/* Testing the implementation of LC_NUMERIC and snprintf().
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Petter Reinholdtsen <pere@hungry.com>, 2003

   Based on tst-fmon.c by Jochen Hein <jochen.hein@delphi.central.de>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stdio.h>
#include <locale.h>
#include <string.h>
#include <stdlib.h>

/*
  test-numeric gets called with four parameters:
   - the locale
   - the format-string to be used
   - the actual number to be formatted
   - the expected string
   If the test passes, test-numeric terminates with returncode 0,
   otherwise with 1.

   uClibc-ng note: glibc drives this from tst-numeric.sh with a table of
   cases.  There is no such driver here, so when called without the four
   arguments we run a built-in table of LC_NUMERIC cases (de/en/fr) instead.
   We exercise thousands grouping only on integers ("%'ld"): uClibc-ng does
   not yet group floats (see the _dtostr() note in libc/stdio/_vfprintf.c),
   so floats are used only to check the decimal radix character.
*/
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define EXIT_SETLOCALE 2
#define EXIT_SNPRINTF 3

/* fr_FR groups thousands with U+202F NARROW NO-BREAK SPACE (harvested from
   the build host's glibc; modern glibc/CLDR use U+202F, older used U+00A0).
   Written as explicit UTF-8 bytes in its own string literal so the compiler's
   execution charset is irrelevant and \x does not consume the next digit.  */
#define NNBSP "\xe2\x80\xaf"

static int
check (const char *locale, const char *fmt, const char *got,
       const char *expected)
{
  int ok = strcmp (got, expected) == 0;
  printf ("locale: \"%s\", format: \"%s\", expected: \"%s\", got: \"%s\" => %s\n",
	  locale, fmt, expected, got, ok ? "correct" : "false");
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* Original glibc behaviour: a single double case taken from argv.  */
static int
run_arg (const char *locale, const char *fmt, const char *number,
	 const char *expected)
{
  char s[201];
  double val = strtod (number, NULL);

  if (setlocale (LC_ALL, locale) == NULL)
    {
      fprintf (stderr, "setlocale(LC_ALL, \"%s\"): %m\n", locale);
      return EXIT_SETLOCALE;
    }
  if (snprintf (s, sizeof (s), fmt, val) == -1)
    {
      perror ("snprintf");
      return EXIT_SNPRINTF;
    }
  return check (locale, fmt, s, expected);
}

int
main (int argc, char *argv[])
{
  if (argc >= 5)
    return run_arg (argv[1], argv[2], argv[3], argv[4]);

  /* is_int selects %ld (long) vs %f (double) formatting.  */
  static const struct
  {
    const char *locale, *fmt;
    int is_int;
    const char *number, *expected;
  } cases[] = {
    /* Integer thousands grouping (the ' flag).  */
    { "de_DE.UTF-8", "%'ld", 1, "1234567", "1.234.567" },
    { "en_US.UTF-8", "%'ld", 1, "1234567", "1,234,567" },
    { "fr_FR.UTF-8", "%'ld", 1, "1234567", "1" NNBSP "234" NNBSP "567" },
    /* Decimal radix character.  */
    { "de_DE.UTF-8", "%.2f", 0, "1234.56", "1234,56" },
    { "en_US.UTF-8", "%.2f", 0, "1234.56", "1234.56" },
    { "fr_FR.UTF-8", "%.2f", 0, "1234.56", "1234,56" },
  };

  int result = EXIT_SUCCESS;
  for (size_t i = 0; i < sizeof (cases) / sizeof (cases[0]); ++i)
    {
      char s[201];
      /* Parse the number in the C locale before switching.  */
      long lval = strtol (cases[i].number, NULL, 10);
      double dval = strtod (cases[i].number, NULL);

      if (setlocale (LC_ALL, cases[i].locale) == NULL)
	{
	  fprintf (stderr, "setlocale(LC_ALL, \"%s\"): %m\n", cases[i].locale);
	  result = EXIT_FAILURE;
	  continue;
	}
      if (cases[i].is_int)
	snprintf (s, sizeof (s), cases[i].fmt, lval);
      else
	snprintf (s, sizeof (s), cases[i].fmt, dval);

      if (check (cases[i].locale, cases[i].fmt, s, cases[i].expected)
	  != EXIT_SUCCESS)
	result = EXIT_FAILURE;

      setlocale (LC_ALL, "C");
    }

  return result;
}
