#include <features.h>

#if defined __UCLIBC__ \
    && (!defined __UCLIBC_HAS_WCHAR__ || !defined __UCLIBC_HAS_LOCALE__)

/* wchar.h is not installed without UCLIBC_HAS_WCHAR, so the body cannot even
   be parsed there -- guarding only main's return would not compile. */
#include <stdio.h>
int
main (void)
{
  puts ("needs UCLIBC_HAS_WCHAR and UCLIBC_HAS_LOCALE");
  return 23;			/* SKIP */
}

#else

#include <stdio.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>

#define NUM_OF_DATES 7
#define NUM_OF_LOCALES 3
#define BUF_SIZE 256

int
main (void)
{
  wchar_t buf[BUF_SIZE];
  struct tm *tp;
  time_t time_list[NUM_OF_DATES] = {
	  500, 68200000, 694223999,
	  694224000, 704900000, 705000000,
	  705900000
  };
  char *locale_list[NUM_OF_LOCALES] = {
	  "C",
	  "fr_FR.ISO-8859-1",
	  "ja_JP.UTF-8"
  };
  int result = 0, ddd, lll;
  size_t n;

  for (lll = 0; lll < NUM_OF_LOCALES; lll++) {
	  printf ("\nUsing locale: %s\n", locale_list[lll]);
	  char* set = setlocale(LC_ALL, locale_list[lll]);
	  if (set == NULL) {
		  printf ("FAILED!\n\n");
		  continue;
	  } else
		  printf ("\n");
	  for (ddd = 0; ddd < NUM_OF_DATES; ddd++) {
		  tp = localtime(&time_list[ddd]);
		  printf ("%ld corresponds to ", time_list[ddd]);

		  n = wcsftime (buf, sizeof (buf) / sizeof (buf[0]),
				L"%H:%M:%S  %Y-%m-%d%n", tp);
		  if (n != 21) {
			result = 1;
			printf ("FAILED!\n");
		  }

		  printf ("%ls", buf);

		  wcsftime (buf, sizeof (buf) / sizeof (buf[0]),
			L"%tor, as %%D %%T: %D %T%n", tp);
		  printf ("%ls", buf);

		  wcsftime (buf, sizeof (buf) / sizeof (buf[0]), L"%A (%a)%n", tp);
		  printf ("The weekday was %ls", buf);

		  wcsftime (buf, sizeof (buf) / sizeof (buf[0]), L"%B (%b) %Y%n", tp);
		  /* glibc bug? forgets aigu from french february février
		   * See s/printf (/wprintf (L/g */
		  //wprintf (L"Month was %ls", buf);
		  printf ("Month was %ls", buf);
	  }
  }
  return result;
}

#endif
