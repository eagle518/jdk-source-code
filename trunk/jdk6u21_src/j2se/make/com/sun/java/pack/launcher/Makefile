#
# @(#)Makefile	1.11 10/03/23 
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building Pack200 launcher
BUILDDIR = ../../../../..
PACKAGE = com.sun.java.util.jar.pack
PRODUCT = sun
PROGRAM = pack200
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
AUTO_FILES_JAVA_DIRS = com/sun/java/util/jar/pack

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms64m\", \"com.sun.java.util.jar.pack.Driver\", \"--pack\" }"
include $(BUILDDIR)/common/Program.gmk
