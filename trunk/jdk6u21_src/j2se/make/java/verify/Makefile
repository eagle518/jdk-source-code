#
# @(#)Makefile	1.25 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build libverify.so
#

BUILDDIR = ../..
PRODUCT = java
LIBRARY = verify
include $(BUILDDIR)/common/Defs.gmk

#
# Link to libjvm so JVM_ functions can be called.
#
OTHER_LDLIBS += $(JVMLIB)

ifeq ($(PLATFORM), windows)
  #
  # Don't want JAVALIB
  #
  JAVALIB = 
  EXTRA_LIBS = 
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif

#
# Files to compile.
#
FILES_c = \
	check_code.c \
	check_format.c

#
# libverify.so needs these 2 header files (opcodes.h opcodes.length) 
# from the VM.
#
CPPFLAGS += -I$(SHARE_SRC)/javavm/include 

#
# Targets.
#
ifeq ($(PLATFORM), solaris)
  ifneq ($(ARCH), amd64)
    FILES_reorder += reorder-$(ARCH)
  endif
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Add to the ambient VPATH.
#
vpath %.c   $(SHARE_SRC)/native/common

