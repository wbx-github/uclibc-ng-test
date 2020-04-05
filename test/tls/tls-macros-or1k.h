/* Macros to support TLS testing, OpenRISC version.
   Copyright (C) 2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#define TLS_LOAD_GOT \
  ({ register long lr __asm__ ("r9");					\
     long got;								\
     asm ("l.jal    0x8\n\t"						\
	  " l.movhi %0, gotpchi(_GLOBAL_OFFSET_TABLE_-4)\n\t"		\
	  "l.ori    %0, %0, gotpclo(_GLOBAL_OFFSET_TABLE_+0)\n\t"	\
	  "l.add    %0, %0, %1"						\
	  : "=r" (got), "=r" (lr));					\
     got; })

/* General Dynamic:
     l.movhi r17, tlsgdhi(symbol)
     l.ori   r17, r17, tlsgdlo(symbol)
     l.add   r17, r17, r16
     l.or    r3, r17, r17
     l.jal   plt(__tls_get_addr)
      l.nop  */

#define TLS_GD(x)					\
  ({ void *__tlsgd;					\
     extern void *__tls_get_addr (void *);          	\
     asm ("l.movhi %0, tlsgdhi(" #x ")\n\t"		\
	  "l.ori   %0, %0, tlsgdlo(" #x ")\n\t"		\
	  : "=r" (__tlsgd));				\
     (int *) __tls_get_addr (TLS_LOAD_GOT + __tlsgd); })

#define TLS_LD(x) TLS_GD(x)

/* Initial Exec:
     l.movhi r17, gottpoffhi(symbol)
     l.add   r17, r17, r16
     l.lwz   r17, gottpofflo(symbol)(r17)
     l.add   r17, r17, r10
     l.lbs   r17, 0(r17)  */

#define TLS_IE(x)					\
  ({ register long __tls __asm__ ("r10");		\
     void *__tlsie;					\
     asm ("l.movhi  %0, gottpoffhi(" #x ")\n\t"		\
	  "l.add    %0, %0, %1\n\t"			\
	  "l.lwz    %0, gottpofflo(" #x ")(%0)\n\t"	\
	  "l.add    %0, %0, %2\n\t"			\
	  : "=&r" (__tlsie) : "r" (TLS_LOAD_GOT),	\
	    "r" (__tls) : "memory");			\
     __tlsie; })

/* Local Exec:
     l.movhi r17, tpoffha(symbol)
     l.add   r17, r17, r10
     l.addi  r17, r17, tpofflo(symbol)
     l.lbs   r17, 0(r17)  */

#define TLS_LE(x)					\
  ({ register long __tls __asm__ ("r10");		\
     void *__tlsle;					\
     asm ("l.movhi  %0, tpoffha(" #x ")\n\t"		\
	  "l.add    %0, %0, %1\n\t"			\
	  "l.addi   %0, %0, tpofflo(" #x ")\n\t"	\
	  : "=&r" (__tlsle) : "r" (__tls) : "memory");	\
     __tlsle; })
