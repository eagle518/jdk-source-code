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

BUILDDIR = ../../../../../../..
PACKAGE = com.sun.org.apache.xalan.internal
PRODUCT = xalan
include $(BUILDDIR)/common/Defs.gmk

#
# Take out enums flag until source code is updated
#
#LANGUAGE_VERSION = -source 1.4

#
#       $(PKGDIR)/xsltc/trax/SmartTransformerFactoryImpl.java \
# Files to compile
#

FILES_java = \
        $(PKGDIR)/xsltc/DOMEnhancedForDTM.java \
        $(PKGDIR)/xsltc/CollatorFactory.java \
        $(PKGDIR)/xsltc/DOM.java \
        $(PKGDIR)/xsltc/DOMCache.java \
        $(PKGDIR)/xsltc/NodeIterator.java \
        $(PKGDIR)/xsltc/ProcessorVersion.java \
        $(PKGDIR)/xsltc/StripFilter.java \
        $(PKGDIR)/xsltc/Translet.java \
        $(PKGDIR)/xsltc/TransletException.java \
        $(PKGDIR)/xsltc/cmdline/ObjectFactory.java \
        $(PKGDIR)/xsltc/cmdline/SecuritySupport.java \
        $(PKGDIR)/xsltc/cmdline/SecuritySupport12.java \
        $(PKGDIR)/xsltc/cmdline/Compile.java \
        $(PKGDIR)/xsltc/cmdline/Transform.java \
        $(PKGDIR)/xsltc/cmdline/getopt/GetOpt.java \
        $(PKGDIR)/xsltc/cmdline/getopt/GetOpt.java \
        $(PKGDIR)/xsltc/cmdline/getopt/GetOptsException.java \
        $(PKGDIR)/xsltc/cmdline/getopt/IllegalArgumentException.java \
        $(PKGDIR)/xsltc/cmdline/getopt/MissingOptArgException.java \
        $(PKGDIR)/xsltc/cmdline/getopt/MissingOptArgException.java \
        $(PKGDIR)/xsltc/compiler/ObjectFactory.java \
        $(PKGDIR)/xsltc/compiler/SecuritySupport.java \
        $(PKGDIR)/xsltc/compiler/SecuritySupport12.java \
        $(PKGDIR)/xsltc/compiler/AbsoluteLocationPath.java \
        $(PKGDIR)/xsltc/compiler/AbsolutePathPattern.java \
        $(PKGDIR)/xsltc/compiler/AlternativePattern.java \
        $(PKGDIR)/xsltc/compiler/AncestorPattern.java \
        $(PKGDIR)/xsltc/compiler/ApplyImports.java \
        $(PKGDIR)/xsltc/compiler/ApplyTemplates.java \
        $(PKGDIR)/xsltc/compiler/ArgumentList.java \
        $(PKGDIR)/xsltc/compiler/Attribute.java \
        $(PKGDIR)/xsltc/compiler/AttributeSet.java \
        $(PKGDIR)/xsltc/compiler/AttributeValue.java \
        $(PKGDIR)/xsltc/compiler/AttributeValueTemplate.java \
        $(PKGDIR)/xsltc/compiler/BinOpExpr.java \
        $(PKGDIR)/xsltc/compiler/BooleanCall.java \
        $(PKGDIR)/xsltc/compiler/BooleanExpr.java \
        $(PKGDIR)/xsltc/compiler/CallTemplate.java \
        $(PKGDIR)/xsltc/compiler/CastCall.java \
        $(PKGDIR)/xsltc/compiler/CastExpr.java \
        $(PKGDIR)/xsltc/compiler/CeilingCall.java \
        $(PKGDIR)/xsltc/compiler/Choose.java \
        $(PKGDIR)/xsltc/compiler/Closure.java \
        $(PKGDIR)/xsltc/compiler/Comment.java \
        $(PKGDIR)/xsltc/compiler/CompilerException.java \
        $(PKGDIR)/xsltc/compiler/ConcatCall.java \
        $(PKGDIR)/xsltc/compiler/Constants.java \
        $(PKGDIR)/xsltc/compiler/ContainsCall.java \
        $(PKGDIR)/xsltc/compiler/Copy.java \
        $(PKGDIR)/xsltc/compiler/CopyOf.java \
        $(PKGDIR)/xsltc/compiler/CurrentCall.java \
        $(PKGDIR)/xsltc/compiler/DecimalFormatting.java \
        $(PKGDIR)/xsltc/compiler/DocumentCall.java \
        $(PKGDIR)/xsltc/compiler/ElementAvailableCall.java \
        $(PKGDIR)/xsltc/compiler/EqualityExpr.java \
        $(PKGDIR)/xsltc/compiler/Expression.java \
        $(PKGDIR)/xsltc/compiler/Fallback.java \
        $(PKGDIR)/xsltc/compiler/FilterExpr.java \
        $(PKGDIR)/xsltc/compiler/FilterParentPath.java \
        $(PKGDIR)/xsltc/compiler/FilteredAbsoluteLocationPath.java \
        $(PKGDIR)/xsltc/compiler/FloorCall.java \
        $(PKGDIR)/xsltc/compiler/FlowList.java \
        $(PKGDIR)/xsltc/compiler/ForEach.java \
        $(PKGDIR)/xsltc/compiler/FormatNumberCall.java \
        $(PKGDIR)/xsltc/compiler/FunctionAvailableCall.java \
        $(PKGDIR)/xsltc/compiler/FunctionCall.java \
        $(PKGDIR)/xsltc/compiler/GenerateIdCall.java \
        $(PKGDIR)/xsltc/compiler/IdKeyPattern.java \
        $(PKGDIR)/xsltc/compiler/IdPattern.java \
        $(PKGDIR)/xsltc/compiler/If.java \
        $(PKGDIR)/xsltc/compiler/IllegalCharException.java \
        $(PKGDIR)/xsltc/compiler/Import.java \
        $(PKGDIR)/xsltc/compiler/Include.java \
        $(PKGDIR)/xsltc/compiler/Instruction.java \
        $(PKGDIR)/xsltc/compiler/IntExpr.java \
        $(PKGDIR)/xsltc/compiler/Key.java \
        $(PKGDIR)/xsltc/compiler/KeyCall.java \
        $(PKGDIR)/xsltc/compiler/KeyPattern.java \
        $(PKGDIR)/xsltc/compiler/LangCall.java \
        $(PKGDIR)/xsltc/compiler/LastCall.java \
        $(PKGDIR)/xsltc/compiler/LiteralAttribute.java \
        $(PKGDIR)/xsltc/compiler/LiteralElement.java \
        $(PKGDIR)/xsltc/compiler/LiteralExpr.java \
        $(PKGDIR)/xsltc/compiler/LocalNameCall.java \
        $(PKGDIR)/xsltc/compiler/LocationPathPattern.java \
        $(PKGDIR)/xsltc/compiler/LogicalExpr.java \
        $(PKGDIR)/xsltc/compiler/Message.java \
        $(PKGDIR)/xsltc/compiler/Mode.java \
        $(PKGDIR)/xsltc/compiler/NameBase.java \
        $(PKGDIR)/xsltc/compiler/NameCall.java \
        $(PKGDIR)/xsltc/compiler/NamespaceAlias.java \
        $(PKGDIR)/xsltc/compiler/NamespaceUriCall.java \
        $(PKGDIR)/xsltc/compiler/NodeTest.java \
        $(PKGDIR)/xsltc/compiler/NotCall.java \
        $(PKGDIR)/xsltc/compiler/Number.java \
        $(PKGDIR)/xsltc/compiler/NumberCall.java \
        $(PKGDIR)/xsltc/compiler/Otherwise.java \
        $(PKGDIR)/xsltc/compiler/Output.java \
        $(PKGDIR)/xsltc/compiler/Param.java \
        $(PKGDIR)/xsltc/compiler/ParameterRef.java \
        $(PKGDIR)/xsltc/compiler/ParentLocationPath.java \
        $(PKGDIR)/xsltc/compiler/Parser.java \
        $(PKGDIR)/xsltc/compiler/Pattern.java \
        $(PKGDIR)/xsltc/compiler/PositionCall.java \
        $(PKGDIR)/xsltc/compiler/Predicate.java \
        $(PKGDIR)/xsltc/compiler/ProcessingInstruction.java \
        $(PKGDIR)/xsltc/compiler/ProcessingInstructionPattern.java \
        $(PKGDIR)/xsltc/compiler/QName.java \
        $(PKGDIR)/xsltc/compiler/RealExpr.java \
        $(PKGDIR)/xsltc/compiler/RelationalExpr.java \
        $(PKGDIR)/xsltc/compiler/RelativeLocationPath.java \
        $(PKGDIR)/xsltc/compiler/RelativePathPattern.java \
        $(PKGDIR)/xsltc/compiler/RoundCall.java \
        $(PKGDIR)/xsltc/compiler/SimpleAttributeValue.java \
        $(PKGDIR)/xsltc/compiler/Sort.java \
        $(PKGDIR)/xsltc/compiler/SourceLoader.java \
        $(PKGDIR)/xsltc/compiler/StartsWithCall.java \
        $(PKGDIR)/xsltc/compiler/Step.java \
        $(PKGDIR)/xsltc/compiler/StepPattern.java \
        $(PKGDIR)/xsltc/compiler/StringCall.java \
        $(PKGDIR)/xsltc/compiler/StringLengthCall.java \
        $(PKGDIR)/xsltc/compiler/Stylesheet.java \
        $(PKGDIR)/xsltc/compiler/SymbolTable.java \
        $(PKGDIR)/xsltc/compiler/SyntaxTreeNode.java \
        $(PKGDIR)/xsltc/compiler/Template.java \
        $(PKGDIR)/xsltc/compiler/TestSeq.java \
        $(PKGDIR)/xsltc/compiler/Text.java \
        $(PKGDIR)/xsltc/compiler/TopLevelElement.java \
        $(PKGDIR)/xsltc/compiler/TransletOutput.java \
        $(PKGDIR)/xsltc/compiler/UnaryOpExpr.java \
        $(PKGDIR)/xsltc/compiler/UnionPathExpr.java \
        $(PKGDIR)/xsltc/compiler/UnparsedEntityUriCall.java \
        $(PKGDIR)/xsltc/compiler/UnresolvedRef.java \
        $(PKGDIR)/xsltc/compiler/UnsupportedElement.java \
        $(PKGDIR)/xsltc/compiler/UseAttributeSets.java \
        $(PKGDIR)/xsltc/compiler/ValueOf.java \
        $(PKGDIR)/xsltc/compiler/Variable.java \
        $(PKGDIR)/xsltc/compiler/VariableBase.java \
        $(PKGDIR)/xsltc/compiler/VariableRef.java \
        $(PKGDIR)/xsltc/compiler/VariableRefBase.java \
        $(PKGDIR)/xsltc/compiler/When.java \
        $(PKGDIR)/xsltc/compiler/Whitespace.java \
        $(PKGDIR)/xsltc/compiler/WithParam.java \
        $(PKGDIR)/xsltc/compiler/XPathLexer.java \
        $(PKGDIR)/xsltc/compiler/XPathParser.java \
        $(PKGDIR)/xsltc/compiler/XSLTC.java \
        $(PKGDIR)/xsltc/compiler/XslAttribute.java \
        $(PKGDIR)/xsltc/compiler/XslElement.java \
        $(PKGDIR)/xsltc/compiler/util/ObjectFactory.java \
        $(PKGDIR)/xsltc/compiler/util/SecuritySupport.java \
        $(PKGDIR)/xsltc/compiler/util/SecuritySupport12.java \
        $(PKGDIR)/xsltc/compiler/util/AttributeSetMethodGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/BooleanType.java \
        $(PKGDIR)/xsltc/compiler/util/ClassGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_ca.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_cs.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_de.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_es.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_fr.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_it.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_ja.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_ko.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_sk.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_zh_CN.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMessages_zh_TW.java \
        $(PKGDIR)/xsltc/compiler/util/ErrorMsg.java \
        $(PKGDIR)/xsltc/compiler/util/FilterGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/IntType.java \
        $(PKGDIR)/xsltc/compiler/util/MatchGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/MethodGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/MethodType.java \
        $(PKGDIR)/xsltc/compiler/util/MultiHashtable.java \
        $(PKGDIR)/xsltc/compiler/util/NamedMethodGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/NodeCounterGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/NodeSetType.java \
        $(PKGDIR)/xsltc/compiler/util/NodeSortRecordFactGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/NodeSortRecordGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/NodeType.java \
        $(PKGDIR)/xsltc/compiler/util/NumberType.java \
        $(PKGDIR)/xsltc/compiler/util/ObjectType.java \
        $(PKGDIR)/xsltc/compiler/util/RealType.java \
        $(PKGDIR)/xsltc/compiler/util/ReferenceType.java \
        $(PKGDIR)/xsltc/compiler/util/ResultTreeType.java \
        $(PKGDIR)/xsltc/compiler/util/RtMethodGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/SlotAllocator.java \
        $(PKGDIR)/xsltc/compiler/util/StringStack.java \
        $(PKGDIR)/xsltc/compiler/util/StringType.java \
        $(PKGDIR)/xsltc/compiler/util/TestGenerator.java \
        $(PKGDIR)/xsltc/compiler/util/Type.java \
        $(PKGDIR)/xsltc/compiler/util/TypeCheckError.java \
        $(PKGDIR)/xsltc/compiler/util/Util.java \
        $(PKGDIR)/xsltc/compiler/util/VoidType.java \
        $(PKGDIR)/xsltc/dom/SortSettings.java \
        $(PKGDIR)/xsltc/dom/ObjectFactory.java \
        $(PKGDIR)/xsltc/dom/SecuritySupport.java \
        $(PKGDIR)/xsltc/dom/SecuritySupport12.java \
        $(PKGDIR)/xsltc/dom/AbsoluteIterator.java \
        $(PKGDIR)/xsltc/dom/AdaptiveResultTreeImpl.java \
        $(PKGDIR)/xsltc/dom/AnyNodeCounter.java \
        $(PKGDIR)/xsltc/dom/BitArray.java \
        $(PKGDIR)/xsltc/dom/CachedNodeListIterator.java \
        $(PKGDIR)/xsltc/dom/ClonedNodeListIterator.java \
        $(PKGDIR)/xsltc/dom/CollatorFactoryBase.java \
        $(PKGDIR)/xsltc/dom/CurrentNodeListFilter.java \
        $(PKGDIR)/xsltc/dom/CurrentNodeListIterator.java \
        $(PKGDIR)/xsltc/dom/DOMAdapter.java \
        $(PKGDIR)/xsltc/dom/DOMBuilder.java \
        $(PKGDIR)/xsltc/dom/DOMWSFilter.java \
        $(PKGDIR)/xsltc/dom/DocumentCache.java \
        $(PKGDIR)/xsltc/dom/DupFilterIterator.java \
        $(PKGDIR)/xsltc/dom/EmptyFilter.java \
        $(PKGDIR)/xsltc/dom/ExtendedSAX.java \
        $(PKGDIR)/xsltc/dom/Filter.java \
        $(PKGDIR)/xsltc/dom/FilterIterator.java \
        $(PKGDIR)/xsltc/dom/FilteredStepIterator.java \
        $(PKGDIR)/xsltc/dom/ForwardPositionIterator.java \
        $(PKGDIR)/xsltc/dom/KeyIndex.java \
        $(PKGDIR)/xsltc/dom/LoadDocument.java \
        $(PKGDIR)/xsltc/dom/MatchingIterator.java \
        $(PKGDIR)/xsltc/dom/MultiDOM.java \
        $(PKGDIR)/xsltc/dom/MultipleNodeCounter.java \
        $(PKGDIR)/xsltc/dom/NodeCounter.java \
        $(PKGDIR)/xsltc/dom/NodeIteratorBase.java \
        $(PKGDIR)/xsltc/dom/NodeSortRecord.java \
        $(PKGDIR)/xsltc/dom/NodeSortRecordFactory.java \
        $(PKGDIR)/xsltc/dom/NthIterator.java \
        $(PKGDIR)/xsltc/dom/SAXImpl.java \
        $(PKGDIR)/xsltc/dom/SimpleResultTreeImpl.java \
        $(PKGDIR)/xsltc/dom/SingleNodeCounter.java \
        $(PKGDIR)/xsltc/dom/SingletonIterator.java \
        $(PKGDIR)/xsltc/dom/SortingIterator.java \
        $(PKGDIR)/xsltc/dom/StepIterator.java \
        $(PKGDIR)/xsltc/dom/StripWhitespaceFilter.java \
        $(PKGDIR)/xsltc/dom/UnionIterator.java \
        $(PKGDIR)/xsltc/dom/XSLTCDTMManager.java \
        $(PKGDIR)/xsltc/runtime/CallFunction.java \
        $(PKGDIR)/xsltc/runtime/ObjectFactory.java \
        $(PKGDIR)/xsltc/runtime/SecuritySupport.java \
        $(PKGDIR)/xsltc/runtime/SecuritySupport12.java \
        $(PKGDIR)/xsltc/runtime/AbstractTranslet.java \
        $(PKGDIR)/xsltc/runtime/AttributeList.java \
        $(PKGDIR)/xsltc/runtime/Attributes.java \
        $(PKGDIR)/xsltc/runtime/BasisLibrary.java \
        $(PKGDIR)/xsltc/runtime/Constants.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_ca.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_cs.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_de.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_es.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_fr.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_it.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_ja.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_ko.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_sk.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_zh_CN.java \
        $(PKGDIR)/xsltc/runtime/ErrorMessages_zh_TW.java \
        $(PKGDIR)/xsltc/runtime/Hashtable.java \
        $(PKGDIR)/xsltc/runtime/MessageHandler.java \
        $(PKGDIR)/xsltc/runtime/Node.java \
        $(PKGDIR)/xsltc/runtime/Operators.java \
        $(PKGDIR)/xsltc/runtime/Parameter.java \
        $(PKGDIR)/xsltc/runtime/StringValueHandler.java \
        $(PKGDIR)/xsltc/runtime/output/OutputBuffer.java \
        $(PKGDIR)/xsltc/runtime/output/StringOutputBuffer.java \
        $(PKGDIR)/xsltc/runtime/output/TransletOutputHandlerFactory.java \
        $(PKGDIR)/xsltc/runtime/output/WriterOutputBuffer.java \
        $(PKGDIR)/xsltc/trax/ObjectFactory.java \
        $(PKGDIR)/xsltc/trax/SecuritySupport.java \
        $(PKGDIR)/xsltc/trax/SecuritySupport12.java \
        $(PKGDIR)/xsltc/trax/DOM2SAX.java \
        $(PKGDIR)/xsltc/trax/DOM2TO.java \
        $(PKGDIR)/xsltc/trax/OutputSettings.java \
        $(PKGDIR)/xsltc/trax/SAX2DOM.java \
        $(PKGDIR)/xsltc/trax/TemplatesHandlerImpl.java \
        $(PKGDIR)/xsltc/trax/TemplatesImpl.java \
        $(PKGDIR)/xsltc/trax/TrAXFilter.java \
        $(PKGDIR)/xsltc/trax/TransformerFactoryImpl.java \
        $(PKGDIR)/xsltc/trax/TransformerHandlerImpl.java \
        $(PKGDIR)/xsltc/trax/TransformerImpl.java \
        $(PKGDIR)/xsltc/trax/Util.java \
        $(PKGDIR)/xsltc/trax/XSLTCSource.java \
        $(PKGDIR)/xsltc/util/IntegerArray.java \
        $(PKGDIR)/xsltc/util/JavaCupRedirect.java \
# Rule
#
include $(BUILDDIR)/common/Classes.gmk

## Messages are in $(PKGDIR)/res/XSLTErrorResources.java 

#
# Resources
#
# RESOURCE_FILES = \
#OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
NEW_RESOURCE_BUNDLES_PROPERTIES = \
   res/XSLTInfo.properties \


RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res:        $(RESOURCE_SRC_DIR)/%.res
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
