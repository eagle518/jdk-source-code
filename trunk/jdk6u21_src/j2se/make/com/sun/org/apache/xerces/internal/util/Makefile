#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.util
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
#        $(PKGDIR)/util/XMLCatalogResolver.java \
# Files to compile
#
FILES_java =\
        $(PKGDIR)/util/SAXMessageFormatter.java \
        $(PKGDIR)/util/AttributesProxy.java \
        $(PKGDIR)/util/DraconianErrorHandler.java \
        $(PKGDIR)/util/ErrorHandlerProxy.java \
        $(PKGDIR)/util/LocatorProxy.java \
        $(PKGDIR)/util/LocatorWrapper.java \
        $(PKGDIR)/util/TeeXMLDocumentFilterImpl.java \
        $(PKGDIR)/util/TypeInfoImpl.java \
        $(PKGDIR)/util/XMLDocumentFilterImpl.java \
        $(PKGDIR)/util/AugmentationsImpl.java \
        $(PKGDIR)/util/DOMEntityResolverWrapper.java \
        $(PKGDIR)/util/DOMErrorHandlerWrapper.java \
        $(PKGDIR)/util/DOMUtil.java \
        $(PKGDIR)/util/DefaultErrorHandler.java \
        $(PKGDIR)/util/EncodingMap.java \
        $(PKGDIR)/util/EntityResolverWrapper.java \
        $(PKGDIR)/util/EntityResolver2Wrapper.java \
        $(PKGDIR)/util/ErrorHandlerWrapper.java \
        $(PKGDIR)/util/IntStack.java \
        $(PKGDIR)/util/MessageFormatter.java \
        $(PKGDIR)/util/NamespaceSupport.java \
        $(PKGDIR)/util/ParserConfigurationSettings.java \
        $(PKGDIR)/util/SecurityManager.java \
        $(PKGDIR)/util/ShadowedSymbolTable.java \
        $(PKGDIR)/util/SymbolHash.java \
        $(PKGDIR)/util/SymbolTable.java \
        $(PKGDIR)/util/SynchronizedSymbolTable.java \
        $(PKGDIR)/util/URI.java \
        $(PKGDIR)/util/XML11Char.java \
        $(PKGDIR)/util/XMLAttributesImpl.java \
        $(PKGDIR)/util/XMLChar.java \
        $(PKGDIR)/util/XMLGrammarPoolImpl.java \
        $(PKGDIR)/util/XMLResourceIdentifierImpl.java \
        $(PKGDIR)/util/XMLStringBuffer.java \
        $(PKGDIR)/util/XMLSymbols.java \
        $(PKGDIR)/util/SAX2XNI.java \
        $(PKGDIR)/util/XMLInputSourceAdaptor.java \
        $(PKGDIR)/util/XMLEntityDescriptionImpl.java \
        $(PKGDIR)/util/XMLErrorCode.java \
        $(PKGDIR)/util/DatatypeMessageFormatter.java \



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
