#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.jaxp
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
        $(PKGDIR)/jaxp/JAXPValidatorComponent.java \
        $(PKGDIR)/jaxp/DefaultValidationErrorHandler.java \
        $(PKGDIR)/jaxp/DocumentBuilderFactoryImpl.java \
        $(PKGDIR)/jaxp/DocumentBuilderImpl.java \
        $(PKGDIR)/jaxp/SAXParserFactoryImpl.java \
        $(PKGDIR)/jaxp/SAXParserImpl.java \
        $(PKGDIR)/jaxp/JAXPConstants.java \
        $(PKGDIR)/jaxp/validation/ValidatorImpl.java \
        $(PKGDIR)/jaxp/validation/ErrorHandlerAdaptor.java \
        $(PKGDIR)/jaxp/validation/ReadonlyGrammarPool.java \
        $(PKGDIR)/jaxp/validation/Util.java \
        $(PKGDIR)/jaxp/validation/ValidatorHandlerImpl.java \
        $(PKGDIR)/jaxp/validation/WrappedSAXException.java \
        $(PKGDIR)/jaxp/datatype/DatatypeFactoryImpl.java \
        $(PKGDIR)/jaxp/datatype/DurationImpl.java \
        $(PKGDIR)/jaxp/datatype/XMLGregorianCalendarImpl.java \



# Rule
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/xalan/internal/res/XSLTErrorResources.java 

#
# Resources
#
 RESOURCE_FILES = \
	jaxp/javax.xml.parsers.DocumentBuilderFactory \
	jaxp/javax.xml.parsers.SAXParserFactory \
	jaxp/validation/javax.xml.validation.SchemaFactory \
OTHER_PROPERTIES = \
	jaxp/javax.xml.parsers.DocumentBuilderFactory \
	jaxp/javax.xml.parsers.SAXParserFactory \
	jaxp/validation/javax.xml.validation.SchemaFactory \
LOCALE_SET_DEFINITION = j2re
NEW_RESOURCE_BUNDLES_PROPERTIES = \
#   xerces/internal/parsers/org.xml.sax.driver \


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
