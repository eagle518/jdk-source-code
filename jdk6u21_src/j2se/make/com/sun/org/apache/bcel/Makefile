#
# @(#)Makefile	1.7 10/03/23 
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.7 10/03/23 
#
# Makefile for building com.sun.org.apache.bcel
#

BUILDDIR = ../../../../..
PACKAGE = com.sun.org.apache.bcel
PRODUCT = bcel
include $(BUILDDIR)/common/Defs.gmk

#
# Take out enums flag until source code is updated
#
# LANGUAGE_VERSION = -source 1.4

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/org/apache/bcel



# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Resources
#
# RESOURCE_FILES = \
# OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
# NEW_RESOURCE_BUNDLES_PROPERTIES = \

RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)

include $(BUILDDIR)/common/Resources.gmk
