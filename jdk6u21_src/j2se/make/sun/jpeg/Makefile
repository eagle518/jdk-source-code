#
# @(#)Makefile	1.36 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.awt
LIBRARY = jpeg
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Use highest optimization level
#
_OPT = $(CC_HIGHEST_OPT)

#
# Files
#
include FILES_c.gmk

AUTO_FILES_JAVA_DIRS = sun/awt/image com/sun/imageio/plugins/jpeg

FILES_export = \
	sun/awt/image/JPEGImageDecoder.java \
	sun/awt/image/codec/JPEGImageDecoderImpl.java \
	sun/awt/image/codec/JPEGImageEncoderImpl.java \
	com/sun/imageio/plugins/jpeg/JPEGImageReader.java \
	com/sun/imageio/plugins/jpeg/JPEGImageWriter.java

#
# Rules
#
ifeq ($(PLATFORM), solaris)
  ifneq ($(ARCH), amd64)
    FILES_reorder += reorder-$(ARCH)
  endif
endif
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# Add to ambient vpath to get files in a subdirectory
#
vpath %.c   $(SHARE_SRC)/native/$(PKGDIR)/image/jpeg

#
# C Flags
#
CFLAGS += -DNIFTY

ifeq ($(PLATFORM), windows)
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif # PLATFORM

CLASSES.export += java.io.InputStream

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni
