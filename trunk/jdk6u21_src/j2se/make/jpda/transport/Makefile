#
# @(#)Makefile	1.19 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of the transports
#

BUILDDIR = ../..
PRODUCT = jpda
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = socket
ifeq ($(PLATFORM), windows)
SUBDIRS += shmem
endif # PLATFORM
all build clean clobber::
	$(SUBDIRS-loop)

#
# Copy jdwpTransport.h into the SDK include directory
#
FILES_h = $(INCLUDEDIR)/jdwpTransport.h

$(INCLUDEDIR)/%.h: $(SHARE_SRC)/javavm/export/%.h
	$(install-file)

all:: $(FILES_h)

clean clobber::
	$(RM) $(FILES_h)

