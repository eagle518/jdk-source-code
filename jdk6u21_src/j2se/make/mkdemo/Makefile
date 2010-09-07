#
# @(#)Makefile	1.35 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all the demos
#

BUILDDIR = ..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = applets jfc jvmti management jni scripting

all build:: nbproject
	$(SUBDIRS-loop)

nbproject:
	$(RM) -r $(DEMODIR)/nbproject
	$(MKDIR) -p $(DEMODIR)
	( $(CD) $(SHARE_SRC)/demo && $(TAR) -cf - \
	  `find nbproject -name SCCS -prune -o -type f -print` ) | \
	  ( $(CD) $(DEMODIR) && $(TAR) -xf - )

clean clobber::
	$(SUBDIRS-loop)

clobber clean ::
	$(RM) -r $(DEMODIR)
