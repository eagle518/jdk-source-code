#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.dom
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
        $(PKGDIR)/dom/ObjectFactory.java \
        $(PKGDIR)/dom/SecuritySupport.java \
        $(PKGDIR)/dom/ASDOMImplementationImpl.java \
        $(PKGDIR)/dom/ASModelImpl.java \
        $(PKGDIR)/dom/AttrImpl.java \
        $(PKGDIR)/dom/AttrNSImpl.java \
        $(PKGDIR)/dom/AttributeMap.java \
        $(PKGDIR)/dom/CDATASectionImpl.java \
        $(PKGDIR)/dom/CharacterDataImpl.java \
        $(PKGDIR)/dom/ChildNode.java \
        $(PKGDIR)/dom/CommentImpl.java \
        $(PKGDIR)/dom/CoreDOMImplementationImpl.java \
        $(PKGDIR)/dom/DOMImplementationListImpl.java \
        $(PKGDIR)/dom/CoreDocumentImpl.java \
        $(PKGDIR)/dom/DOMConfigurationImpl.java \
        $(PKGDIR)/dom/DOMErrorImpl.java \
        $(PKGDIR)/dom/DOMImplementationImpl.java \
        $(PKGDIR)/dom/DOMLocatorImpl.java \
        $(PKGDIR)/dom/DOMMessageFormatter.java \
        $(PKGDIR)/dom/DOMNormalizer.java \
        $(PKGDIR)/dom/DeepNodeListImpl.java \
        $(PKGDIR)/dom/DeferredAttrImpl.java \
        $(PKGDIR)/dom/DeferredAttrNSImpl.java \
        $(PKGDIR)/dom/DeferredCDATASectionImpl.java \
        $(PKGDIR)/dom/DeferredCommentImpl.java \
        $(PKGDIR)/dom/DeferredDocumentImpl.java \
        $(PKGDIR)/dom/DeferredDocumentTypeImpl.java \
        $(PKGDIR)/dom/DeferredElementDefinitionImpl.java \
        $(PKGDIR)/dom/DeferredElementImpl.java \
        $(PKGDIR)/dom/DeferredElementNSImpl.java \
        $(PKGDIR)/dom/DeferredEntityImpl.java \
        $(PKGDIR)/dom/DeferredEntityReferenceImpl.java \
        $(PKGDIR)/dom/DeferredNode.java \
        $(PKGDIR)/dom/DeferredNotationImpl.java \
        $(PKGDIR)/dom/DeferredProcessingInstructionImpl.java \
        $(PKGDIR)/dom/DeferredTextImpl.java \
        $(PKGDIR)/dom/DocumentFragmentImpl.java \
        $(PKGDIR)/dom/DocumentImpl.java \
        $(PKGDIR)/dom/DocumentTypeImpl.java \
        $(PKGDIR)/dom/ElementDefinitionImpl.java \
        $(PKGDIR)/dom/ElementImpl.java \
        $(PKGDIR)/dom/ElementNSImpl.java \
        $(PKGDIR)/dom/EntityImpl.java \
        $(PKGDIR)/dom/EntityReferenceImpl.java \
        $(PKGDIR)/dom/LCount.java \
        $(PKGDIR)/dom/NamedNodeMapImpl.java \
        $(PKGDIR)/dom/NodeImpl.java \
        $(PKGDIR)/dom/NodeIteratorImpl.java \
        $(PKGDIR)/dom/NodeListCache.java \
        $(PKGDIR)/dom/NotationImpl.java \
        $(PKGDIR)/dom/PSVIAttrNSImpl.java \
        $(PKGDIR)/dom/PSVIDOMImplementationImpl.java \
        $(PKGDIR)/dom/PSVIDocumentImpl.java \
        $(PKGDIR)/dom/PSVIElementNSImpl.java \
        $(PKGDIR)/dom/ParentNode.java \
        $(PKGDIR)/dom/ProcessingInstructionImpl.java \
        $(PKGDIR)/dom/RangeExceptionImpl.java \
        $(PKGDIR)/dom/RangeImpl.java \
        $(PKGDIR)/dom/TextImpl.java \
        $(PKGDIR)/dom/TreeWalkerImpl.java \
        $(PKGDIR)/dom/events/EventImpl.java \
        $(PKGDIR)/dom/events/MutationEventImpl.java \
        $(PKGDIR)/dom/DOMImplementationSourceImpl.java \
        $(PKGDIR)/dom/DOMInputImpl.java \
        $(PKGDIR)/dom/DOMStringListImpl.java \
        $(PKGDIR)/dom/DOMXSImplementationSourceImpl.java \
        $(PKGDIR)/dom/DOMOutputImpl.java \
        $(PKGDIR)/dom3/as/ASAttributeDeclaration.java \
        $(PKGDIR)/dom3/as/ASContentModel.java \
        $(PKGDIR)/dom3/as/ASDataType.java \
        $(PKGDIR)/dom3/as/ASElementDeclaration.java \
        $(PKGDIR)/dom3/as/ASEntityDeclaration.java \
        $(PKGDIR)/dom3/as/ASModel.java \
        $(PKGDIR)/dom3/as/ASNamedObjectMap.java \
        $(PKGDIR)/dom3/as/ASNotationDeclaration.java \
        $(PKGDIR)/dom3/as/ASObject.java \
        $(PKGDIR)/dom3/as/ASObjectList.java \
        $(PKGDIR)/dom3/as/CharacterDataEditAS.java \
        $(PKGDIR)/dom3/as/DOMASBuilder.java \
        $(PKGDIR)/dom3/as/DOMASException.java \
        $(PKGDIR)/dom3/as/DOMASWriter.java \
        $(PKGDIR)/dom3/as/DOMImplementationAS.java \
        $(PKGDIR)/dom3/as/DocumentAS.java \
        $(PKGDIR)/dom3/as/DocumentEditAS.java \
        $(PKGDIR)/dom3/as/ElementEditAS.java \
        $(PKGDIR)/dom3/as/NodeEditAS.java \




# Rule
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/xalan/internal/res/XSLTErrorResources.java 

#
# Resources
#
RESOURCE_FILES = \
	dom/org.apache.xerces.dom.DOMImplementationSourceImpl \
	dom/org.w3c.dom.DOMImplementationSourceList \
#OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
#NEW_RESOURCE_BUNDLES_PROPERTIES = \
#	dom/org.apache.xerces.dom.DOMImplementationSourceImpl \
#	dom/org.w3c.dom.DOMImplementationSourceList \


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
