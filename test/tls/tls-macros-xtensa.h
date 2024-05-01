#if defined(__XTENSA_WINDOWED_ABI__)
#define TLS_GD(x)							\
  ({ int *__l;								\
     __asm__ ("movi  a8, " #x "@TLSFUNC\n\t"				\
	  "movi a10, " #x "@TLSARG\n\t"					\
	  "callx8.tls a8, " #x "@TLSCALL\n\t"				\
	  "mov %0, a10\n\t"						\
	  : "=r" (__l)							\
	  :								\
	  : "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15");	\
      __l; })

#define TLS_LD(x)							\
  ({ int *__l;								\
     __asm__ ("movi  a8, _TLS_MODULE_BASE_@TLSFUNC\n\t"			\
	  "movi a10, _TLS_MODULE_BASE_@TLSARG\n\t"			\
	  "callx8.tls a8, _TLS_MODULE_BASE_@TLSCALL\n\t"		\
	  "movi %0, " #x "@DTPOFF\n\t"					\
	  "add %0, %0, a10\n\t"						\
	  : "=r" (__l)							\
	  :								\
	  : "a8", "a9", "a10", "a11", "a12", "a13", "a14", "a15");	\
      __l; })
#elif defined(__XTENSA_CALL0_ABI__)
#ifdef __FDPIC__
#define TLS_GD(x)							\
  ({ int *__l;								\
     int __t;								\
     extern unsigned long _GLOBAL_OFFSET_TABLE_[];			\
     __asm__ ("movi	%[tmp], " #x "@GOTTLSDESC\n\t"			\
	      ".reloc	., R_XTENSA_TLS_ARG, " #x "\n\t"		\
	      "add	a2, %[tmp], %[got]\n\t"				\
	      ".reloc	., R_XTENSA_TLS_FUNCDESC, " #x "\n\t"		\
	      "l32i	%[tmp], a2, 0\n\t"				\
	      "mov	a12, a11\n\t"					\
	      ".reloc	., R_XTENSA_TLS_GOT, " #x "\n\t"		\
	      "l32i	a11, %[tmp], 4\n\t"				\
	      ".reloc	., R_XTENSA_TLS_FUNC, " #x "\n\t"		\
	      "_l32i	%[tmp], %[tmp], 0\n\t"				\
	      ".reloc	., R_XTENSA_TLS_CALL, " #x "\n\t"		\
	      "callx0	%[tmp]\n\t"					\
	      "mov	a11, a12\n\t"					\
	      "mov	%[res], a2\n\t"					\
	      : [res] "=r" (__l), [tmp] "=&r" (__t)			\
	      : [got] "r" (_GLOBAL_OFFSET_TABLE_)			\
	      : "a0", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a12");\
	      __l; })

#define TLS_LD(x)							\
  ({ int *__l;								\
     int __t;								\
     extern unsigned long _GLOBAL_OFFSET_TABLE_[];			\
     __asm__ ("movi	%[tmp], _TLS_MODULE_BASE_@GOTTLSDESC\n\t"	\
	      ".reloc	., R_XTENSA_TLS_ARG, _TLS_MODULE_BASE_\n\t"	\
	      "add	a2, %[tmp], %[got]\n\t"				\
	      ".reloc	., R_XTENSA_TLS_FUNCDESC, _TLS_MODULE_BASE_\n\t"\
	      "l32i	%[tmp], a2, 0\n\t"				\
	      "mov	a12, a11\n\t"					\
	      ".reloc	., R_XTENSA_TLS_GOT, _TLS_MODULE_BASE_\n\t"	\
	      "l32i	a11, %[tmp], 4\n\t"				\
	      ".reloc	., R_XTENSA_TLS_FUNC, _TLS_MODULE_BASE_\n\t"	\
	      "_l32i	%[tmp], %[tmp], 0\n\t"				\
	      ".reloc	., R_XTENSA_TLS_CALL, _TLS_MODULE_BASE_\n\t"	\
	      "callx0	%[tmp]\n\t"					\
	      "mov	a11, a12\n\t"					\
	      "movi	%[res], " #x "@DTPOFF\n\t"			\
	      "add	%[res], %[res], a2\n\t"				\
	      : [res] "=r" (__l), [tmp] "=&r" (__t)			\
	      : [got] "r" (_GLOBAL_OFFSET_TABLE_)			\
	      : "a0", "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a12");\
	      __l; })

#define TLS_IE(x)							\
  ({ int *__l;								\
     int __t;								\
     extern unsigned long _GLOBAL_OFFSET_TABLE_[];			\
     __asm__ ("movi	%[tmp], " #x "@GOTTPOFF\n\t"			\
	      ".reloc	., R_XTENSA_TLS_TPOFF_PTR, " #x "\n\t"		\
	      "add	%[tmp], %[tmp], %[got]\n\t"			\
	      ".reloc	., R_XTENSA_TLS_TPOFF_LOAD, " #x "\n\t"		\
	      "l32i	%[tmp], %[tmp], 0\n\t"				\
	      "rur	%[res], threadptr\n\t"				\
	      "add	%[res], %[res], %[tmp]\n\t"			\
	      : [res] "=r" (__l), [tmp] "=&r" (__t)			\
	      : [got] "r" (_GLOBAL_OFFSET_TABLE_));			\
     __l; })

#define TLS_LE(x)							\
  ({ int *__l;								\
     int __t;								\
     __asm__ ("rur	%0, threadptr\n\t"				\
	      "movi	%1, " #x "@TPOFF\n\t"				\
	      "add	%0, %0, %1\n\t"					\
	      : "=r" (__l), "=r" (__t) );				\
     __l; })
#else
#define TLS_GD(x)							\
  ({ int *__l;								\
     __asm__ ("movi  a0, " #x "@TLSFUNC\n\t"				\
	  "movi a2, " #x "@TLSARG\n\t"					\
	  "callx0.tls a0, " #x "@TLSCALL\n\t"				\
	  "mov %0, a2\n\t"						\
	  : "=r" (__l)							\
	  :								\
	  : "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11");\
      __l; })

#define TLS_LD(x)							\
  ({ int *__l;								\
     __asm__ ("movi  a0, _TLS_MODULE_BASE_@TLSFUNC\n\t"			\
	  "movi a2, _TLS_MODULE_BASE_@TLSARG\n\t"			\
	  "callx0.tls a0, _TLS_MODULE_BASE_@TLSCALL\n\t"		\
	  "movi %0, " #x "@DTPOFF\n\t"					\
	  "add %0, %0, a2\n\t"						\
	  : "=r" (__l)							\
	  :								\
	  : "a2", "a3", "a4", "a5", "a6", "a7", "a8", "a9", "a10", "a11");\
      __l; })
#endif
#else
#error Unsupported Xtensa ABI
#endif

#ifndef __FDPIC__
#define TLS_IE(x) TLS_LE(x)

#define TLS_LE(x)							\
  ({ int *__l;								\
     int __t;								\
     __asm__ ("rur %0, threadptr\n\t"					\
	  "movi %1, " #x "@TPOFF\n\t"					\
	  "add %0, %0, %1\n\t"						\
	  : "=r" (__l), "=r" (__t) );					\
     __l; })
#endif
