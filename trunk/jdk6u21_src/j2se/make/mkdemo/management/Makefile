#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the java.lang.management demo(s)
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

# Can be built in any order
SUBDIRS = FullThreadDump \
	  VerboseGC      \
	  MemoryMonitor  \
	  JTop
all build clean clobber::
	$(SUBDIRS-loop)

all build :: $(ABS_OUTPUTDIR)/demo/management $(ABS_OUTPUTDIR)/demo/management/index.html

$(ABS_OUTPUTDIR)/demo/management/index.html: $(SHARE_SRC)/demo/management/index.html
	$(install-file)

