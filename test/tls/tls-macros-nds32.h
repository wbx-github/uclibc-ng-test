/* Macros to support TLS testing in times of missing compiler support.
   nds32: the sequences gcc emits for each model, $r25 is the thread
   pointer.  gcc has no traditional dialect here, so both dynamic models go
   through a TLS descriptor, as compiled code does.  The .relax_hint groups
   are what makes the assembler emit the descriptor relocations; %= keeps
   them apart between expansions.

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

#define TLS_NDS32_GOT                                   \
        "push.s { $gp $lp }\n\t"                        \
        ".relax_hint 1%=\n\t"                           \
        "sethi $gp, hi20(_GLOBAL_OFFSET_TABLE_-8)\n\t"  \
        ".relax_hint 1%=\n\t"                           \
        "ori $gp, $gp, lo12(_GLOBAL_OFFSET_TABLE_-4)\n\t" \
        "add5.pc $gp\n\t"

/* The descriptor function may end in __tls_get_addr: a call.  */
#define TLS_NDS32_CALL_CLOBBERS                         \
        "$r0", "$r1", "$r2", "$r3", "$r4", "$r5",       \
        "$r15", "$r16", "$r17", "$r18", "$r19", "$r20", \
        "$r21", "$r22", "$r23", "$r24", "memory"

#define TLS_GD(x)                                       \
        ({ void *__result;                              \
        asm volatile (TLS_NDS32_GOT                     \
             ".relax_hint 2%=\n\t"                      \
             "sethi $r0, hi20(" #x "@TLSDESC)\n\t"      \
             ".relax_hint 2%=\n\t"                      \
             "ori $r0, $r0, lo12(" #x "@TLSDESC)\n\t"   \
             ".relax_hint 2%=\n\t"                      \
             "lw $r15, [$r0 + $gp]\n\t"                 \
             ".relax_hint 2%=\n\t"                      \
             "add $r0, $r0, $gp\n\t"                    \
             ".relax_hint 2%=\n\t"                      \
             "jral $r15\n\t"                            \
             "pop.s { $gp $lp }\n\t"                    \
             "move %0, $r0\n\t"                         \
             : "=r" (__result) : : TLS_NDS32_CALL_CLOBBERS); \
        __result; })

#define TLS_LD(x) TLS_GD(x)

#ifdef __PIC__
# define TLS_IE(x)                                      \
        ({ void *__result;                              \
        asm (TLS_NDS32_GOT                              \
             ".relax_hint 2%=\n\t"                      \
             "sethi $r0, hi20(" #x "@GOTTPOFF)\n\t"     \
             ".relax_hint 2%=\n\t"                      \
             "ori $r0, $r0, lo12(" #x "@GOTTPOFF)\n\t"  \
             ".relax_hint 2%=\n\t"                      \
             "lw $r0, [$r0 + $gp]\n\t"                  \
             "pop.s { $gp $lp }\n\t"                    \
             "add %0, $r0, $r25\n\t"                    \
             : "=r" (__result) : : "$r0");              \
        __result; })
#else
# define TLS_IE(x)                                      \
        ({ void *__result;                              \
        asm (".relax_hint 2%=\n\t"                      \
             "sethi %0, hi20(" #x "@GOTTPOFF)\n\t"      \
             ".relax_hint 2%=\n\t"                      \
             "lwi %0, [%0 + lo12(" #x "@GOTTPOFF)]\n\t" \
             "add %0, %0, $r25\n\t"                     \
             : "=r" (__result));                        \
        __result; })
#endif

#define TLS_LE(x)                                       \
        ({ void *__result;                              \
        asm (".relax_hint 3%=\n\t"                      \
             "sethi %0, hi20(" #x "@TPOFF)\n\t"         \
             ".relax_hint 3%=\n\t"                      \
             "ori %0, %0, lo12(" #x "@TPOFF)\n\t"       \
             "add %0, %0, $r25\n\t"                     \
             : "=r" (__result));                        \
        __result; })
