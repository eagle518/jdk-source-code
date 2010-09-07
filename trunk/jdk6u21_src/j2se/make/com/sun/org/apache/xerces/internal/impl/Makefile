#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.impl
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
        $(PKGDIR)/impl/Constants.java \
        $(PKGDIR)/impl/RevalidationHandler.java \
        $(PKGDIR)/impl/Version.java \
        $(PKGDIR)/impl/XML11NSDocumentScannerImpl.java \
        $(PKGDIR)/impl/XML11DTDScannerImpl.java \
        $(PKGDIR)/impl/XML11DocumentScannerImpl.java \
        $(PKGDIR)/impl/XML11EntityScanner.java \
        $(PKGDIR)/impl/XML11NamespaceBinder.java \
        $(PKGDIR)/impl/XMLDTDScannerImpl.java \
        $(PKGDIR)/impl/XMLDocumentFragmentScannerImpl.java \
        $(PKGDIR)/impl/XMLDocumentScannerImpl.java \
        $(PKGDIR)/impl/XMLEntityHandler.java \
        $(PKGDIR)/impl/XMLEntityManager.java \
        $(PKGDIR)/impl/XMLEntityScanner.java \
        $(PKGDIR)/impl/XMLErrorReporter.java \
        $(PKGDIR)/impl/XMLNSDocumentScannerImpl.java \
        $(PKGDIR)/impl/XMLNamespaceBinder.java \
        $(PKGDIR)/impl/XMLScanner.java \
        $(PKGDIR)/impl/XMLVersionDetector.java \
        $(PKGDIR)/impl/ExternalSubsetResolver.java \
        $(PKGDIR)/impl/XMLEntityDescription.java \
        $(PKGDIR)/impl/dtd/DTDGrammar.java \
        $(PKGDIR)/impl/dtd/DTDGrammarBucket.java \
        $(PKGDIR)/impl/dtd/XML11NSDTDValidator.java \
        $(PKGDIR)/impl/dtd/XML11DTDProcessor.java \
        $(PKGDIR)/impl/dtd/XML11DTDValidator.java \
        $(PKGDIR)/impl/dtd/XMLAttributeDecl.java \
        $(PKGDIR)/impl/dtd/XMLContentSpec.java \
        $(PKGDIR)/impl/dtd/XMLDTDDescription.java \
        $(PKGDIR)/impl/dtd/XMLDTDLoader.java \
        $(PKGDIR)/impl/dtd/XMLDTDProcessor.java \
        $(PKGDIR)/impl/dtd/XMLDTDValidator.java \
        $(PKGDIR)/impl/dtd/XMLDTDValidatorFilter.java \
        $(PKGDIR)/impl/dtd/XMLElementDecl.java \
        $(PKGDIR)/impl/dtd/XMLEntityDecl.java \
        $(PKGDIR)/impl/dtd/XMLNSDTDValidator.java \
        $(PKGDIR)/impl/dtd/XMLNotationDecl.java \
        $(PKGDIR)/impl/dtd/XMLSimpleType.java \
        $(PKGDIR)/impl/dtd/models/CMAny.java \
        $(PKGDIR)/impl/dtd/models/CMBinOp.java \
        $(PKGDIR)/impl/dtd/models/CMLeaf.java \
        $(PKGDIR)/impl/dtd/models/CMNode.java \
        $(PKGDIR)/impl/dtd/models/CMStateSet.java \
        $(PKGDIR)/impl/dtd/models/CMUniOp.java \
        $(PKGDIR)/impl/dtd/models/ContentModelValidator.java \
        $(PKGDIR)/impl/dtd/models/DFAContentModel.java \
        $(PKGDIR)/impl/dtd/models/MixedContentModel.java \
        $(PKGDIR)/impl/dtd/models/SimpleContentModel.java \
        \
        $(PKGDIR)/impl/dv/ObjectFactory.java \
        $(PKGDIR)/impl/dv/SecuritySupport.java \
        $(PKGDIR)/impl/dv/DTDDVFactory.java \
        $(PKGDIR)/impl/dv/DVFactoryException.java \
        $(PKGDIR)/impl/dv/DatatypeException.java \
        $(PKGDIR)/impl/dv/DatatypeValidator.java \
        $(PKGDIR)/impl/dv/InvalidDatatypeFacetException.java \
        $(PKGDIR)/impl/dv/InvalidDatatypeValueException.java \
        $(PKGDIR)/impl/dv/SchemaDVFactory.java \
        $(PKGDIR)/impl/dv/ValidatedInfo.java \
        $(PKGDIR)/impl/dv/ValidationContext.java \
        $(PKGDIR)/impl/dv/XSFacets.java \
        $(PKGDIR)/impl/dv/XSSimpleType.java \
        $(PKGDIR)/impl/dv/dtd/DTDDVFactoryImpl.java \
        $(PKGDIR)/impl/dv/dtd/ENTITYDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/IDDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/IDREFDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/ListDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/NMTOKENDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/NOTATIONDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/StringDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/XML11DTDDVFactoryImpl.java \
        $(PKGDIR)/impl/dv/dtd/XML11IDDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/XML11IDREFDatatypeValidator.java \
        $(PKGDIR)/impl/dv/dtd/XML11NMTOKENDatatypeValidator.java \
        $(PKGDIR)/impl/dv/util/Base64.java \
        $(PKGDIR)/impl/dv/util/HexBin.java \
        $(PKGDIR)/impl/dv/xs/AbstractDateTimeDV.java \
        $(PKGDIR)/impl/dv/xs/AnySimpleDV.java \
        $(PKGDIR)/impl/dv/xs/AnyURIDV.java \
        $(PKGDIR)/impl/dv/xs/Base64BinaryDV.java \
        $(PKGDIR)/impl/dv/xs/BaseDVFactory.java \
        $(PKGDIR)/impl/dv/xs/BooleanDV.java \
        $(PKGDIR)/impl/dv/xs/DateDV.java \
        $(PKGDIR)/impl/dv/xs/DateTimeDV.java \
        $(PKGDIR)/impl/dv/xs/DayDV.java \
        $(PKGDIR)/impl/dv/xs/DecimalDV.java \
        $(PKGDIR)/impl/dv/xs/DoubleDV.java \
        $(PKGDIR)/impl/dv/xs/DurationDV.java \
        $(PKGDIR)/impl/dv/xs/EntityDV.java \
        $(PKGDIR)/impl/dv/xs/FloatDV.java \
        $(PKGDIR)/impl/dv/xs/FullDVFactory.java \
        $(PKGDIR)/impl/dv/xs/HexBinaryDV.java \
        $(PKGDIR)/impl/dv/xs/IDDV.java \
        $(PKGDIR)/impl/dv/xs/IDREFDV.java \
        $(PKGDIR)/impl/dv/xs/IntegerDV.java \
        $(PKGDIR)/impl/dv/xs/ListDV.java \
        $(PKGDIR)/impl/dv/xs/MonthDV.java \
        $(PKGDIR)/impl/dv/xs/MonthDayDV.java \
        $(PKGDIR)/impl/dv/xs/QNameDV.java \
        $(PKGDIR)/impl/dv/xs/SchemaDVFactoryImpl.java \
        $(PKGDIR)/impl/dv/xs/SchemaDateTimeException.java \
        $(PKGDIR)/impl/dv/xs/StringDV.java \
        $(PKGDIR)/impl/dv/xs/TimeDV.java \
        $(PKGDIR)/impl/dv/xs/TypeValidator.java \
        $(PKGDIR)/impl/dv/xs/UnionDV.java \
        $(PKGDIR)/impl/dv/xs/XSSimpleTypeDecl.java \
        $(PKGDIR)/impl/dv/xs/YearDV.java \
        $(PKGDIR)/impl/dv/xs/YearMonthDV.java \
        \
        $(PKGDIR)/impl/io/ASCIIReader.java \
        $(PKGDIR)/impl/io/UCSReader.java \
        $(PKGDIR)/impl/io/UTF8Reader.java \
        $(PKGDIR)/impl/io/MalformedByteSequenceException.java \
        \
        $(PKGDIR)/impl/msg/XMLMessageFormatter.java \
        \
        $(PKGDIR)/impl/validation/EntityState.java \
        $(PKGDIR)/impl/validation/ValidationManager.java \
        $(PKGDIR)/impl/validation/ValidationState.java \
        $(PKGDIR)/impl/xpath/XPath.java \
        $(PKGDIR)/impl/xpath/XPathException.java \
        $(PKGDIR)/impl/xpath/regex/BMPattern.java \
        $(PKGDIR)/impl/xpath/regex/Match.java \
        $(PKGDIR)/impl/xpath/regex/Op.java \
        $(PKGDIR)/impl/xpath/regex/ParseException.java \
        $(PKGDIR)/impl/xpath/regex/ParserForXMLSchema.java \
        $(PKGDIR)/impl/xpath/regex/REUtil.java \
        $(PKGDIR)/impl/xpath/regex/RangeToken.java \
        $(PKGDIR)/impl/xpath/regex/RegexParser.java \
        $(PKGDIR)/impl/xpath/regex/RegularExpression.java \
        $(PKGDIR)/impl/xpath/regex/Token.java \
	$(PKGDIR)/impl/xs/AttributePSVImpl.java \
        $(PKGDIR)/impl/xs/ElementPSVImpl.java \
        $(PKGDIR)/impl/xs/SchemaGrammar.java \
        $(PKGDIR)/impl/xs/SchemaNamespaceSupport.java \
        $(PKGDIR)/impl/xs/SchemaSymbols.java \
        $(PKGDIR)/impl/xs/SubstitutionGroupHandler.java \
        $(PKGDIR)/impl/xs/XMLSchemaException.java \
        $(PKGDIR)/impl/xs/XMLSchemaLoader.java \
        $(PKGDIR)/impl/xs/XMLSchemaValidator.java \
        $(PKGDIR)/impl/xs/XSAttributeDecl.java \
        $(PKGDIR)/impl/xs/XSAttributeGroupDecl.java \
        $(PKGDIR)/impl/xs/XSAttributeUseImpl.java \
        $(PKGDIR)/impl/xs/XSComplexTypeDecl.java \
        $(PKGDIR)/impl/xs/XSConstraints.java \
        $(PKGDIR)/impl/xs/XSDDescription.java \
        $(PKGDIR)/impl/xs/XSDeclarationPool.java \
        $(PKGDIR)/impl/xs/XSElementDecl.java \
        $(PKGDIR)/impl/xs/XSGroupDecl.java \
        $(PKGDIR)/impl/xs/XSMessageFormatter.java \
        $(PKGDIR)/impl/xs/XSModelGroupImpl.java \
        $(PKGDIR)/impl/xs/XSModelImpl.java \
        $(PKGDIR)/impl/xs/XSNotationDecl.java \
        $(PKGDIR)/impl/xs/XSParticleDecl.java \
        $(PKGDIR)/impl/xs/XSWildcardDecl.java \
        $(PKGDIR)/impl/xs/XSImplementationImpl.java \
        $(PKGDIR)/impl/xs/identity/Field.java \
        $(PKGDIR)/impl/xs/identity/FieldActivator.java \
        $(PKGDIR)/impl/xs/identity/IdentityConstraint.java \
        $(PKGDIR)/impl/xs/identity/KeyRef.java \
        $(PKGDIR)/impl/xs/identity/Selector.java \
        $(PKGDIR)/impl/xs/identity/UniqueOrKey.java \
        $(PKGDIR)/impl/xs/identity/ValueStore.java \
        $(PKGDIR)/impl/xs/identity/XPathMatcher.java \
        $(PKGDIR)/impl/xs/models/CMBuilder.java \
        $(PKGDIR)/impl/xs/models/CMNodeFactory.java \
        $(PKGDIR)/impl/xs/models/XSAllCM.java \
        $(PKGDIR)/impl/xs/models/XSCMBinOp.java \
        $(PKGDIR)/impl/xs/models/XSCMLeaf.java \
        $(PKGDIR)/impl/xs/models/XSCMUniOp.java \
        $(PKGDIR)/impl/xs/models/XSCMValidator.java \
        $(PKGDIR)/impl/xs/models/XSDFACM.java \
        $(PKGDIR)/impl/xs/models/XSEmptyCM.java \
        $(PKGDIR)/impl/xs/opti/DefaultDocument.java \
        $(PKGDIR)/impl/xs/opti/DefaultElement.java \
        $(PKGDIR)/impl/xs/opti/DefaultNode.java \
        $(PKGDIR)/impl/xs/opti/DefaultXMLDocumentHandler.java \
        $(PKGDIR)/impl/xs/opti/ElementImpl.java \
        $(PKGDIR)/impl/xs/opti/NamedNodeMapImpl.java \
        $(PKGDIR)/impl/xs/opti/NodeImpl.java \
        $(PKGDIR)/impl/xs/opti/SchemaDOM.java \
        $(PKGDIR)/impl/xs/opti/SchemaDOMParser.java \
        $(PKGDIR)/impl/xs/opti/SchemaParsingConfig.java \
        $(PKGDIR)/impl/xs/traversers/XSAttributeChecker.java \
        $(PKGDIR)/impl/xs/traversers/XSDAbstractIDConstraintTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDAbstractParticleTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDAbstractTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDAttributeGroupTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDAttributeTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDComplexTypeTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDElementTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDGroupTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDHandler.java \
        $(PKGDIR)/impl/xs/traversers/XSDKeyrefTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDNotationTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDSimpleTypeTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDUniqueOrKeyTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDWildcardTraverser.java \
        $(PKGDIR)/impl/xs/traversers/XSDocumentInfo.java \
        $(PKGDIR)/impl/xs/util/NSItemListImpl.java \
        $(PKGDIR)/impl/xs/util/SimpleLocator.java \
        $(PKGDIR)/impl/xs/util/ShortListImpl.java \
        $(PKGDIR)/impl/xs/util/StringListImpl.java \
        $(PKGDIR)/impl/xs/util/XInt.java \
        $(PKGDIR)/impl/xs/util/XIntPool.java \
        $(PKGDIR)/impl/xs/util/XSGrammarPool.java \
        $(PKGDIR)/impl/xs/util/XSNamedMap4Types.java \
        $(PKGDIR)/impl/xs/util/XSNamedMapImpl.java \
        $(PKGDIR)/impl/xs/util/XSObjectListImpl.java \


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
NEW_RESOURCE_BUNDLES_PROPERTIES = \
	impl/msg/XIncludeMessages.properties \
	impl/msg/DOMMessages.properties \
        impl/msg/SAXMessages.properties \
	impl/msg/XMLMessages.properties \
	impl/msg/XMLSchemaMessages.properties \
	impl/msg/XMLSerializerMessages.properties \
	impl/msg/DatatypeMessages.properties \
	impl/xpath/regex/message.properties \
	impl/xpath/regex/message_fr.properties \
	impl/xpath/regex/message_ja.properties \


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
