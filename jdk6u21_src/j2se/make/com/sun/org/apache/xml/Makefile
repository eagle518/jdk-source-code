#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.0 02/10/23
#
# Makefile for building com.sun.org.apache.xml
#

BUILDDIR = ../../../../..
PACKAGE = com.sun.org.apache.xml
PRODUCT = xml
include $(BUILDDIR)/common/Defs.gmk

#
# Take out enums flag until source code is updated
#
# LANGUAGE_VERSION = -source 1.4

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/org/apache/xml


# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Resources
#
# RESOURCE_FILES = \
#OTHER_PROPERTIES = \
LOCALE_SET_DEFINITION = j2re
NEW_RESOURCE_BUNDLES_PROPERTIES = \
   internal/serialize/HTMLEntities.res \
   internal/serializer/Encodings.properties \
   internal/serializer/HTMLEntities.properties \
   internal/serializer/output_html.properties \
   internal/serializer/output_text.properties \
   internal/serializer/output_unknown.properties \
   internal/serializer/output_xml.properties \
   internal/serializer/XMLEntities.properties \
   internal/security/resource/config.dtd \
   internal/security/resource/config.xml \
   internal/security/resource/xmlsecurity_de.properties \
   internal/security/resource/xmlsecurity_en.properties \

RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)


## Xalan is using a special ".res" extension for entities.
$(RESOURCE_DEST_DIR)/%.res: \
	$(RESOURCE_SRC_DIR)/%.res
	$(install-file)

$(RESOURCE_DEST_DIR)/internal/security/resource/config.dtd: \
	$(RESOURCE_SRC_DIR)/internal/security/resource/config.dtd
	$(install-file)

$(RESOURCE_DEST_DIR)/internal/security/resource/config.xml: \
	$(RESOURCE_SRC_DIR)/internal/security/resource/config.xml
	$(install-file)

include $(BUILDDIR)/common/Resources.gmk
