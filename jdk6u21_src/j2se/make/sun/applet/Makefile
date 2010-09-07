#
# @(#)Makefile	1.57 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for appletviewer classes, wrappers, and resources.
#

BUILDDIR = ../..
PACKAGE = sun.applet
PRODUCT = sun
PROGRAM = appletviewer
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
AUTO_FILES_JAVA_DIRS = sun/applet

#
# Resources
#
RESOURCE_BUNDLES_JAVA = $(TARGDIR)sun/applet/resources/MsgAppletViewer.java
LOCALE_SET_DEFINITION = j2re
include $(BUILDDIR)/common/Resources.gmk

ifneq ($(PLATFORM), windows)
#
# Anything with a GUI needs X11 to be linked in.
#
OTHER_LDLIBS += -L$(OPENWIN_LIB) -lX11 
endif # PLATFORM

#
# Rules.
#
JAVA_ARGS = "{ \"sun.applet.Main\" }"
include $(BUILDDIR)/common/Program.gmk

