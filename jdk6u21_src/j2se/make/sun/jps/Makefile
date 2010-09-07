#
# @(#)Makefile	1.6 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jps
#

BUILDDIR = ../..
PACKAGE = sun.tools.jps
PRODUCT = sun
PROGRAM = jps
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
AUTO_FILES_JAVA_DIRS = sun/tools/jps

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.jps.Jps\" }"
include $(BUILDDIR)/common/Program.gmk
