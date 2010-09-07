#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.20 10/03/23
#
# Makefile for building policytool utility.
#

BUILDDIR = ../../..
PACKAGE = sun.tools.security
PRODUCT = sun
PROGRAM = policytool
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level 
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Class to launch tools with.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.security.tools.PolicyTool\" }"

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

ifneq ($(PLATFORM), windows)
#
# Anything with a GUI needs X11 to be linked in.
#
OTHER_LDLIBS += -L$(OPENWIN_LIB) -lX11 
endif # PLATFORM
