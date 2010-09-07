#
# @(#)Makefile	1.29 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.misc
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Use highest optimization level
#
_OPT = $(CC_HIGHEST_OPT)



#
# Java source files
#
AUTO_FILES_JAVA_DIRS = sun/misc sun/jkernel

RESOURCE_BUNDLES_JAVA= \
	$(TARGDIR)sun/misc/resources/Messages.java

LOCALE_SUFFIXES = $(J2RE_LOCALES)



#
# Resources
#
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk
