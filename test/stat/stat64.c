#include <features.h>

#if defined __UCLIBC__ && !defined __UCLIBC_HAS_LFS__

/* Without UCLIBC_HAS_LFS the *64 syscalls are not built, so this is a link
   error rather than a compile error -- which takes the whole directory down
   just the same. */
#include <stdio.h>
int
main (void)
{
  puts ("needs UCLIBC_HAS_LFS");
  return 23;			/* SKIP */
}

#else

#include "stat.c"

#endif
