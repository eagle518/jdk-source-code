#
# @(#)Makefile	1.17 10/03/23
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building javaw.exe (for windows)
#

BUILDDIR = ../../..
PROGRAM = javaw
PRODUCT = java

#
# Statically link javaw to avoid the dependency on msvcr71.dll.  This
# must be set before Defs.gmk is included.
#
MS_RUNTIME_STATIC = true

#
# Statically link javaw to avoid the dependency on jli.dll.  This
# must be set before Program.gmk is included.
#
STATIC_JLI = true

include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level 
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
# 
BUILD_MANIFEST=true

OTHER_CPPFLAGS += -DJAVAW
LDLIBS_COMMON +=  user32.lib

# Override the default version info with our own resource file (see 5106536)
ifeq ($(PLATFORM), windows)
  VERSIONINFO_RESOURCE = $(PLATFORM_SRC)/resource/java.rc
endif

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS
