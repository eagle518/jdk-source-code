#
# @(#)Makefile	1.23 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the generic version of medialib
#

BUILDDIR = ../../..
PACKAGE = sun.awt.medialib
LIBRARY = mlib_image
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Use highest level of optimization on this library
#
_OPT = $(CC_HIGHEST_OPT)

#
# Use mapfile
#
FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files
#
include FILES_c.gmk

#
# Rules
#
include $(BUILDDIR)/common/Library.gmk

#
# set compile options for solaris 32- or 64-bit build
#
ifeq ($(ARCH_DATA_MODEL), 64)
  CFLAGS += -DMLIB_OS64BIT
endif # ARCH_DATA_MODEL

ifneq ($(PLATFORM), sparc)
      OTHER_CFLAGS += -DMLIB_NO_LIBSUNMATH
endif

CPPFLAGS += \
            -I$(PLATFORM_SRC)/native/$(PKGDIR)/medialib 

OTHER_CFLAGS += -D__USE_J2D_NAMES -D__MEDIALIB_OLD_NAMES
OTHER_LDLIBS = $(LIBM) -ldl

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
