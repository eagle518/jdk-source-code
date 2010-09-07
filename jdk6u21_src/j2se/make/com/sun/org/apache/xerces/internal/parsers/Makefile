#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.parsers
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
        $(PKGDIR)/parsers/ObjectFactory.java \
        $(PKGDIR)/parsers/SecuritySupport.java \
        $(PKGDIR)/parsers/AbstractDOMParser.java \
        $(PKGDIR)/parsers/AbstractSAXParser.java \
        $(PKGDIR)/parsers/AbstractXMLDocumentParser.java \
        $(PKGDIR)/parsers/BasicParserConfiguration.java \
        $(PKGDIR)/parsers/CachingParserPool.java \
        $(PKGDIR)/parsers/DOMASBuilderImpl.java \
        $(PKGDIR)/parsers/DOMParser.java \
        $(PKGDIR)/parsers/DOMParserImpl.java \
        $(PKGDIR)/parsers/DTDConfiguration.java \
        $(PKGDIR)/parsers/DTDParser.java \
        $(PKGDIR)/parsers/IntegratedParserConfiguration.java \
        $(PKGDIR)/parsers/NonValidatingConfiguration.java \
        $(PKGDIR)/parsers/SAXParser.java \
        $(PKGDIR)/parsers/SecurityConfiguration.java \
        $(PKGDIR)/parsers/StandardParserConfiguration.java \
        $(PKGDIR)/parsers/XIncludeParserConfiguration.java \
        $(PKGDIR)/parsers/XML11Configuration.java \
        $(PKGDIR)/parsers/XMLDocumentParser.java \
        $(PKGDIR)/parsers/XMLGrammarCachingConfiguration.java \
        $(PKGDIR)/parsers/XMLGrammarParser.java \
        $(PKGDIR)/parsers/XMLGrammarPreparser.java \
        $(PKGDIR)/parsers/XMLParser.java \
        $(PKGDIR)/parsers/XPointerParserConfiguration.java \


#        $(PKGDIR)/parsers/DOMBuilderImpl.java \
# Rule
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/xalan/internal/res/XSLTErrorResources.java 

#
# Resources
#
RESOURCE_FILES = \
	parsers/org.apache.xerces.xni.parser.DTDConfiguration \
	parsers/org.apache.xerces.xni.parser.XML11Configuration \
	parsers/org.apache.xerces.xni.parser.XMLParserConfiguration \
	parsers/org.xml.sax.driver \
OTHER_PROPERTIES = \
	parsers/org.apache.xerces.xni.parser.DTDConfiguration \
	parsers/org.apache.xerces.xni.parser.XML11Configuration \
	parsers/org.apache.xerces.xni.parser.XMLParserConfiguration \
	parsers/org.xml.sax.driver \
LOCALE_SET_DEFINITION = j2re
#NEW_RESOURCE_BUNDLES_PROPERTIES = \
#   xerces/internal/parsers/org.xml.sax.driver \


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
