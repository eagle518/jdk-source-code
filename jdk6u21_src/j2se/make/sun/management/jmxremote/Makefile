#
# @(#)Makefile	1.7 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JMX agent (RMI connector bootstrap)
# for J2SE out of the box management.
#

BUILDDIR = ../../..
PACKAGE = sun.management.jmxremote
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = sun/management/jmxremote

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Clean up internal-use-only package.
#
clean::
	$(RM) -r $(CLASSDESTDIR)/sun/management/jmxremote
