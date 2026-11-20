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
  { "$6$saltstring", "Hello world!",
    "$6$saltstring$svn8UoSVapNtMuq1ukKS4tPQd8iKwSMHWjl/O817G3uBnIFNjnQJu"
    "esI68u4OTLiBFdcbYEdFCoEOfaS35inz1" },
  { "$6$rounds=2000$saltstringsaltstring", "Hello world!",
    "$6$rounds=2000$saltstringsaltst$ExMQRIOBwGFyDkFHJkxf3OMEB0SecFHArw"
    "ODUtFxpuj4rxyaNZoy3paKfi5uPQh1lCMN9KISj7z8wg8Yksj2G/" },
  { "$6$rounds=5000$toolongsaltstring", "This is just a test",
    "$6$rounds=5000$toolongsaltstrin$lQ8jolhgVRVhY4b5pZKaysCLi0QBxGoNeKQ"
    "zQ3glMhwllF7oGDZxUhx1yxdYcz/e1JSbq3y6JMxxl8audkUEm0" },
  { "$6$rounds=1400$anotherlongsaltstring",
    "a very much longer text to encrypt.  This one even stretches over more"
    "than one line.",
    "$6$rounds=1400$anotherlongsalts$POfYwTEok97VWcjxIiSOjiykti.o/pQs.wP"
    "vMxQ6Fm7I6IoYN3CmLs66x9t0oSwbtEW7o7UmJEiDwGqd8p4ur1" },
  { "$6$rounds=1777$short", "we have a short salt string but not a short password",
    "$6$rounds=1777$short$npftYrqsQ/uyghE82ahlbo5bAk3gKb/hGB4A9Lz7Fs76L"
    "fc1YAnlK9sZzBVUWHcDRNevYwVtKVLaOYRhd0vxJ/" },
  { "$6$rounds=1234$asaltof16chars..", "a short string",
    "$6$rounds=1234$asaltof16chars..$Rvw21r4rdL3t7MDylfOt8ug5ULy4YzR.75"
    "yoyVvQ4dCui/ypt1nAasxWHxp6Z6WIfpk81AYcUU2kTmTXhlX5P." },
  { "$6$rounds=10$roundstoolow", "the minimum number is still observed",
    "$6$rounds=1000$roundstoolow$kUMsbe306n21p9R.FRkW3IGn.S9NPN0x50YhH1x"
    "hLsPuWGsUSklZt58jaTfF4ZEQpyUNGc0dqbpBYYBaHHrsX." },
};
#define ntests (sizeof (tests) / sizeof (tests[0]))


static int
do_test (void)
{
#if __UCLIBC_HAS_SHA512_CRYPT_IMPL__
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
