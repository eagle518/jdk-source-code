#
# @(#)Makefile	1.29 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for javah.
#

BUILDDIR = ../../..
PACKAGE = com.sun.tools.javah
PRODUCT = sun
PROGRAM = javah
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
AUTO_FILES_JAVA_DIRS = com/sun/tools/javah

#
# Rules.
#
JAVA_ARGS = "{ \"com.sun.tools.javah.Main\" }"

include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS

#
# Resources.
#

RESOURCE_BUNDLES_PROPERTIES = l10n.properties

ifeq ($(PLATFORM), solaris)
OTHER_PROPERTIES = SunOS_sparc.properties SunOS_sparcv9.properties
endif 

# property files are at src/share/classes/com/sun/tools/javah/resources/
ifeq ($(PLATFORM), windows)
OTHER_PROPERTIES = win32_x86.properties
endif # PLATFORM

ifeq ($(PLATFORM), linux)
   OTHER_PROPERTIES = Linux_ppc.properties Linux_sparc.properties
endif

LOCALE_SET_DEFINITION = j2sdk
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/com/sun/tools/javah/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/com/sun/tools/javah/resources
include $(BUILDDIR)/common/Resources.gmk
