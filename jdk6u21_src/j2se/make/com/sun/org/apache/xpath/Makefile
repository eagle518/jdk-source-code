#
# @(#)Makefile	1.4 05/01/26
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building com.sun.org.apache.xpath code
#

BUILDDIR = ../../../../..
PACKAGE = com.sun.org.apache.xpath
PRODUCT = xpath
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
include FILES.gmk
AUTO_FILES_JAVA_DIRS = com/sun/org/apache/xpath

#
# Resources (only if you have them)
#
LOCALE_SET_DEFINITION = j2sdk
NEW_RESOURCE_BUNDLES_PROPERTIES = $(PROPERTY_FILES)
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
RESOURCE_DEST_DIR = $(CLASSDESTDIR)
include $(BUILDDIR)/common/Resources.gmk

#
# Apache can use a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

#
# Rules
#

build: other_files

clean:: classes.clean

include $(BUILDDIR)/common/Classes.gmk

# TODO: still relevant?
other_files: $(MISC_FILES)
