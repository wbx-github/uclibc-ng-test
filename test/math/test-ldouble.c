/* Copyright (C) 1997, 1999, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1997.

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

/* Both before math.h: __NO_MATH_INLINES so the header leaves the inline
   definitions out, _GNU_SOURCE for M_PIl and the other GNU names that
   libm-test.inc uses -- it defines _GNU_SOURCE itself, but only reaches that
   point after the include below.  */
#ifndef __NO_MATH_INLINES
# define __NO_MATH_INLINES
#endif
#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <math.h>

/* Without UCLIBC_HAS_LONG_DOUBLE_MATH libm has no *l entry point at all and
   the test below would not link, so say so and skip.  math.h reports it as
   __NO_LONG_DOUBLE_MATH; the Makefile cannot, because the test suite never
   reads uClibc-ng's .config.  23 is the runner's skip status.  */
#ifdef __NO_LONG_DOUBLE_MATH

# include <stdio.h>

int main(void)
{
	puts("SKIP: built without UCLIBC_HAS_LONG_DOUBLE_MATH, libm has no *l");
	return 23;
}

#else

#define FUNC(function) function##l
#define FLOAT long double
#define TEST_MSG "testing long double (without inline functions)\n"
#define MATHCONST(x) x##L
#define CHOOSE(Clongdouble,Cdouble,Cfloat,Cinlinelongdouble,Cinlinedouble,Cinlinefloat) Clongdouble
#define PRINTF_EXPR "Le"
#define PRINTF_XEXPR "La"
#define PRINTF_NEXPR "Lf"
#define TEST_LDOUBLE 1

#include "libm-test.c"

#endif
