#
# @(#)Makefile	1.32 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the JDI interface and 
# front-end implementation
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.jdi
PRODUCT = jpda
include $(BUILDDIR)/common/Defs.gmk

#
#  Java Files to compile are found automatically.
#
AUTO_FILES_JAVA_DIRS = com/sun/jdi com/sun/tools/jdi

# 
# Files that need to be copied 
# 
SERVICEDIR = $(CLASSBINDIR)/META-INF/services
  
FILES_copy = \
	$(SERVICEDIR)/com.sun.jdi.connect.spi.TransportService \
	$(SERVICEDIR)/com.sun.jdi.connect.Connector

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = com/sun/tools/jdi/resources/jdi.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

# 
# Copy the service provider configuration files into the resource
# directory. Copy the configuration file also uncomments any
# providers that need to be enabled for this platform.
#

build: copy-files

copy-files: $(FILES_copy)

$(SERVICEDIR)/%: $(SHARE_SRC)/classes/com/sun/tools/jdi/META-INF/services/%
	$(prep-target)
	$(RM) $@.temp
	@$(CAT) $< | $(SED) -e "s/^#\[$(PLATFORM)\]//" > $@.temp
	$(MV) $@.temp $@

clean clobber::
	$(RM) $(FILES_copy)

.PHONY: copy-files

