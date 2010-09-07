#
# @(#)Makefile	1.28 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for native threads HPI.
#
# Note:
# The fdlibm libraries are built using special rules in Library.gmk.
#

BUILDDIR     = ../..
LIBRARY      = fdlibm
PRODUCT      = java
include $(BUILDDIR)/common/Defs.gmk

#
# Where is fdlibm in the source tree?
#
FDLIBM_SRC = $(SHARE_SRC)/native/java/lang/fdlibm

# windows compiler flags
ifeq ($(PLATFORM),windows)
  # Turn all optimizations off
  _OPT = $(CC_NO_OPT)
  OTHER_CFLAGS =
  CPPFLAGS_DBG += -DLOGGING
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif

#
# Include path.
#
OTHER_INCLUDES = -I$(FDLIBM_SRC)/include

#
# Things that must be linked in.
#
OTHER_LDLIBS =

#
# Files to compile.
#
include FILES_c.gmk

#
# Rules for the .a file.
#
include $(BUILDDIR)/common/Library.gmk

#
# Disable optimization to get correctly reproducible 
# floating-point results.
#
ifeq ($(PLATFORM),linux)
  # Turn all optimizations off
  _OPT = $(CC_NO_OPT)
endif

#
# Find fdlibm source files.
#
vpath %.c
vpath %.c $(FDLIBM_SRC)/src
