#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.xni
#

BUILDDIR = ../../../../../../..
PACKAGE = com.sun.org.apache.xerces.internal
PRODUCT = xerces
include $(BUILDDIR)/common/Defs.gmk

#
# Take out enums flag until source code is updated
#
# LANGUAGE_VERSION = -source 1.4

#
# Files to compile
#
FILES_java =\
        $(PKGDIR)/xni/Augmentations.java \
        $(PKGDIR)/xni/NamespaceContext.java \
        $(PKGDIR)/xni/QName.java \
        $(PKGDIR)/xni/XMLAttributes.java \
        $(PKGDIR)/xni/XMLDTDContentModelHandler.java \
        $(PKGDIR)/xni/XMLDTDHandler.java \
        $(PKGDIR)/xni/XMLDocumentFragmentHandler.java \
        $(PKGDIR)/xni/XMLDocumentHandler.java \
        $(PKGDIR)/xni/XMLLocator.java \
        $(PKGDIR)/xni/XMLResourceIdentifier.java \
        $(PKGDIR)/xni/XMLString.java \
        $(PKGDIR)/xni/XNIException.java \
        $(PKGDIR)/xni/grammars/Grammar.java \
        $(PKGDIR)/xni/grammars/XMLDTDDescription.java \
        $(PKGDIR)/xni/grammars/XMLGrammarDescription.java \
        $(PKGDIR)/xni/grammars/XMLGrammarLoader.java \
        $(PKGDIR)/xni/grammars/XMLGrammarPool.java \
        $(PKGDIR)/xni/grammars/XMLSchemaDescription.java \
        $(PKGDIR)/xni/grammars/XSGrammar.java \
        $(PKGDIR)/xni/parser/XMLComponent.java \
        $(PKGDIR)/xni/parser/XMLComponentManager.java \
        $(PKGDIR)/xni/parser/XMLConfigurationException.java \
        $(PKGDIR)/xni/parser/XMLDTDContentModelFilter.java \
        $(PKGDIR)/xni/parser/XMLDTDContentModelSource.java \
        $(PKGDIR)/xni/parser/XMLDTDFilter.java \
        $(PKGDIR)/xni/parser/XMLDTDScanner.java \
        $(PKGDIR)/xni/parser/XMLDTDSource.java \
        $(PKGDIR)/xni/parser/XMLDocumentFilter.java \
        $(PKGDIR)/xni/parser/XMLDocumentScanner.java \
        $(PKGDIR)/xni/parser/XMLDocumentSource.java \
        $(PKGDIR)/xni/parser/XMLEntityResolver.java \
        $(PKGDIR)/xni/parser/XMLErrorHandler.java \
        $(PKGDIR)/xni/parser/XMLInputSource.java \
        $(PKGDIR)/xni/parser/XMLParseException.java \
        $(PKGDIR)/xni/parser/XMLParserConfiguration.java \
        $(PKGDIR)/xni/parser/XMLPullParserConfiguration.java \




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
#   xerces/internal/parsers/org.xml.sax.driver \


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
