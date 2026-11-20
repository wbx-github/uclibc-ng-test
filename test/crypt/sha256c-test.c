#include <crypt.h>
#include <stdio.h>
#include <string.h>

static const struct
{
  const char *salt;
  const char *input;
  const char *expected;
} tests[] =
{
  { "$5$saltstring", "Hello world!",
    "$5$saltstring$5B8vYYiY.CVt1RlTTf8KbXBH3hsxY/GNooZaBBGWEc5" },
  { "$5$rounds=2000$saltstringsaltstring", "Hello world!",
    "$5$rounds=2000$saltstringsaltst$582cUmqhBG5P2P5q0Q13Bee5SB0.Ee.tH"
    "Tbzb2UMg73" },
  { "$5$rounds=5000$toolongsaltstring", "This is just a test",
    "$5$rounds=5000$toolongsaltstrin$Un/5jzAHMgOGZ5.mWJpuVolil07guHPvOW8"
    "mGRcvxa5" },
  { "$5$rounds=1400$anotherlongsaltstring",
    "a very much longer text to encrypt.  This one even stretches over more"
    "than one line.",
    "$5$rounds=1400$anotherlongsalts$Rx.j8H.h8HjEDGomFU8bDkXm3XIUnzyxf12"
    "oP84Bnq1" },
  { "$5$rounds=1777$short", "we have a short salt string but not a short password",
    "$5$rounds=1777$short$XmDTckYpEM9CD37qgLk0RuCWoK5yiQ8JXV.G0dg93xB" },
  { "$5$rounds=1234$asaltof16chars..", "a short string",
    "$5$rounds=1234$asaltof16chars..$hLa/Rz9bcawSsyDsqz83eUuG5ydmTGhkM"
    "GB5XIx4wtD" },
  { "$5$rounds=10$roundstoolow", "the minimum number is still observed",
    "$5$rounds=1000$roundstoolow$yfvwcWrQ8l/K0DAWyuPMDNHpIVlTQebY9l/gL97"
    "2bIC" },
};
#define ntests (sizeof (tests) / sizeof (tests[0]))



static int
do_test (void)
{
#if __UCLIBC_HAS_SHA256_CRYPT_IMPL__
  int result = 0;

  int i;

  for (i = 0; i < ntests; ++i)
    {
      char *cp = crypt (tests[i].input, tests[i].salt);

      if (strcmp (cp, tests[i].expected) != 0)
	{
	  printf ("test %d: expected \"%s\", got \"%s\"\n",
		  i, tests[i].expected, cp);
	  result = 1;
	}
    }

  return result;
#else
  return 23;
#endif

}

#define TIMEOUT 6
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
