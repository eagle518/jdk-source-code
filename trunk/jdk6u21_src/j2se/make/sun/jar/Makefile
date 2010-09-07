#
# @(#)Makefile	1.43 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jar utility.
#

BUILDDIR = ../..
PACKAGE = sun.tools
PRODUCT = sun
PROGRAM = jar
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
AUTO_FILES_JAVA_DIRS = sun/tools/jar

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/tools/jar/resources/jar.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.jar.Main\" }"
include $(BUILDDIR)/common/Program.gmk
