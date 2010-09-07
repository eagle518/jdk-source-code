#
# @(#)Makefile	1.20 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


BUILDDIR = ../..
PACKAGE = sun.jdga
LIBRARY = sunwjdga
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
FILES_c = \
	dgalock.c

#
# Add use of mapfile
#
ifeq ($(ARCH_FAMILY), sparc)
FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk
endif

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(PLATFORM_SRC)/native/$(PKGDIR)

ifneq ($(PLATFORM), windows)
  LDLIBS =  -ldga -lX11 -ldl -lc

  CPPFLAGS += \
	-I$(SHARE_SRC)/javavm/export \
	-I$(PLATFORM_SRC)/javavm/export \
	-I$(OPENWIN_HOME)/include 
endif

LDFLAGS += -L$(OPENWIN_LIB) -R$(OPENWIN_LIB)
