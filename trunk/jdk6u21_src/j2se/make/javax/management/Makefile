#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of JMX
#

BUILDDIR = ../..
PACKAGE = javax.management
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = javax/management com/sun/jmx com/sun/management/jmx 

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Clean up internal-use-only package.
#
clean clobber::
	$(RM) -r $(CLASSDESTDIR)/com/sun/jmx \
                 $(CLASSDESTDIR)/com/sun/management/jmx \
                 $(CLASSDESTDIR)/org/omg/stub/javax/management/remote/rmi \

