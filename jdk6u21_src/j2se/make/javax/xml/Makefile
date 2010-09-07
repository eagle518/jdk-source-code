#
# @(#)Makefile	1.29 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building packages under javax.xml
#

BUILDDIR = ../..
PACKAGE = javax.xml
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
include FILES.gmk
AUTO_FILES_JAVA_DIRS = javax/xml


#Resources 
LOCALE_SET_DEFINITION = j2sdk
NEW_RESOURCE_BUNDLES_PROPERTIES = $(PROPERTY_FILES)
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
RESOURCE_DEST_DIR = $(CLASSBINDIR)
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

