#
# @(#)Makefile	1.29 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the tty (aka JDB) on JDI
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.example.debug.tty
PRODUCT = jpda
PROGRAM = jdb
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Java Files to compile are found automatically.
#
AUTO_FILES_JAVA_DIRS = com/sun/tools/example/debug/tty

#
# This gets added to OTHER_CPPFLAGS
#
JAVA_ARGS = "{\"com.sun.tools.example.debug.tty.TTY\"}"

#
# Override the default APP_CLASSPATH to pick up sa-jdi.jar also.
#   Default is defined in src/[solaris,windows]/bin/java_md.h
#   PROGRAM, JAVA_ARGS, and APP_CLASSPATH are used in src/share/bin/java.c
#   SA is currently not available on windows (for any ARCH), or linux-ia64:
ifneq ($(ARCH), ia64)
    JDB_CLASSPATH = "{ \"/lib/tools.jar\", \"/lib/sa-jdi.jar\", \"/classes\" }"
    OTHER_CPPFLAGS += -DAPP_CLASSPATH=$(JDB_CLASSPATH)
endif

#
# Resources
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_JAVA = $(TARGDIR)com/sun/tools/example/debug/tty/TTYResources.java
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

