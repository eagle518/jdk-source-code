#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the jvmti demo(s)
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

# Can be built in any order, agent_util is just shared source, hprof and 
#   java_crw_demo are really built in make/java as part of JRE and are listed
#   here just to populate the demo/jvmti directory with the sources and README
#   files.
SUBDIRS = hprof \
	  java_crw_demo \
	  agent_util \
	  versionCheck \
          gctest \
	  heapViewer \
	  heapTracker \
	  minst \
	  mtrace \
	  waiters
all build clean clobber::
	$(SUBDIRS-loop)

all build :: $(ABS_OUTPUTDIR)/demo/jvmti $(ABS_OUTPUTDIR)/demo/jvmti/index.html

$(ABS_OUTPUTDIR)/demo/jvmti/index.html: $(SHARE_SRC)/demo/jvmti/index.html
	$(install-file)

