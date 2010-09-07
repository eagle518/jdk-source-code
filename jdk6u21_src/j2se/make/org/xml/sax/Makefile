#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.10 10/03/23
#
# Makefile for building SAX API classes
#

BUILDDIR = ../../..
PACKAGE = org.xml.sax
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
FILES_java = \
	$(PKGDIR)/AttributeList.java \
	$(PKGDIR)/Attributes.java \
	$(PKGDIR)/ContentHandler.java \
	$(PKGDIR)/DTDHandler.java \
	$(PKGDIR)/DocumentHandler.java \
	$(PKGDIR)/EntityResolver.java \
	$(PKGDIR)/ErrorHandler.java \
	$(PKGDIR)/HandlerBase.java \
	$(PKGDIR)/InputSource.java \
	$(PKGDIR)/Locator.java \
	$(PKGDIR)/Parser.java \
	$(PKGDIR)/SAXException.java \
	$(PKGDIR)/SAXNotRecognizedException.java \
	$(PKGDIR)/SAXNotSupportedException.java \
	$(PKGDIR)/SAXParseException.java \
	$(PKGDIR)/XMLFilter.java \
	$(PKGDIR)/XMLReader.java \
	\
	$(PKGDIR)/helpers/AttributeListImpl.java \
	$(PKGDIR)/helpers/AttributesImpl.java \
	$(PKGDIR)/helpers/DefaultHandler.java \
	$(PKGDIR)/helpers/LocatorImpl.java \
	$(PKGDIR)/helpers/NamespaceSupport.java \
	$(PKGDIR)/helpers/ParserAdapter.java \
	$(PKGDIR)/helpers/ParserFactory.java \
	$(PKGDIR)/helpers/XMLFilterImpl.java \
	$(PKGDIR)/helpers/XMLReaderAdapter.java \
	$(PKGDIR)/helpers/XMLReaderFactory.java \
	$(PKGDIR)/helpers/NewInstance.java \
	\
	$(PKGDIR)/ext/DeclHandler.java \
	$(PKGDIR)/ext/Attributes2.java \
	$(PKGDIR)/ext/Attributes2Impl.java \
	$(PKGDIR)/ext/DefaultHandler2.java \
	$(PKGDIR)/ext/EntityResolver2.java \
	$(PKGDIR)/ext/Locator2.java \
	$(PKGDIR)/ext/Locator2Impl.java \
	$(PKGDIR)/ext/LexicalHandler.java

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk
