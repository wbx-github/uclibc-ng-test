/* For now */
#define TLS_LD(x)	TLS_IE(x)

#define TLS_GD(x)					\
  ({ void *__result;					\
     extern void *__tls_get_addr (void *);      \
     __asm__ ("add %0, pcl, @" #x "@tlsgd \n"     \
        ".tls_gd_ld " #x " \n"    \
      : "=r" (__result));		\
     (int *)__tls_get_addr(__result); })

#define TLS_LE(x)					\
  ({ int *__result;					\
     void *tp = __builtin_thread_pointer();		                \
     __asm__ ("add %0, %1, @" #x "@tpoff   \n"		\
	  : "=r" (__result) : "r"(tp));	        \
     __result; })

#define TLS_IE(x)					\
  ({ int *__result;					\
     void *tp = __builtin_thread_pointer();		                \
     __asm__ ("ld %0, [pcl, @" #x "@tlsie]      \n"     \
	  "add %0, %1, %0                       \n"				\
	  : "=&r" (__result) : "r" (tp));		\
     __result; })

