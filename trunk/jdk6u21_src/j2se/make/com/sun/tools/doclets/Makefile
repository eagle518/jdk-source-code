#
# @(#)Makefile	1.50 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building doclets and the doclet toolkit
#

BUILDDIR = ../../../..
PACKAGE = com.sun.tools.doclets
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/tools/doclets

#
# Resources
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
	$(PKGDIR)/internal/toolkit/resources/doclets.properties \
	$(PKGDIR)/formats/html/resources/standard.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

GIF_FILES =	   $(PKGDIR)/internal/toolkit/resources/inherit.gif
LAYOUT_XML_FILES = $(PKGDIR)/internal/toolkit/resources/doclet.xml

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

all: gif_files layout_xml_files classes

gif_files: $(CLASSBINDIR)/$(GIF_FILES)

layout_xml_files: $(CLASSBINDIR)/$(LAYOUT_XML_FILES)


$(CLASSBINDIR)/%.gif: $(SHARE_SRC)/classes/%.gif
	$(install-file)

$(CLASSBINDIR)/%.xml: $(SHARE_SRC)/classes/%.xml
	$(install-file)

.PHONY: gif_files layout_xml_files
