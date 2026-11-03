# Generating the reference tables

`../exp-ref.h`, `../hypot-ref.h`, `../expm1-ref.h`, `../erf-ref.h`, `../lgamma-ref.h`,
`../tgamma-ref.h`, `../sin-ref.h`, `../sinh-ref.h`, `../tan-ref.h`, `../log-ref.h`,
`../exp2-ref.h`, `../pow-ref.h`, `../bessel-ref.h`, `../sqrt-ref.h` and `../fma-ref.h`
hold correctly rounded
results for a fixed set of inputs, and the budgets in the matching
`../tst-*-ulp.c` come from the same run. The guest cannot compute a reference of its own: uClibc-ng's
`long double` is a wrapper around `double` on most ports, so `expl` is the very
function under test.

Each header carries two tables, one for `double` and one for `float`. The `float`
entry points are not separate implementations — `libm/w_expf.c`,
`libm/w_hypotf.c` and `libm/float_wrappers.c` compute in `double` and convert —
but they need their own inputs, because `expf(x)` is the correctly rounded `exp`
of the `float` x, not of the `double` it was derived from.

These generators run on the **host** and expect the implementations linked in under
distinct names:

| symbol | where it comes from |
|---|---|
| `fd_exp`, `fd_hypot`, `fd_expm1`, `fd_erf`, `fd_erfc` | uClibc-ng `libm/e_exp.c`, `libm/e_hypot.c`, `libm/s_expm1.c`, `libm/s_erf.c`, `libm/e_lgamma_r.c` (fdlibm, the SMALL variant) |
| `exp`, `gl_hypot`, `gl_expm1` | `libm/optimized/e_exp.c` (Arm), `libm/optimized/e_hypot.c` and `libm/optimized/s_expm1.c` (glibc) |
| `cr_exp`, `cr_hypot`, `cr_expm1`, `cr_erf`, `cr_erfc`, `cr_lgamma`, `cr_tgamma` | `libm/accurate/…` (CORE-MATH) — these are the reference |
| `cr_expf`, `cr_hypotf`, `cr_expm1f`, `cr_erff`, `cr_erfcf`, `cr_lgammaf`, `cr_tgammaf` | CORE-MATH `src/binary32/…` — the reference for the float tables |
| `arm_erf` | `libm/optimized/s_erf.c` (Arm), which needs `erf_data.c` beside it |

Building them means compiling those sources for the host with the function renamed,
which needs a little scaffolding, because the in-tree files expect the uClibc-ng
build environment:

- `libm/e_*.c` want `math_private.h`; a short header providing `GET_HIGH_WORD`,
  `SET_HIGH_WORD`, `EXTRACT_WORDS`, `INSERT_WORDS`, `u_int32_t` and empty
  `libm_hidden_def`/`attribute_hidden` is enough.
- `libm/optimized/e_exp.c` needs musl's helpers, which sit next to it in
  `musl-support.h`, plus `exp_data.c`.
- `libm/accurate/e_hypot.c` compiles as is; `-DUCLIBC_HYPOT_FORCE_SPLIT` selects the
  path without a 128-bit integer, which is how the 32-bit emulation was checked
  against the `__int128` one.
- CORE-MATH's binary32 files compile as they are.
- CORE-MATH's `sin`, `cos` and `tan` want a 128-bit integer, which a 64-bit host
  has; that is why the reference for them can be generated even though the
  implementation cannot be shipped on most 32-bit targets.
- `libm/s_sin.c`, `s_cos.c` and `s_tan.c` are the public entry points, so rename
  with `objcopy --redefine-sym sin=fd_sin` rather than `-Dsin=fd_sin`, which
  would mangle the declaration in the host's `math.h`.
- `libm/e_lgamma_r.c` also wants `k_sin.c` and `k_cos.c` beside it, for the
  sin_pi it uses on negative arguments.
- The accurate files call fma, so link one in: on a host whose libm has a
  conforming fma that is automatic, elsewhere take libm/s_fma.c.

Generate on a 64-bit host.  On a 32-bit x86 host the x87 computes with 64
mantissa bits and the counts come out wrong -- and not in one direction:
`-fexcess-precision=standard` is not enough to stop it, since it says where the
compiler rounds, not how wide the hardware computes.  What it takes is the FPU's
own precision control field, which is what `libm/x87-precision.h` sets for the
functions that need it.

Run without arguments to see how often each variant misses the reference — those are
the numbers the budgets are set from. Run with any argument to print the tables; the
comment at the top of each header is written by hand and has to be prepended again.

Whoever regenerates a table should say in the header comment what the point set is
and why, the way the current ones do: for `hypot` the double pairs are deliberately
biased towards hard cases, or the variants cannot be told apart at all. The float
tables need no such bias — every variant hits every float point, because computing in
`double` leaves the error far below the `float` rounding boundary.

## The three that do not use CORE-MATH

`gen-bessel-ref.c`, `gen-sqrt-ref.c` and `gen-fma-ref.c` take their reference
from **MPFR** (`mpfr_j0` and siblings, `mpfr_sqrt`, `mpfr_fma`), so regenerating
those three tables needs `libmpfr-dev` on the host -- `-lmpfr -lgmp`.  Building
or running the tests does not.

For Bessel there is no alternative: CORE-MATH has no Bessel functions and glibc
and musl both ship the same fdlibm code we do, so there was nothing to compare
against.  For sqrt and fma there is no need of one: IEEE 754 requires both
correctly rounded, so the reference is not a choice and the budget is zero.
