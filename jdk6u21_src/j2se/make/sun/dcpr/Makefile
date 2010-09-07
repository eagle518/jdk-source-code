#
# @(#)Makefile	1.27 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.dc
LIBRARY = dcpr
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_c.gmk

AUTO_FILES_JAVA_DIRS = sun/dc

FILES_export = \
	sun/dc/pr/PathDasher.java \
	sun/dc/pr/PathFiller.java \
	sun/dc/pr/PathStroker.java \

#
# Rules.
#
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Add to the ambient vpath to pick up files in subdirectories
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/doe
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/path
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/pr
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/util

ifneq ($(PLATFORM), windows)
  OTHER_LDLIBS = $(LIBM)
endif # PLATFORM

CPPFLAGS += \
	-I$(SHARE_SRC)/native/$(PKGDIR)/doe \
	-I$(SHARE_SRC)/native/$(PKGDIR)/path \
	-I$(SHARE_SRC)/native/$(PKGDIR)/util \

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
