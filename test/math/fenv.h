/* until we support fenv ... */
#define __FE_UNDEFINED 0
#define feclearexcept(X)
#define fetestexcept(X) (0)
#define fegetround(X) (0)
#define fesetround(X) (0)
#define issignaling(X) (0)
