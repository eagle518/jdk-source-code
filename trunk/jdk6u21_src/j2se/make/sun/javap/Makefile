#
# @(#)Makefile	1.21 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building javap
#

BUILDDIR = ../..
PACKAGE = sun.tools.javap
PRODUCT = sun
PROGRAM = javap
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files.
#
AUTO_FILES_JAVA_DIRS = sun/tools/javap

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.javap.Main\" }"
include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS
