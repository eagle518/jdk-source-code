#
# @(#)Makefile	1.21 02/08/02
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.21 02/08/02
#
# Makefile for building com.sun.org.apache.xerces.internal.xinclude
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
        $(PKGDIR)/xinclude/ObjectFactory.java \
        $(PKGDIR)/xinclude/SecuritySupport.java \
        $(PKGDIR)/xinclude/MultipleScopeNamespaceSupport.java \
        $(PKGDIR)/xinclude/XIncludeHandler.java \
        $(PKGDIR)/xinclude/XIncludeMessageFormatter.java \
        $(PKGDIR)/xinclude/XIncludeNamespaceSupport.java \
        $(PKGDIR)/xinclude/XIncludeTextReader.java \
        $(PKGDIR)/xinclude/XPointerElementHandler.java \
        $(PKGDIR)/xinclude/XPointerFramework.java \
        $(PKGDIR)/xinclude/XPointerSchema.java \
        $(PKGDIR)/xinclude/XInclude11TextReader.java \



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
