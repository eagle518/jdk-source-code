#
# @(#)Makefile	1.6 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jsadebugd
#

BUILDDIR = ../..
PACKAGE = sun.jvm.hotspot.jdi
PRODUCT = sun
PROGRAM = jsadebugd
include $(BUILDDIR)/common/Defs.gmk

SA_CLASSPATH = "{ \"/lib/tools.jar\", \"/lib/sa-jdi.jar\", \"/classes\"}"
OTHER_CPPFLAGS += -DAPP_CLASSPATH=$(SA_CLASSPATH)

#
# Files.
#
# none

#
# Rules.
#
JAVA_ARGS = "{ \"sun.jvm.hotspot.jdi.SADebugServer\" }"
include $(BUILDDIR)/common/Program.gmk
