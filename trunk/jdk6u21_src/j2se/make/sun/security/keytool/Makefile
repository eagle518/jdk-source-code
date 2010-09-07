#
# @(#)Makefile	1.14 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.14 10/03/23
#
# Makefile for building keytool utility.
#

BUILDDIR = ../../..
PACKAGE = sun.security.tools
PRODUCT = sun
PROGRAM = keytool
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.security.tools.KeyTool\" }"
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level 
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
# 
BUILD_MANIFEST=true

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk
