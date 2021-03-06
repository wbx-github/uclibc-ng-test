#define __TLS_CALL_CLOBBERS						\
	"0", "4", "5", "6", "7", "8", "9", "10", "11", "12",	\
	"lr", "ctr", "cr0", "cr1", "cr5", "cr6", "cr7"

/* PowerPC32 Local Exec TLS access.  */
#define TLS_LE(x)				\
  ({ int *__result;				\
     __asm__ ("addi %0,2," #x "@tprel"		\
	  : "=r" (__result));			\
     __result; })

/* PowerPC32 Initial Exec TLS access.  */
#ifdef HAVE_ASM_PPC_REL16
# define TLS_IE(x)					\
  ({ int *__result;					\
     __asm__ ("bcl 20,31,1f\n1:\t"				\
	  "mflr %0\n\t"					\
	  "addis %0,%0,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"	\
	  "addi %0,%0,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"	\
	  "lwz %0," #x "@got@tprel(%0)\n\t"		\
	  "add %0,%0," #x "@tls"			\
	  : "=b" (__result) :				\
	  : "lr");					\
     __result; })
#else
# define TLS_IE(x)					\
  ({ int *__result;					\
     __asm__ ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"	\
	  "mflr %0\n\t"					\
	  "lwz %0," #x "@got@tprel(%0)\n\t"		\
	  "add %0,%0," #x "@tls"			\
	  : "=b" (__result) :				\
	  : "lr");					\
     __result; })
#endif

/* PowerPC32 Local Dynamic TLS access.  */
#ifdef HAVE_ASM_PPC_REL16
# define TLS_LD(x)					\
  ({ int *__result;					\
     __asm__ ("bcl 20,31,1f\n1:\t"				\
	  "mflr 3\n\t"					\
	  "addis 3,3,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"	\
	  "addi 3,3,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"	\
	  "addi 3,3," #x "@got@tlsld\n\t"		\
	  "bl __tls_get_addr@plt\n\t"			\
	  "addi %0,3," #x "@dtprel"			\
	  : "=r" (__result) :				\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
#else
# define TLS_LD(x)					\
  ({ int *__result;					\
     __asm__ ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"	\
	  "mflr 3\n\t"					\
	  "addi 3,3," #x "@got@tlsld\n\t"		\
	  "bl __tls_get_addr@plt\n\t"			\
	  "addi %0,3," #x "@dtprel"			\
	  : "=r" (__result) :				\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
#endif

/* PowerPC32 General Dynamic TLS access.  */
#ifdef HAVE_ASM_PPC_REL16
# define TLS_GD(x)					\
  ({ register int *__result __asm__ ("r3");		\
     __asm__ ("bcl 20,31,1f\n1:\t"				\
	  "mflr 3\n\t"					\
	  "addis 3,3,_GLOBAL_OFFSET_TABLE_-1b@ha\n\t"	\
	  "addi 3,3,_GLOBAL_OFFSET_TABLE_-1b@l\n\t"	\
	  "addi 3,3," #x "@got@tlsgd\n\t"		\
	  "bl __tls_get_addr@plt"			\
	  : :						\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
#else
# define TLS_GD(x)					\
  ({ register int *__result __asm__ ("r3");		\
     __asm__ ("bl _GLOBAL_OFFSET_TABLE_@local-4\n\t"	\
	  "mflr 3\n\t"					\
	  "addi 3,3," #x "@got@tlsgd\n\t"		\
	  "bl __tls_get_addr@plt"			\
	  : :						\
	  : __TLS_CALL_CLOBBERS);			\
     __result; })
#endif
