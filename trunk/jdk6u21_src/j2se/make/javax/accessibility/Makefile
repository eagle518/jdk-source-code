#
# @(#)Makefile	1.35 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.accessibility
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
AUTO_FILES_JAVA_DIRS = javax/accessibility

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = com/sun/accessibility/internal/resources/accessibility.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk
LOCALE_SUFFIXES += en

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

