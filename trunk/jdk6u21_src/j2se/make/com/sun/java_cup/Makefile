#
# @(#)Makefile	1.6 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building com.sun.org.apache.internal.html
#

BUILDDIR = ../../..
PACKAGE = com.sun
PRODUCT = java_cup
include $(BUILDDIR)/common/Defs.gmk

#
# Take out enums flag until source code is updated
#
# LANGUAGE_VERSION = -source 1.4

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = $(PKGDIR)/java_cup/internal

# Rule
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/xalan/internal/res/XSLTErrorResources.java 

#
# Resources
#
# RESOURCE_FILES = \
#OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
#NEW_RESOURCE_BUNDLES_PROPERTIES = \
#   xalan/internal/res/XSLTInfo.properties \
#   xalan/internal/serialize/serializer.properties \
#   xalan/internal/serialize/XMLEntities.res \
#   xalan/internal/serialize/HTMLEntities.res \
#   xalan/internal/serialize/Encodings.properties \
#   xalan/internal/templates/output_html.properties \
#   xalan/internal/templates/output_text.properties \
#   xalan/internal/templates/output_xml.properties 


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
