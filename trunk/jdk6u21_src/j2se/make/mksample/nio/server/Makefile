#
# @(#)Makefile	1.5 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for the nioserver sample code
#

BUILDDIR = ../../..

PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

SAMPLE_SRC_DIR = $(SHARE_SRC)/sample/nio/server
SAMPLE_DST_DIR = $(SAMPLEDIR)/nio/server

SAMPLE_FILES =							\
	$(SAMPLE_DST_DIR)/AcceptHandler.java			\
	$(SAMPLE_DST_DIR)/Acceptor.java				\
	$(SAMPLE_DST_DIR)/B1.java				\
	$(SAMPLE_DST_DIR)/BN.java				\
	$(SAMPLE_DST_DIR)/BP.java				\
	$(SAMPLE_DST_DIR)/ChannelIO.java			\
	$(SAMPLE_DST_DIR)/ChannelIOSecure.java			\
	$(SAMPLE_DST_DIR)/Content.java				\
	$(SAMPLE_DST_DIR)/Dispatcher.java			\
	$(SAMPLE_DST_DIR)/Dispatcher1.java			\
	$(SAMPLE_DST_DIR)/DispatcherN.java			\
	$(SAMPLE_DST_DIR)/FileContent.java			\
	$(SAMPLE_DST_DIR)/Handler.java				\
	$(SAMPLE_DST_DIR)/MalformedRequestException.java	\
	$(SAMPLE_DST_DIR)/N1.java				\
	$(SAMPLE_DST_DIR)/N2.java				\
	$(SAMPLE_DST_DIR)/README.txt				\
	$(SAMPLE_DST_DIR)/Reply.java				\
	$(SAMPLE_DST_DIR)/Request.java				\
	$(SAMPLE_DST_DIR)/RequestHandler.java			\
	$(SAMPLE_DST_DIR)/RequestServicer.java			\
	$(SAMPLE_DST_DIR)/Sendable.java				\
	$(SAMPLE_DST_DIR)/Server.java				\
	$(SAMPLE_DST_DIR)/StringContent.java			\
	$(SAMPLE_DST_DIR)/URLDumper.java

all build: $(SAMPLE_FILES)

$(SAMPLE_DST_DIR)/%: $(SAMPLE_SRC_DIR)/%
	$(install-file)

clean clobber:
	$(RM) -r $(SAMPLE_DST_DIR)

.PHONY: all build clean clobber
