#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../test-skeleton.h"

#ifdef __ARCH_USE_MMU__

#define NKEYS 100
static pthread_key_t keys[NKEYS];
static pthread_barrier_t b;


static void *
tf (void *arg)
{
  void *res = NULL;
  for (int i = 0; i < NKEYS; ++i)
    {
      void *p = pthread_getspecific (keys[i]);
      pthread_setspecific (keys[i], (void *) 7);
      if (p != NULL)
	res = p;
    }
  if (arg != NULL)
    {
      pthread_barrier_wait (arg);
      pthread_barrier_wait (arg);
    }
  return res;
}


static int
do_test (void)
{
  pthread_barrier_init (&b, NULL, 2);

  for (int i = 0; i < NKEYS; ++i)
    if (pthread_key_create (&keys[i], NULL) != 0)
      {
	puts ("cannot create keys");
	return 1;
      }

  pthread_t th;
  if (pthread_create (&th, NULL, tf, &b) != 0)
    {
      puts ("cannot create thread in parent");
      return 1;
    }

  pthread_barrier_wait (&b);

  pid_t pid = fork ();
  if (pid == 0)
    {
      if (pthread_create (&th, NULL, tf, NULL) != 0)
	{
	  puts ("cannot create thread in child");
	  exit (1);
	}

      void *res;
      pthread_join (th, &res);

      exit (res != NULL);
    }
  else if (pid == -1)
    {
      puts ("cannot create child process");
      return 1;
    }

  int s;
  if (TEMP_FAILURE_RETRY (waitpid (pid, &s, 0)) != pid)
    {
      puts ("failing to wait for child process");
      return 1;
    }

  pthread_barrier_wait (&b);
  pthread_join (th, NULL);

  return !WIFEXITED (s) ? 2 : WEXITSTATUS (s);
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"

#else

int main(void)
{
    printf("Skipping test on non-mmu host!\n");
    return 23;
}

#endif
