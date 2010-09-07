#
# Copyright (c) 1999, 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#  
#

# Sets make macros for making debug version of VM

# Compiler specific DEBUG_CFLAGS are passed in from gcc.make, sparcWorks.make
DEBUG_CFLAGS/DEFAULT= $(DEBUG_CFLAGS)
DEBUG_CFLAGS/BYFILE = $(DEBUG_CFLAGS/$@)$(DEBUG_CFLAGS/DEFAULT$(DEBUG_CFLAGS/$@))
CFLAGS += $(DEBUG_CFLAGS/BYFILE)

# Linker mapfile
MAPFILE = $(GAMMADIR)/make/linux/makefiles/mapfile-vers-debug

_JUNK_ := $(shell echo -e >&2 ""\
 "----------------------------------------------------------------------\n" \
 "WARNING: 'make debug' is deprecated. It will be removed in the future.\n" \
 "Please use 'make jvmg' to build debug JVM.                            \n" \
 "----------------------------------------------------------------------\n")

G_SUFFIX = _g
VERSION = debug
SYSDEFS += -DASSERT -DDEBUG
PICFLAGS = DEFAULT
