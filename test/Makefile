# Makefile for uClibc-ng-test
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

top_srcdir=../
top_builddir=../
include Rules.mak

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

test check all: run

run: subdirs_run

gen:
	-rm -f $(top_builddir)/test/uclibcng-testrunner.in
	$(MAKE) run UCLIBCNG_GENERATE_TESTRUNNER=1

compile: subdirs_compile

clean: subdirs_clean

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
