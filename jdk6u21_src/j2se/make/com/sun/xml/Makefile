#
# @(#)Makefile	1.29 05/04/04
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building packages under javax.xml
#

BUILDDIR = ../../..
PACKAGE = com.sun.xml
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk



#
# Files to compile
#
include FILES.gmk
AUTO_FILES_JAVA_DIRS = com/sun/codemodel com/sun/istack com/sun/activation com/sun/xml com/sun/xml/internal/dtdparser com/sun/tools/internal/xjc com/sun/tools/internal/jxc


#Resources (only if you have them)
LOCALE_SET_DEFINITION = j2sdk
NEW_RESOURCE_BUNDLES_PROPERTIES = $(PROPERTY_FILES)
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
RESOURCE_DEST_DIR = $(CLASSBINDIR)
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#

build: other_files

clean:: classes.clean

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

other_files: $(MISC_FILES)

$(CLASSBINDIR)/%.xsd: $(SHARE_SRC)/classes/%.xsd
	$(install-file)

