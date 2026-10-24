# Common makefile rules for tests
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

shellescape='$(subst ','\'',$(1))'

ifeq ($(TESTS),)
TESTS := $(patsubst %.c,%,$(wildcard *.c))
endif
# DISABLED_TESTS comes from get_disabled_tests.py via test/Makefile: dir/test
# pairs separated by commas.  Each directory takes its own.
comma := ,
TESTS_DISABLED += $(patsubst $(notdir $(CURDIR))/%,%,$(filter $(notdir $(CURDIR))/%,$(subst $(comma), ,$(DISABLED_TESTS))))
ifneq ($(strip $(TESTS_DISABLED)),)
TESTS := $(filter-out $(TESTS_DISABLED),$(TESTS))
endif

ifneq ($(filter-out test,$(strip $(TESTS))),$(strip $(TESTS)))
$(error Sanity check: cannot have a test named "test.c")
endif

# NO_PIE comes from get_disabled_tests.py, where a PIE cannot be linked.
ifeq ($(NO_PIE),1)
TESTS := $(filter-out %-pie,$(TESTS))
endif

# gcc-torture-style: build every test a second time at -O2.  The suite
# default is no -O flag, so each test runs once at -O0 and once at -O2 --
# different optimization levels exercise different codegen in the tests
# (inline asm, gcc builtins/header inlines, unwind tables; see the
# tls-macros-i386.h ebx clobber, invisible at -O0).  Directories opt in
# with O2_TWINS := 1; individual tests opt out via TESTS_NO_O2.
ifeq ($(O2_TWINS),1)
O2_TWIN_TARGETS := $(addsuffix -O2,$(filter-out %_glibc $(TESTS_NO_O2),$(TESTS)))
endif

TARGETS := $(TESTS)

CLEAN_TARGETS := $(TARGETS) $(O2_TWIN_TARGETS)
CLEAN_TARGETS += $(TESTS_DISABLED)
COMPILE_TARGETS :=  $(TARGETS) $(O2_TWIN_TARGETS)
# We sort the targets so uClibc and host-libc tests are run adjacent
RUN_TARGETS := $(sort $(addsuffix .exe,$(TARGETS) $(O2_TWIN_TARGETS)))
COMPILE_TARGETS :=  $(sort $(COMPILE_TARGETS))
# provide build rules even for disabled tests:
TARGETS += $(TESTS_DISABLED)
CFLAGS += $(CFLAGS_$(notdir $(CURDIR)))
ifeq (1,$(UCLIBCNG_GENERATE_TESTRUNNER))
UCLIBCNG_TEST_SUBDIR ?= $(patsubst $(realpath $(TESTDIR))/%,%,$(CURDIR))
endif

define binary_name
$(patsubst %.exe,%,$@)
endef
define tst_src_name
$(patsubst %_glibc,%,$(patsubst %-O2,%,$(binary_name)))
endef

define diff_test
	$(Q)\
	for x in "$(binary_name).out" "$(tst_src_name).out" ; do \
		test -e "$$x.good" && $(do_showdiff) "$(binary_name).out" "$$x.good" && exec diff -u "$(binary_name).out" "$$x.good" ; \
	done ; \
	true
endef
define uclibc_glibc_diff_test
	$(Q)\
	test -z "$(DODIFF_$(tst_src_name))" && exec true ; \
	uclibc_out="$(binary_name).out" ; \
	glibc_out="$(tst_src_name).out" ; \
	$(do_showdiff) $$uclibc_out $$glibc_out ; \
	exec diff -u "$$uclibc_out" "$$glibc_out"
endef
define exec_test
	$(showtest)
	$(Q)\
	$(SIMULATOR) $(WRAPPER) $(WRAPPER_$(tst_src_name)) \
	./$(binary_name) $(OPTS) $(OPTS_$(tst_src_name)) > "$(binary_name).out" 2>&1 ; \
		ret=$$? ; \
		expected_ret="$(RET_$(tst_src_name))" ; \
		test -z "$$expected_ret" && export expected_ret=0 ; \
	if ! test $$ret -eq $$expected_ret ; then \
		echo "ret == $$ret ; expected_ret == $$expected_ret" ; \
		echo "The output of the failed test is:"; \
		cat "$(binary_name).out"; \
		exit 1 ; \
	fi
	$(SCAT) "$(binary_name).out"
endef

test check all: run
run: $(RUN_TARGETS)

$(addsuffix .exe,$(TARGETS)): SIMULATOR:=$(SIMULATOR)
$(RUN_TARGETS):
ifeq (1,$(UCLIBCNG_GENERATE_TESTRUNNER))
	$(Q)\
	expected_ret="$(RET_$(tst_src_name))"; echo \
	    "$${expected_ret:-0}" \
	    $(call shellescape,$(tst_src_name)) \
	    $(call shellescape,$(binary_name)) \
	    $(call shellescape,$(UCLIBCNG_TEST_SUBDIR)) \
	    $(call shellescape,$(WRAPPER) $(WRAPPER_$(tst_src_name)) ./$(binary_name) $(OPTS) $(OPTS_$(tst_src_name))) \
	    >>$(top_builddir)/test/uclibcng-testrunner.in
else
	$(exec_test)
	$(diff_test)
endif

compile: $(COMPILE_TARGETS)

TARGET_SRCS := $(addsuffix .c,$(TARGETS))

MAKE_SRCS := $(wildcard Makefile.in) $(TESTDIR)Makefile $(TESTDIR)Rules.mak $(TESTDIR)Test.mak

# Rebuild a test when the C library it links against changes; otherwise an
# incremental build keeps a stale (statically linked) copy of libc.
SYSROOT  := $(patsubst --sysroot=%,%,$(firstword $(filter --sysroot=%,$(CFLAGS) $(LDFLAGS))))
LIBC_DEP := $(wildcard $(addsuffix /usr/lib/libc.a,$(SYSROOT)))

$(TARGETS): $(TARGET_SRCS) $(MAKE_SRCS) $(LIBC_DEP)
	$(showlink)
	$(Q)$(CC) $(filter-out $(CFLAGS-OMIT-$@),$(CFLAGS)) $(EXTRA_CFLAGS) $(CFLAGS_$(notdir $(CURDIR))) $(CFLAGS_$@) -MD -MP -MT $@ -MF $@.d -c $@.c -o $@.o
	$(Q)$(CC) $(filter-out $(LDFLAGS-OMIT-$@),$(LDFLAGS)) $@.o -o $@ $(EXTRA_LDFLAGS) $(LDFLAGS_$@)

# -O2 twins: same source, same per-test flags (looked up via the base
# name $*), -O2 inserted before them so an explicit per-test -O wins.
# Depending on the base test inherits its extra prerequisites (helper
# modules etc.) without duplicating them here.
$(O2_TWIN_TARGETS): %-O2: %.c % $(MAKE_SRCS) $(LIBC_DEP)
	$(showlink)
	$(Q)$(CC) $(filter-out $(CFLAGS-OMIT-$*),$(CFLAGS)) $(EXTRA_CFLAGS) -O2 $(CFLAGS_$(notdir $(CURDIR))) $(CFLAGS_$*) -MD -MP -MT $@ -MF $@.d -c $*.c -o $@.o
	$(Q)$(CC) $(filter-out $(LDFLAGS-OMIT-$*),$(LDFLAGS)) $@.o -o $@ $(EXTRA_LDFLAGS) $(LDFLAGS_$*)

%.so: %.c
	$(showlink)
	$(Q)$(CC) \
		$(filter-out $(CFLAGS-OMIT-$<),$(CFLAGS)) $(EXTRA_CFLAGS) \
		$(CFLAGS_$(patsubst %_glibc,%,$@)) \
		-fPIC -shared $< -o $@ -Wl,-soname,$@ \
		$(filter-out $(LDFLAGS-OMIT-$<),$(LDFLAGS)) $(EXTRA_LIBS) \
		$(LDFLAGS_$(patsubst %_glibc,%,$@))

clean:
	$(showclean)
	$(Q)$(RM) *.a *.o *.d *.so *~ core *.out *.gdb $(CLEAN_TARGETS) $(EXTRA_CLEAN)
	$(Q)$(RM_R) $(EXTRA_DIRS)

# Auto-generated header dependencies (see -MD above).
-include $(wildcard *.d)

.PHONY: all check clean test run compile
