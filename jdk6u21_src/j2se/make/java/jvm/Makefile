#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.166 10/03/23
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

#
# Make the directories of include files shipped with the JDK.
#

FILES_h = $(INCLUDEDIR)/jni.h 			\
	  $(PLATFORM_INCLUDE)/jni_md.h		\
          $(INCLUDEDIR)/jvmti.h			\
	  $(INCLUDEDIR)/classfile_constants.h

$(INCLUDEDIR)/%.h: $(SHARE_SRC)/javavm/export/%.h
	$(install-file)

$(PLATFORM_INCLUDE)/%.h: $(PLATFORM_SRC)/javavm/export/%.h
	$(install-file)

all: build

build: $(FILES_h)

clean clobber:: 
	$(RM) $(FILES_h)

