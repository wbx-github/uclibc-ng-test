/* Macros to support TLS testing in times of missing compiler support.
   kvx: the sequences gcc emits for each model; $r13 is the thread pointer,
   and ";;" closes a bundle.

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

#define TLS_GD(x)                                       \
        ({ void *__result;                              \
        extern void *__tls_get_addr (void *);           \
        asm ("pcrel %0 = @gotaddr()\n\t;;\n\t"          \
             "addd %0 = %0, @tlsgd(" #x ")\n\t;;\n\t"   \
             : "=r" (__result));                        \
        __tls_get_addr (__result); })

#define TLS_LD(x)                                       \
        ({ void *__base, *__result;                     \
        extern void *__tls_get_addr (void *);           \
        asm ("pcrel %0 = @gotaddr()\n\t;;\n\t"          \
             "addd %0 = %0, @tlsld(" #x ")\n\t;;\n\t"   \
             : "=r" (__base));                          \
        __base = __tls_get_addr (__base);               \
        asm ("addd %0 = %1, @dtpoff(" #x ")\n\t;;\n\t"  \
             : "=r" (__result) : "r" (__base));         \
        __result; })

#define TLS_IE(x)                                       \
        ({ void *__result;                              \
        asm ("pcrel %0 = @gotaddr()\n\t;;\n\t"          \
             "ld %0 = @tlsie(" #x ")[%0]\n\t;;\n\t"     \
             "addd %0 = $r13, %0\n\t;;\n\t"             \
             : "=r" (__result));                        \
        __result; })

#define TLS_LE(x)                                       \
        ({ void *__result;                              \
        asm ("addd %0 = $r13, @tlsle(" #x ")\n\t;;\n\t" \
             : "=r" (__result));                        \
        __result; })
