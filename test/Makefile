# Makefile for uClibc-ng-test
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

top_srcdir=../
top_builddir=../
include Rules.mak

# What does not run against this C library.  get_disabled_tests.py asks the
# compiler and prints NO_*=1 and DISABLED_TESTS=dir/test,...; whatever is
# given on the command line or in the environment wins.  ALL_TESTS=1 runs
# everything.
PYTHON ?= python3
ifneq ($(ALL_TESTS),1)
DISABLE_VARS := $(shell CC='$(CC)' CFLAGS='$(CFLAGS)' TARGET_ARCH='$(TARGET_ARCH)' $(PYTHON) $(top_srcdir)get_disabled_tests.py)
ifneq ($(.SHELLSTATUS),0)
$(error get_disabled_tests.py failed)
endif
$(info disabled by get_disabled_tests.py: $(DISABLE_VARS))
$(foreach v,$(DISABLE_VARS),$(if $(filter undefined,$(origin $(firstword $(subst =, ,$(v))))),$(eval $(v))))
export DISABLED_TESTS
endif

ALL_SUBDIRS := $(patsubst %/Makefile,%,$(wildcard */Makefile))

DIRS := $(ALL_SUBDIRS)

ifeq ($(NO_THREADS),1)
DIRS := $(filter-out nptl,$(DIRS))
DIRS := $(filter-out pthread,$(DIRS))
DIRS := $(filter-out dlopen,$(DIRS))
endif

ifeq ($(NO_NPTL),1)
DIRS := $(filter-out nptl,$(DIRS))
endif

ifeq ($(NO_TLS),1)
DIRS := $(filter-out tls,$(DIRS))
endif

ifeq ($(NO_DL),1)
DIRS := $(filter-out dlopen,$(DIRS))
endif

ifeq ($(NO_WCHAR),1)
DIRS := $(filter-out locale-mbwc,$(DIRS))
endif

ifeq ($(NO_LOCALE),1)
DIRS := $(filter-out locale,$(DIRS))
endif

ifeq ($(NO_MATH),1)
DIRS := $(filter-out math,$(DIRS))
endif

ifeq ($(NO_CRYPT),1)
DIRS := $(filter-out crypt,$(DIRS))
endif

test check all: run

run: subdirs_run

gen:
	-rm -f $(top_builddir)/test/uclibcng-testrunner.in
	$(MAKE) run UCLIBCNG_GENERATE_TESTRUNNER=1

# The runner, built for the target with the same CC and flags as the tests.
# "make install" leaves out *.[ochmd], so the binary goes into the rootfs and
# its source does not.
uclibcng-testrunner: uclibcng-testrunner.c
	$(CC) $(CFLAGS) $(EXTRA_CFLAGS) -o $@ $< $(LDFLAGS) $(EXTRA_LDFLAGS)

compile: uclibcng-testrunner subdirs_compile

clean: subdirs_clean
	-rm -f uclibcng-testrunner

subdirs: $(patsubst %, _dir_%, $(DIRS))
subdirs_compile: $(patsubst %, _dircompile_%, $(DIRS))
subdirs_run: $(patsubst %, _dirrun_%, $(DIRS))
subdirs_clean: $(patsubst %, _dirclean_%, $(ALL_SUBDIRS))

$(patsubst %, _dir_%, $(DIRS)) : dummy
	$(Q)$(MAKE) -C $(patsubst _dir_%, %, $@) \
		KCONFIG_CONFIG=$(KCONFIG_CONFIG)

$(patsubst %, _dirrun_%, $(DIRS)) : dummy
	$(Q)$(MAKE) -C $(patsubst _dirrun_%, %, $@) run \
		UCLIBCNG_TEST_SUBDIR=$(strip $(patsubst _dirrun_%, %, $@))

$(patsubst %, _dircompile_%, $(DIRS)) : dummy
	$(Q)$(MAKE) -C $(patsubst _dircompile_%, %, $@) compile

$(patsubst %, _dirclean_%, $(ALL_SUBDIRS)) : dummy
	$(Q)$(MAKE) -C $(patsubst _dirclean_%, %, $@) clean

.PHONY: all check clean dummy subdirs_compile subdirs_run subdirs subdirs_clean test run compile
