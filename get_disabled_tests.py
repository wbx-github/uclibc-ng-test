#!/usr/bin/python3
#
# What of the test suite does not run against a given C library.
#
#   CC=<compiler> CFLAGS=<flags> LDFLAGS=<flags> TARGET_ARCH=<arch> \
#       get_disabled_tests.py
#
# test/Makefile calls it with its own CC, CFLAGS, LDFLAGS and TARGET_ARCH;
# it prints make variables for test/Makefile and test/Test.mak:
#
#   NO_TLS=1 NO_DL=1 ...                  whole directories (test/Makefile)
#   DISABLED_TESTS=nptl/tst-foo,misc/bar  single tests, dir/name, comma
#                                         separated so that it stays one
#                                         word (test/Test.mak)
#
# This is the one place that decides it.  It asks the compiler rather than
# reading a .config: the predefined macros after <features.h> say which C
# library the tests are built against, and for uClibc-ng
# bits/uClibc_config.h carries every option as __<OPTION>__.  So the answer
# is about the library that is actually linked, and the same call works for
# glibc and musl.  A test that can find out for itself whether it applies
# (features.h, sysconf, a probe) skips itself instead; what is here cannot.

import os
import shlex
import subprocess
import sys

if len(sys.argv) > 1:
    sys.exit("get_disabled_tests.py takes no arguments; set CC and CFLAGS")

def macros():
    cc = shlex.split(os.environ.get("CC", "gcc"))
    flags = shlex.split(os.environ.get("CFLAGS", ""))
    # <sys/types.h> for musl, which has no macro of its own; its
    # bits/alltypes.h leaves __DEFINED_size_t behind
    src = "#include <features.h>\n#include <sys/types.h>\n"
    r = subprocess.run(cc + flags + ["-dM", "-E", "-x", "c", "-"],
                       input=src, capture_output=True, text=True)
    if r.returncode != 0:
        sys.stderr.write(r.stderr)
        sys.exit("get_disabled_tests.py: cannot run " + " ".join(cc))
    m = {}
    for l in r.stdout.splitlines():
        f = l.split(None, 2)
        if len(f) >= 2 and f[0] == "#define":
            m[f[1]] = f[2] if len(f) > 2 else ""
    return m

def links_pie():
    """Whether toolchain and libc can link a PIE: a PIC Scrt1.o and a linker
    that produces ET_DYN executables."""
    cc = shlex.split(os.environ.get("CC", "gcc"))
    flags = (shlex.split(os.environ.get("CFLAGS", ""))
             + shlex.split(os.environ.get("LDFLAGS", "")))
    r = subprocess.run(cc + flags + ["-fPIE", "-pie", "-x", "c", "-",
                                     "-o", os.devnull],
                       input="int main(void){return 0;}\n",
                       capture_output=True, text=True)
    return r.returncode == 0

defined = macros()

# uClibc-ng defines __GLIBC__ too, so it is asked first
if "__UCLIBC__" in defined:
    libc = "uclibc"
elif "__GLIBC__" in defined:
    libc = "glibc"
elif "__DEFINED_size_t" in defined:
    libc = "musl"
else:
    libc = "unknown"

uclibc = libc == "uclibc"

def is_set(option):
    """A uClibc-ng option, as bits/uClibc_config.h has it."""
    return "__" + option + "__" in defined

def arch(name):
    """TARGET_ARCH as test/Rules.mak derives it from $(CC) -dumpmachine."""
    return os.environ.get("TARGET_ARCH", "") == name

no = set()
tests = []

def disable(*names):
    tests.extend(names)


# --- whole directories ----------------------------------------------------

# For uClibc-ng the options say it.  glibc and musl always have locales,
# wide characters, threads and TLS; whether a static-only build has dlopen
# is not visible here, a caller sets NO_DL=1 for that.
if uclibc:
    if not is_set("UCLIBC_HAS_LOCALE"):
        no.add("NO_LOCALE")
    if not is_set("HAVE_SHARED") or is_set("ARCH_HAS_NO_SHARED"):
        no.add("NO_DL")
    if not is_set("UCLIBC_HAS_WCHAR"):
        no.add("NO_WCHAR")

    if is_set("HAS_NO_THREADS"):
        no.update(["NO_THREADS", "NO_NPTL", "NO_TLS"])
    if is_set("UCLIBC_HAS_LINUXTHREADS"):
        no.update(["NO_NPTL", "NO_TLS"])

# The *-pie variants, wherever a PIE cannot be linked (microblaze: non-PIC
# Scrt1.o plus a BFD assertion on the attempt).
if not links_pie():
    no.add("NO_PIE")

# Both from openadk's package/uclibc-ng-test/Makefile, which sets them
# without a reason; taken over so that it need not set them itself.
if libc == "glibc":
    no.add("NO_CRYPT")
if libc == "musl":
    no.add("NO_MATH")


# --- single tests -----------------------------------------------------------

# A libc without shared support cannot link the modules these tests load.
# What is left of tls/, tst-tls1 and tst-tls2, is what a static program can
# do with TLS at all.
if "NO_DL" in no:
    disable("nptl/tst-tls3", "nptl/tst-tls4", "nptl/tst-atfork2",
            "nptl/tst-cancelx4-pie")
    disable(*["tls/" + t for t in (
        "tst-tls3", "tst-tls4", "tst-tls5", "tst-tls6", "tst-tls7", "tst-tls8",
        "tst-tls9", "tst-tls10", "tst-tls11", "tst-tls12", "tst-tls13",
        "tst-tls14", "tst-tls15", "tst-tls17", "tst-tls18", "tst-tls-at-ctor",
        "tst-tls1-pie", "tst-tls2-pie", "tst-tls3-pie")])

# Moved here from the Makefile.in of each directory.  Those that asked a
# uClibc-ng option never fired as meant: the test suite does not read the
# .config, so the variable was always empty.  Here the option is known.
# Most such tests ask features.h themselves and report SKIP; these cannot:
# getrpcent_r and arc4random no longer exist in uClibc-ng at all.
if uclibc:
    if not is_set("UCLIBC_HAS_REENTRANT_RPC"):
        disable("rpc/getrpcent_r")
    if not is_set("UCLIBC_HAS_ARC4RANDOM"):
        disable("stdlib/testarc4random")
    if not is_set("UCLIBC_HAS_GLIBC_CUSTOM_STREAMS"):
        disable("stdio/tst-fmemopen")

# Off everywhere, word for word from the Makefile.in, each with the reason
# given there, if any.
disable("locale/tst-ctype", "locale/tst-fmon", "locale/tst-leaks",
        "locale/tst-rpmatch", "locale/tst-strfmon1")   # known not to build
disable("locale/bug-usesetlocale", "locale/tst-xlocale1",
        "locale/tst-xlocale2", "locale/xfrm-test",
        "locale/tst-C-locale")                     # need UCLIBC_HAS_XLOCALE
disable("locale-mbwc/tst_strfmon")                 # build failure
# gamma: removed from TESTS, needs "small errors are ok" machinery
disable("math/gamma")
# long double: test-ldouble and test-ildoubl fail where libm wraps double;
# compile_test and c99_test went with them.  Behind an always-empty
# UCLIBC_HAS_LONG_DOUBLE_MATH before, so off everywhere.
disable("math/test-ldouble", "math/test-ildoubl", "math/compile_test",
        "math/c99_test")
disable("misc/outb", "misc/tst-fnmatch", "misc/bug-glob1",
        "misc/tst-gnuglob")
disable("malloc/time_malloc")
disable("pthread/cancellation-points")
# testregex: slows embedded test runs down extremely, some tests fail, and
# gcc 5.3.0 ICEs on it for lm32
disable("regex/testregex")
disable("rt/tst-shm")
disable("time/bug-asctime", "time/bug-asctime_r", "time/time",
        "time/tst-mktime2", "time/tst-posixtz", "time/tst-strftime",
        "time/tst-strptime", "time/tst-timezone")
# tst-tls16: always fails, guessing some unimplemented check or feature
disable("tls/tst-tls1-static", "tls/tst-tls2-static", "tls/tst-tls9-static",
        "tls/tst-tls16")
# fallocate64: 4 GB of disk space are not available on embedded systems
disable("unistd/tst-posix_fallocate64", "unistd/tst-fallocate64")

# Per arch, word for word from the Makefile.in; none came with a reason
# beyond the one noted.
if arch("avr32"):
    disable("misc/tst-inotify", "time/tst-timerfd", "unistd/tst-fallocate",
            "unistd/tst-posix_fallocate", "unistd/tst-fallocate64")
if arch("bfin") or arch("m68k"):
    disable("misc/tst-rlimit", "misc/tst-rlimit64")
if arch("arm") or arch("riscv32") or arch("xtensa"):
    disable("mmap/mmap2")
if arch("metag"):
    disable("nptl/tst-cleanup0", "nptl/tst-cleanupx0")
if arch("riscv32"):
    disable("nptl/tst-rwlock9")
if arch("riscv64"):
    disable("inet/tst-ifaddrs")
# nptl on sh: "deadlock", the one word the Makefile.in gave.
if arch("sh"):
    disable(*["nptl/" + t for t in (
        "tst-cond1", "tst-cond4", "tst-cond6", "tst-cond7", "tst-cond10",
        "tst-cond13", "tst-cond14", "tst-cond15", "tst-cond18", "tst-cond20",
        "tst-cond21", "tst-cond22", "tst-cputimer1", "tst-cputimer2",
        "tst-cputimer3", "tst-mutex8", "tst-timer4", "tst-timer5",
        "tst-spin3")])
    disable("pthread/ex2", "pthread/ex7")
if arch("microblaze") or arch("nds32"):
    disable("pthread/ex3")
if arch("cris"):
    disable("pthread/ex2", "pthread/ex5")
if arch("or1k"):
    disable("unistd/tst-pselect")
if arch("h8300"):
    disable("string/stratcliff")


out = [n + "=1" for n in sorted(no)]
if tests:
    out.append("DISABLED_TESTS=" + ",".join(tests))
print(" ".join(out))
