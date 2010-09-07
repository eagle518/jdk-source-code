#
# @(#)Makefile	1.12 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.util.logging
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
AUTO_FILES_JAVA_DIRS = java/util/logging

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/util/logging/resources/logging.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes

include $(BUILDDIR)/common/Resources.gmk
include $(BUILDDIR)/common/Classes.gmk

properties: $(LIBDIR)/logging.properties

$(LIBDIR)/logging.properties: $(SHARE_SRC)/lib/logging.properties
	$(install-file)

build: properties

