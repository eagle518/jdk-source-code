#
# @(#)Makefile	1.30 05/04/05
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building packages under javax.xml
#

BUILDDIR = ../..
PACKAGE = javax.activation
PRODUCT = jaf
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = javax/activation com/sun/activation

# Copy registry files into META-INF
MASTER_COPY_DIR = $(SHARE_SRC)/lib/activation
DESTINATION_DIR = $(CLASSBINDIR)/META-INF/

# List of resulting files we want created
FILES_copy = \
	$(DESTINATION_DIR)/mailcap.default \
	$(DESTINATION_DIR)/mimetypes.default

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

# Make sure the default build rule includes this copy action
build: copy-files

# Handy copy action rule name
copy-files: $(FILES_copy)

# Pattern copy rule that just copies dependency to target
$(DESTINATION_DIR)/%: $(MASTER_COPY_DIR)/%
	$(install-file)

# Mark this rule as not being a file at all, just a target name
.PHONY: copy-files

