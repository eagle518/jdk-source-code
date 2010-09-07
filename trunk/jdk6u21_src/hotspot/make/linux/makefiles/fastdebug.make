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

# Compiler specific OPT_CFLAGS are passed in from gcc.make, sparcWorks.make
OPT_CFLAGS/DEFAULT= $(OPT_CFLAGS)
OPT_CFLAGS/BYFILE = $(OPT_CFLAGS/$@)$(OPT_CFLAGS/DEFAULT$(OPT_CFLAGS/$@))

# (OPT_CFLAGS/SLOWER is also available, to alter compilation of buggy files)

ifeq ($(BUILDARCH), ia64)
  # Bug in GCC, causes hang.  -O1 will override the -O3 specified earlier
  OPT_CFLAGS/callGenerator.o += -O1
  OPT_CFLAGS/ciTypeFlow.o += -O1
  OPT_CFLAGS/compile.o += -O1
  OPT_CFLAGS/concurrentMarkSweepGeneration.o += -O1
  OPT_CFLAGS/doCall.o += -O1
  OPT_CFLAGS/generateOopMap.o += -O1
  OPT_CFLAGS/generateOptoStub.o += -O1
  OPT_CFLAGS/graphKit.o += -O1
  OPT_CFLAGS/instanceKlass.o += -O1
  OPT_CFLAGS/interpreterRT_ia64.o += -O1
  OPT_CFLAGS/output.o += -O1
  OPT_CFLAGS/parse1.o += -O1
  OPT_CFLAGS/runtime.o += -O1
  OPT_CFLAGS/synchronizer.o += -O1
endif


# If you set HOTSPARC_GENERIC=yes, you disable all OPT_CFLAGS settings
CFLAGS$(HOTSPARC_GENERIC) += $(OPT_CFLAGS/BYFILE)

# Set the environment variable HOTSPARC_GENERIC to "true"
# to inhibit the effect of the previous line on CFLAGS.

# Linker mapfile
MAPFILE = $(GAMMADIR)/make/linux/makefiles/mapfile-vers-debug

G_SUFFIX = _g
VERSION = optimized
SYSDEFS += -DASSERT -DFASTDEBUG
PICFLAGS = DEFAULT
