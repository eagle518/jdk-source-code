#
# @(#)Makefile	1.13 10/03/23
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
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
# LANGUAGE_VERSION = -source 1.4

#
# Files to compile
#


FILES_java = \
        $(PKGDIR)/Version.java \
        \
        $(PKGDIR)/extensions/ExpressionContext.java \
        \
        $(PKGDIR)/res/XSLTErrorResources.java \
        $(PKGDIR)/res/XSLTErrorResources_de.java \
        $(PKGDIR)/res/XSLTErrorResources_en.java \
        $(PKGDIR)/res/XSLTErrorResources_es.java \
        $(PKGDIR)/res/XSLTErrorResources_fr.java \
        $(PKGDIR)/res/XSLTErrorResources_it.java \
        $(PKGDIR)/res/XSLTErrorResources_ja.java \
        $(PKGDIR)/res/XSLTErrorResources_ko.java \
        $(PKGDIR)/res/XSLTErrorResources_sv.java \
        $(PKGDIR)/res/XSLTErrorResources_zh_CN.java \
        $(PKGDIR)/res/XSLTErrorResources_zh_TW.java \
        $(PKGDIR)/res/XSLMessages.java \
        $(PKGDIR)/lib/ExsltBase.java \
        $(PKGDIR)/lib/ExsltCommon.java \
        $(PKGDIR)/lib/ExsltDatetime.java \
        $(PKGDIR)/lib/ExsltDynamic.java \
        $(PKGDIR)/lib/ExsltMath.java \
        $(PKGDIR)/lib/ExsltSets.java \
        $(PKGDIR)/lib/ExsltStrings.java \
        $(PKGDIR)/lib/Extensions.java \
        $(PKGDIR)/lib/ObjectFactory.java \
        $(PKGDIR)/lib/SecuritySupport.java \
        $(PKGDIR)/lib/SecuritySupport12.java \
        $(PKGDIR)/templates/Constants.java \
        $(PKGDIR)/xslt/ObjectFactory.java \
        $(PKGDIR)/xslt/SecuritySupport.java \
        $(PKGDIR)/xslt/SecuritySupport12.java \
        $(PKGDIR)/xslt/Process.java \
        $(PKGDIR)/xslt/EnvironmentCheck.java 

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
