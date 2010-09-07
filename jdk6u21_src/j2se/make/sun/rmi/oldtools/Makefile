#
# @(#)Makefile	1.32 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the old java compiler.
#

BUILDDIR = ../../..
PRODUCT	 = oldjavac

include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
include FILES_java.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = javac.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/sun/tools/javac/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/tools/javac/resources
include $(BUILDDIR)/common/Resources.gmk

all:	$(PFILES)

clean clobber::
	$(RM) -r $(CLASSBINDIR)/sun/tools/asm $(CLASSBINDIR)/sun/tools/java \
		 $(CLASSBINDIR)/sun/tools/javac $(CLASSBINDIR)/sun/tools/tree \
		 $(CLASSBINDIR)/sun/tools/util
