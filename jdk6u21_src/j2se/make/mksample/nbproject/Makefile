#
# @(#)Makefile	1.4 06/03/27
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for the nbproject sample code
#

BUILDDIR = ../..

PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

SAMPLE_SRC_DIR = $(SHARE_SRC)/sample/nbproject
SAMPLE_DST_DIR = $(SAMPLEDIR)/nbproject

SAMPLE_FILES =							\
	$(SAMPLE_DST_DIR)/project.xml \

all build: $(SAMPLE_FILES)

$(SAMPLE_DST_DIR)/%: $(SAMPLE_SRC_DIR)/%
	$(install-file)

clean clobber:
	$(RM) -r $(SAMPLE_DST_DIR)

.PHONY: all build clean clobber
