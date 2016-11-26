# Rules.mak for uClibc-ng-test subdirs
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

.SUFFIXES:

top_builddir ?= ../
abs_top_builddir ?= $(shell cd $(top_builddir); pwd)/

TESTDIR=$(top_builddir)test/

include $(top_srcdir)Rules.mak

#--------------------------------------------------------
# Ensure consistent sort order, 'gcc -print-search-dirs' behavior, etc.
LC_ALL:= C
export LC_ALL

ifeq ($(strip $(TARGET_ARCH)),)
TARGET_ARCH:=$(shell $(CC) -dumpmachine | sed -e s'/-.*//' \
	-e 's/i.86/i386/' \
	-e 's/sun.*/sparc/' -e 's/sparc.*/sparc/' \
	-e 's/sa110/arm/' -e 's/arm.*/arm/g' \
	-e 's/m68k.*/m68k/' \
	-e 's/parisc.*/hppa/' \
	-e 's/ppc/powerpc/g' \
	-e 's/sh[234].*/sh/' \
	-e 's/microblaze.*/microblaze/' \
	-e 's/mips.*/mips/' \
	-e 's/cris.*/cris/' \
	-e 's/xtensa.*/xtensa/' \
	)
endif
export TARGET_ARCH

RM_R = $(Q)$(RM) -r
LN_S = $(Q)$(LN) -fs

XCOMMON_CFLAGS := -I$(top_builddir)test -D_GNU_SOURCE
XWARNINGS      += $(CFLAG_-Wstrict-prototypes)
CFLAGS         ?= 
CFLAGS         += $(XCOMMON_CFLAGS)
CFLAGS         += $(OPTIMIZATION) $(CPU_CFLAGS) $(XWARNINGS)

$(eval $(call check-gcc-var,-Wno-missing-field-initializers))
CFLAGS         += $(CFLAG_-Wno-missing-field-initializers)

# Can't add $(OPTIMIZATION) here, it may be target-specific.
# Just adding -Os for now.
HOST_CFLAGS    += $(XCOMMON_CFLAGS) -Os $(XWARNINGS) -std=gnu99

#LDFLAGS := $(CPU_LDFLAGS-y) -Wl,-z,now
#LDFLAGS += -Wl,-rpath,$(shell pwd)

ifneq ($(strip $(UCLIBC_EXTRA_CFLAGS)),"")
CFLAGS += $(call qstrip,$(UCLIBC_EXTRA_CFLAGS))
endif
ifneq ($(strip $(UCLIBC_EXTRA_LDFLAGS)),"")
LDFLAGS += $(call qstrip,$(UCLIBC_EXTRA_LDFLAGS))
endif

ifneq ($(findstring -s,$(MAKEFLAGS)),)
DISP := sil
Q    := @
SCAT := -@true
else
ifneq ($(V)$(VERBOSE),)
DISP := ver
Q    :=
SCAT := cat
else
DISP := pur
Q    := @
SCAT := -@true
endif
endif
ifneq ($(Q),)
MAKEFLAGS += --no-print-directory
endif

banner := ---------------------------------
pur_showclean = echo "  "CLEAN $(notdir $(CURDIR))
pur_showdiff  = echo "  "TEST_DIFF $(notdir $(CURDIR))/
pur_showlink  = echo "  "TEST_LINK $(notdir $(CURDIR))/ $@
pur_showtest  = echo "  "TEST_EXEC $(notdir $(CURDIR))/ $(@:.exe=)
sil_showclean =
sil_showdiff  = true
sil_showlink  = true
sil_showtest  = true
ver_showclean =
ver_showdiff  = true echo
ver_showlink  = true echo
ver_showtest  = printf "\n$(banner)\nTEST $(notdir $(CURDIR))/ $(@:.exe=)\n$(banner)\n"
do_showclean  = $($(DISP)_showclean)
do_showdiff   = $($(DISP)_showdiff)
do_showlink   = $($(DISP)_showlink)
do_showtest   = $($(DISP)_showtest)
showclean = @$(do_showclean)
showdiff  = @$(do_showdiff)
showlink  = @$(do_showlink)
showtest  = @$(do_showtest)
