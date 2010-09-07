#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building Xalan
#

BUILDDIR = ../../../../..
PACKAGE = com.sun.org.apache
PRODUCT = regexp
include $(BUILDDIR)/common/Defs.gmk

#
# Take out enums flag until source code is updated
#
# LANGUAGE_VERSION = -source 1.4

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/regexp/internal/CharacterArrayCharacterIterator.java \
	$(PKGDIR)/regexp/internal/CharacterIterator.java \
	$(PKGDIR)/regexp/internal/RE.java \
	$(PKGDIR)/regexp/internal/RECompiler.java \
	$(PKGDIR)/regexp/internal/REDebugCompiler.java \
	$(PKGDIR)/regexp/internal/REProgram.java \
	$(PKGDIR)/regexp/internal/RESyntaxException.java \
	$(PKGDIR)/regexp/internal/RETest.java \
	$(PKGDIR)/regexp/internal/REUtil.java \
	$(PKGDIR)/regexp/internal/ReaderCharacterIterator.java \
	$(PKGDIR)/regexp/internal/StreamCharacterIterator.java \
	$(PKGDIR)/regexp/internal/StringCharacterIterator.java \
	$(PKGDIR)/regexp/internal/recompile.java \



# Rules
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/xalan/res/XSLTErrorResources.java 

#
# Resources
#
# RESOURCE_FILES = \
#OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
NEW_RESOURCE_BUNDLES_PROPERTIES = \
#   xalan/res/XSLTInfo.properties \
#   xalan/serialize/serializer.properties \
#   xalan/serialize/XMLEntities.res \
#   xalan/serialize/HTMLEntities.res \
#   xalan/templates/output_html.properties \
#   xalan/templates/output_text.properties \
#   xalan/templates/output_xml.properties \
	xpath/internal/res/XPATHErrorResources.properties

RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:	$(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
