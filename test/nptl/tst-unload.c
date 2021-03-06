/* Tests for non-unloading of libpthread.
   Copyright (C) 2000, 2002, 2003, 2004 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2000.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static int
do_test (void)
{
  void *p = dlopen ("libpthread.so.1", RTLD_LAZY);

  if (p == NULL)
    {
      puts ("failed to load libpthread.so.1");
      return 1;
    }

  if (dlclose (p) != 0)
    {
      puts ("dlclose (libpthread.so.1) failed");
      return 1;
    }

  puts ("seems to work");

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
