#
# @(#)Makefile	1.4 06/03/27
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for the EbayClient sample code
#

BUILDDIR = ../../..

PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

SAMPLE_SRC_DIR = $(SHARE_SRC)/sample/webservices/EbayClient
SAMPLE_DST_DIR = $(SAMPLEDIR)/webservices/EbayClient

SAMPLE_FILES =							\
	$(SAMPLE_DST_DIR)/src/ebay/client/MainGUIApp.java	\
	$(SAMPLE_DST_DIR)/src/ebay/client/ViewItemDetails.java	\
	$(SAMPLE_DST_DIR)/src/ebay/client/handler/RequesterCredentials.java \
	$(SAMPLE_DST_DIR)/build.xml \
	$(SAMPLE_DST_DIR)/build.properties \
	$(SAMPLE_DST_DIR)/eBaySvc.wsdl \
	$(SAMPLE_DST_DIR)/ebay.properties \
	$(SAMPLE_DST_DIR)/jax-ws-catalog.xml \
	$(SAMPLE_DST_DIR)/manifest.mf \
	$(SAMPLE_DST_DIR)/index.html \
	$(SAMPLE_DST_DIR)/nbproject/file-targets.xml \
	$(SAMPLE_DST_DIR)/nbproject/project.xml \
	$(SAMPLE_DST_DIR)/nbproject/jdk.xml \
	$(SAMPLE_DST_DIR)/nbproject/netbeans-targets.xml \

all build: $(SAMPLE_FILES)

$(SAMPLE_DST_DIR)/%: $(SAMPLE_SRC_DIR)/%
	$(install-file)

clean clobber:
	$(RM) -r $(SAMPLE_DST_DIR)

.PHONY: all build clean clobber
