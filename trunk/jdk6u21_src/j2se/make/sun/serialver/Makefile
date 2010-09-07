#
# @(#)Makefile	1.33 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building sun.tools.serialver.*
#

BUILDDIR = ../..
PACKAGE = sun.tools.serialver
PRODUCT = sun
PROGRAM = serialver
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files to compile.
#
AUTO_FILES_JAVA_DIRS = sun/tools/serialver

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.serialver.SerialVer\" }"
include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = serialver.properties 
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/sun/tools/serialver
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/tools/serialver/resources
include $(BUILDDIR)/common/Resources.gmk

