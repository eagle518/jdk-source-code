# @(#)Makefile	1.8 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building apt
#

BUILDDIR = ../../..
PRODUCT = sun
PACKAGE = com.sun.tools.apt
PROGRAM = apt
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
AUTO_FILES_JAVA_DIRS = com/sun/mirror com/sun/tools/apt 

#
# Resources (must be made before .recompile).
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
        com/sun/tools/apt/resources/apt.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.apt.Main\" }"
include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS
