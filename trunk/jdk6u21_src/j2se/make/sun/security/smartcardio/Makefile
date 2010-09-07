#
# @(#)Makefile	1.5 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = sun.security.smartcardio
LIBRARY = j2pcsc
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# C and Java Files
#
include FILES_c.gmk

AUTO_FILES_JAVA_DIRS = sun/security/smartcardio

#
# Java files that define native methods
#
FILES_export = \
    sun/security/smartcardio/PCSC.java \
    sun/security/smartcardio/PlatformPCSC.java

#
# Find native code
#
vpath %.c \
  $(SHARE_SRC)/native/sun/security/smartcardio \
  $(PLATFORM_SRC)/native/sun/security/smartcardio

#
# Find include files
#
OTHER_INCLUDES += \
  -I$(SHARE_SRC)/native/sun/security/smartcardio \
  -I$(PLATFORM_SRC)/native/sun/security/smartcardio \
  -I$(PLATFORM_SRC)/native/sun/security/smartcardio/MUSCLE

#
# Rules
#
#CLASSDESTDIR = $(TMPDIR)/classes

#FILES_class = $(FILES_java:%.java=$(CLASSDESTDIR)/%.class)

include $(BUILDDIR)/common/Mapfile-vers.gmk

include $(BUILDDIR)/common/Library.gmk

#
# Libraries to link
#
ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS = $(JVMLIB) winscard.lib
else
  OTHER_LDLIBS = -ldl $(JVMLIB)
  OTHER_CFLAGS = -D__sun_jdk
endif
