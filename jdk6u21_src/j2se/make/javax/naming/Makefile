#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of JNDI core
#

BUILDDIR = ../..
PACKAGE = javax.naming
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = javax/naming com/sun/naming/internal

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Clean up internal-use-only package.
#
clean clobber::
	$(RM) -r $(CLASSDESTDIR)/com/sun/naming

