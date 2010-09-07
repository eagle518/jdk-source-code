#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.xs
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
        $(PKGDIR)/xs/AttributePSVI.java \
        $(PKGDIR)/xs/ElementPSVI.java \
        $(PKGDIR)/xs/LSInputList.java \
        $(PKGDIR)/xs/PSVIProvider.java \
        $(PKGDIR)/xs/ShortList.java \
        $(PKGDIR)/xs/StringList.java \
        $(PKGDIR)/xs/XSAnnotation.java \
        $(PKGDIR)/xs/XSAttributeDeclaration.java \
        $(PKGDIR)/xs/XSAttributeUse.java \
        $(PKGDIR)/xs/XSComplexTypeDefinition.java \
        $(PKGDIR)/xs/XSConstants.java \
        $(PKGDIR)/xs/XSElementDeclaration.java \
        $(PKGDIR)/xs/XSException.java \
        $(PKGDIR)/xs/XSFacet.java \
        $(PKGDIR)/xs/XSIDCDefinition.java \
        $(PKGDIR)/xs/XSImplementation.java \
        $(PKGDIR)/xs/XSLoader.java \
        $(PKGDIR)/xs/XSModel.java \
        $(PKGDIR)/xs/XSModelGroup.java \
        $(PKGDIR)/xs/XSModelGroupDefinition.java \
        $(PKGDIR)/xs/XSMultiValueFacet.java \
        $(PKGDIR)/xs/XSNamedMap.java \
        $(PKGDIR)/xs/XSNamespaceItem.java \
        $(PKGDIR)/xs/XSNamespaceItemList.java \
        $(PKGDIR)/xs/XSNotationDeclaration.java \
        $(PKGDIR)/xs/XSObject.java \
        $(PKGDIR)/xs/XSObjectList.java \
        $(PKGDIR)/xs/XSParticle.java \
        $(PKGDIR)/xs/XSSimpleTypeDefinition.java \
        $(PKGDIR)/xs/XSTerm.java \
        $(PKGDIR)/xs/XSTypeDefinition.java \
        $(PKGDIR)/xs/XSWildcard.java \



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
