#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <link.h>

#define TEST_FUNCTION do_test ()
static int
do_test (void)
{
#ifdef USE_TLS
  static const char modname[] = "tst-tlsmod3.so";
  int result = 0;
  int (*fp) (void);
  void *h;
  int i;
  int modid = -1;

  for (i = 0; i < 10; ++i)
    {
      h = dlopen (modname, RTLD_LAZY);
      if (h == NULL)
	{
	  printf ("cannot open '%s': %s\n", modname, dlerror ());
	  exit (1);
	}

      fp = dlsym (h, "in_dso2");
      if (fp == NULL)
	{
	  printf ("cannot get symbol 'in_dso2': %s\n", dlerror ());
	  exit (1);
	}

      result |= fp ();

      dlclose (h);
    }

  return result;
#else
  return 0;
#endif
}


#include "../test-skeleton.c"
