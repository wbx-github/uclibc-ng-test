/* Macros to support TLS testing in times of missing compiler support.
   LoongArch, after tls-macros-riscv.h: the assembler expands the la.tls.*
   pseudo instructions into the relocation sequences of each model.

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
   <https://www.gnu.org/licenses/>.  */

#if __loongarch_grlen == 64
# define TLS_LA_ADD "add.d"
#else
# define TLS_LA_ADD "add.w"
#endif

#define TLS_GD(x)                                       \
        ({ void *__result;                              \
        extern void *__tls_get_addr (void *);           \
        asm ("la.tls.gd %0, " #x "\n\t"                 \
             : "=r" (__result));                        \
        __tls_get_addr (__result); })

#define TLS_LD(x) TLS_GD(x)

#define TLS_IE(x)                                       \
        ({ void *__result;                              \
        asm ("la.tls.ie %0, " #x "\n\t"                 \
             TLS_LA_ADD " %0, %0, $tp\n\t"              \
             : "=r" (__result));                        \
        __result; })

#define TLS_LE(x)                                       \
        ({ void *__result;                              \
        asm ("la.tls.le %0, " #x "\n\t"                 \
             TLS_LA_ADD " %0, %0, $tp\n\t"              \
             : "=r" (__result));                        \
        __result; })
