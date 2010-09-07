#
# @(#)Makefile	1.23 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the doclet API and the javadoc tool.
#

BUILDDIR = ../../..
PRODUCT = sun
PACKAGE = com.sun.tools.javadoc
PROGRAM = javadoc
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/javadoc com/sun/tools/javadoc

#
# Resources
# 
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
	com/sun/tools/javadoc/resources/javadoc.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.javadoc.Main\" }"
include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS
