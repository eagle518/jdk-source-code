#
# @(#)Makefile	1.6 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jinfo
#

BUILDDIR = ../..
PACKAGE = sun.jvm.hotspot.tools
PRODUCT = sun
PROGRAM = jinfo
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

SA_CLASSPATH = "{ \"/lib/tools.jar\", \"/lib/sa-jdi.jar\", \"/classes\"}"
OTHER_CPPFLAGS += -DAPP_CLASSPATH=$(SA_CLASSPATH)

AUTO_FILES_JAVA_DIRS = sun/tools/jinfo

#
# Files.
#
# none

#
# Rules.
#
JAVA_ARGS = "{ \"-Dsun.jvm.hotspot.debugger.useProcDebugger\", \"-Dsun.jvm.hotspot.debugger.useWindbgDebugger\", \"sun.tools.jinfo.JInfo\" }"

include $(BUILDDIR)/common/Program.gmk
