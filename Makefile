# Makefile for uClibc-ng-test
# Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.

top_srcdir=./
top_builddir=$(if $(O),$(O),.)/
export top_builddir

# We do not need built-in implicit rules
MAKEFLAGS += -r
CONFIG_SHELL ?= /bin/sh
export CONFIG_SHELL

include $(top_srcdir)Makefile.in
