#
# @(#)Makefile	1.19 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JDWP
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.jdwpgen
PRODUCT = jpda
include $(BUILDDIR)/common/Defs.gmk
include $(BUILDDIR)/common/Rules.gmk

#
# Where do generated Java source files go?
#
JAVA_SRC_DEST_DIR = $(GENSRCDIR)

#
# Where do generated C source files go?
# the jdwp build is what needs the generated C files
# so they are being placed there for automatic pickup by
# the value of CLASSHDRDIR when jdwp is being built.
#
C_SRC_DEST_DIR = $(CLASSHDRDIR)/../jdwp

#
# Rules.
#
TARGPKGDIR = com/sun/tools/jdi
GEN_JDWP_JAVA_DIR = $(JAVA_SRC_DEST_DIR)/$(TARGPKGDIR)
GEN_JDWP_JAVA = $(GEN_JDWP_JAVA_DIR)/JDWP.java
GEN_JDWP_C = $(C_SRC_DEST_DIR)/JDWPCommands.h

#DOC_DIR = $(BUILDDIR)/doc
#API_DOC_DIR = $(DOC_DIR)/api
#GEN_JDWP_DOC_DIR =  $(API_DOC_DIR)

JDWP_SPEC = $(SHARE_SRC)/classes/$(PKGDIR)/jdwp.spec

build: $(GEN_JDWP_JAVA) $(GEN_JDWP_C)

$(GEN_JDWP_JAVA) $(GEN_JDWP_C): $(JDWP_SPEC)
	$(MKDIR) -p $(GEN_JDWP_JAVA_DIR)
	$(MKDIR) -p $(C_SRC_DEST_DIR)
#	$(MKDIR) -p $(GEN_JDWP_DOC_DIR)
	$(JAVA) -classpath $(CLASSBINDIR) $(PACKAGE).Main $(JDWP_SPEC) \
		-jdi $(GEN_JDWP_JAVA) \
		-include $(GEN_JDWP_C)
	@$(java-vm-cleanup)

# no docs in regular build ... -doc $(GEN_JDWP_DOC_DIR)/jdwp-protocol.html

#
# Extra clean.
#
clean clobber::
	$(RM) -r $(C_SRC_DEST_DIR)
	$(RM) -r $(GEN_JDWP_JAVA_DIR)
#	$(RM) -r $(GEN_JDWP_DOC_DIR)

