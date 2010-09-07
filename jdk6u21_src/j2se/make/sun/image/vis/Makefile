#
# @(#)Makefile	1.22 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the VIS (solaris-only) version of medialib
#
BUILDDIR = ../../..
PACKAGE = sun.awt.medialib
LIBRARY = mlib_image_v
PRODUCT = sun

#
# Tell Defs.gmk we need VIS instructions
#
VIS_NEEDED=true

include $(BUILDDIR)/common/Defs.gmk

#
# Select highest level of optimization for this library
#
_OPT = $(CC_HIGHEST_OPT)

#
# Use generic mapfile
#
FILES_m = ../generic/mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files
#
include FILES_c.gmk

#
# Rules
#
include $(BUILDDIR)/common/Library.gmk

vpath %.s	$(PLATFORM_SRC)/native/sun/awt/medialib
vpath %.c	$(SHARE_SRC)/native/sun/awt/medialib

#
# set compile options for solaris 32- or 64-bit build
#
ASFLAGS += -P
CFLAGS_sparcv9 = -DMLIB_OS64BIT
INLINE = $(PLATFORM_SRC)/native/sun/awt/medialib/vis_$(ARCH_DATA_MODEL).il

CFLAGS += $(CFLAGS_$(ARCH)) $(INLINE) -I$(PLATFORM_SRC)/native/sun/awt/medialib -I$(SHARE_SRC)/native/sun/awt/medialib

OTHER_CFLAGS += -D__USE_J2D_NAMES -D__MEDIALIB_OLD_NAMES
OTHER_LDLIBS = $(LIBM) -ldl

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
