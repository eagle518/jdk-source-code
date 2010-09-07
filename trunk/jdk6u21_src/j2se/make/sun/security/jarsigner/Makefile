#
# @(#)Makefile	1.14 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.14 10/03/23
#
# Makefile for building jarsigner utility.
#

BUILDDIR = ../../..
PACKAGE = sun.security.tools
PRODUCT = sun
PROGRAM = jarsigner
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true


#
# Class to launch tool with.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.security.tools.JarSigner\" }"

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk
