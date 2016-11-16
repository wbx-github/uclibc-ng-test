# Rules.mak for uClibc
#
# Copyright (C) 2000-2008 Erik Andersen <andersen@uclibc.org>
#
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
#

# make nano-doc
# FOO = bar  -- recursively expanded variable. Value is remebered verbatim.
#               If it contains references to other variables, these references
#               are expanded whenever this variable is _substituted_.
# FOO := bar -- simply expanded variable. Right hand is expanded when
#               the variable is _defined_. Therefore faster than =.
# FOO ?= bar -- set a value only if it is not already set
#               (behaves as =, not :=).
# FOO += bar -- append; if FOO is not defined, acts like = (not :=).


# check for proper make version
ifneq ($(findstring x3.7,x$(MAKE_VERSION)),)
$(error Your make is too old $(MAKE_VERSION). Go get at least 3.80)
endif

#-----------------------------------------------------------
# This file contains rules which are shared between multiple
# Makefiles.  All normal configuration options live in the
# file named ".config".  Don't mess with this file unless
# you know what you are doing.

clean_targets := clean realclean distclean \
	objclean-y headers_clean-y CLEAN_utils
noconfig_targets := menuconfig config nconfig \
	oldaskconfig silentoldconfig oldconfig allnoconfig allyesconfig \
	alldefconfig randconfig defconfig savedefconfig listnewconfig \
	olddefconfig \
	xconfig gconfig update-po-config mconf qconf gconf nconf conf \
	release dist tags help


#-----------------------------------------------------------
# If you are running a cross compiler, you will want to set
# 'CROSS_COMPILE' to something more interesting ...  Target
# architecture is determined by asking the CC compiler what
# arch it compiles things for, so unless your compiler is
# broken, you should not need to specify TARGET_ARCH.
#
# Most people will set this stuff on the command line, i.e.
#        make CROSS_COMPILE=arm-linux-
# will build uClibc for 'arm'.
# CROSS is still supported for backward compatibily only

CROSS_COMPILE ?= $(CROSS)

CC         = $(CROSS_COMPILE)gcc
AR         = $(CROSS_COMPILE)ar
LD         = $(CROSS_COMPILE)ld
NM         = $(CROSS_COMPILE)nm
OBJDUMP    = $(CROSS_COMPILE)objdump
STRIPTOOL  = $(CROSS_COMPILE)strip

INSTALL    = install
LN         = ln
RM         = rm -f
TAR        = tar
SED        = sed
AWK        = awk

DESTDIR    ?=

STRIP_FLAGS ?= -x -R .note -R .comment

# Select the compiler needed to build binaries for your development system
HOSTCC     = gcc
BUILD_CFLAGS = -Os

#---------------------------------------------------------
# Nothing beyond this point should ever be touched by mere
# mortals.  Unless you hang out with the gods, you should
# probably leave all this stuff alone.

# strip quotes
qstrip = $(strip $(subst ",,$(1)))
#"))

TARGET_ARCH:=$(call qstrip,$(TARGET_ARCH))
ifeq ($(TARGET_ARCH),)
ARCH ?= $(shell uname -m | $(SED) -e s/i.86/i386/ \
				  -e s/sun.*/sparc/ -e s/sparc.*/sparc/ \
				  -e s/arm.*/arm/ -e s/sa110/arm/ \
				  -e s/nds32.*/nds32/ \
				  -e s/sh.*/sh/ \
				  -e s/s390x/s390/ -e s/parisc.*/hppa/ \
				  -e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
				  -e s/xtensa.*/xtensa/ )
else
ARCH = $(TARGET_ARCH)
endif
export ARCH

# Make certain these contain a final "/", but no "//"s.
scrub_path = $(strip $(subst //,/, $(subst ,/, $(call qstrip,$(1)))))
TARGET_SUBARCH := $(call qstrip,$(TARGET_SUBARCH))
RUNTIME_PREFIX := $(call scrub_path,$(RUNTIME_PREFIX))
DEVEL_PREFIX   := $(call scrub_path,$(DEVEL_PREFIX))
MULTILIB_DIR   := $(call scrub_path,$(MULTILIB_DIR))
KERNEL_HEADERS := $(call scrub_path,$(KERNEL_HEADERS))
export RUNTIME_PREFIX DEVEL_PREFIX KERNEL_HEADERS MULTILIB_DIR

# Make sure DESTDIR and PREFIX can be used to install
# PREFIX is a uClibcism while DESTDIR is a common GNUism
ifndef PREFIX
PREFIX = $(DESTDIR)
endif

comma:=,
space:= #

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE=$(call qstrip,$(CROSS_COMPILER_PREFIX))
endif

# A nifty macro to make testing gcc features easier
check_gcc=$(shell \
	if $(CC) $(1) -S -o /dev/null -xc /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; else echo "$(2)"; fi)
check_as=$(shell \
	if $(CC) -Wa,$(1) -Wa,-Z -c -o /dev/null -xassembler /dev/null > /dev/null 2>&1; \
	then echo "-Wa,$(1)"; fi)
check_ld=$(shell \
	if $(CC) $(LDFLAG-fuse-ld) $(CFLAG_-Wl--no-warn-mismatch) -Wl,$(1) $(CFLAG_-nostdlib) -o /dev/null -Wl,-b,binary /dev/null > /dev/null 2>&1; \
	then echo "$(1)"; fi)

# Use variable indirection here so that we can have variable
# names with fun chars in them like equal signs
define check-tool-var
ifeq ($(filter $(clean_targets) CLEAN_%,$(MAKECMDGOALS)),)
_v = $(2)_$(3)
ifndef $$(_v)
$$(_v) := $$(call $(1),$(subst %, ,$(3)))
export $$(_v)
endif
endif
endef

# Usage: check-gcc-var,<flag>
# Check the C compiler to see if it supports <flag>.
# Export the variable CFLAG_<flag> if it does.
define check-gcc-var
$(call check-tool-var,check_gcc,CFLAG,$(1))
endef
# Usage: check-as-var,<flag>
# Check the assembler to see if it supports <flag>.  Export the
# variable ASFLAG_<flag> if it does (for invoking the assembler),
# as well CFLAG_-Wa<flag> (for invoking the compiler driver).
define check-as-var
$(call check-tool-var,check_as,ASFLAG,$(1))
_v = CFLAG_-Wa$(1)
export $$(_v) = $$(if $$(ASFLAG_$(1)),-Wa$$(comma)$$(ASFLAG_$(1)))
endef
# Usage: check-ld-var,<flag>
# Check the linker to see if it supports <flag>.  Export the
# variable LDFLAG_<flag> if it does (for invoking the linker),
# as well CFLAG_-Wl<flag> (for invoking the compiler driver).
define check-ld-var
$(call check-tool-var,check_ld,LDFLAG,$(1))
_v = CFLAG_-Wl$(1)
export $$(_v) = $$(if $$(LDFLAG_$(1)),-Wl$$(comma)$$(LDFLAG_$(1)))
endef
# Usage: cache-output-var,<variable>,<shell command>
# Execute <shell command> and cache the output in <variable>.
define cache-output-var
ifndef $(1)
$(1) := $$(shell $(2))
export $(1)
endif
endef


ARFLAGS:=cr

ifneq ($(strip $(UCLIBC_EXTRA_CFLAGS)),"")
CFLAGS += $(call qstrip,$(UCLIBC_EXTRA_CFLAGS))
endif
ifneq ($(strip $(UCLIBC_EXTRA_LDFLAGS)),"")
LDFLAGS += $(call qstrip,$(UCLIBC_EXTRA_LDFLAGS))
endif
