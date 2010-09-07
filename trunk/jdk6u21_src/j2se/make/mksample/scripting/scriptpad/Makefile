#
# @(#)Makefile	1.3 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for the scriptpad sample code
#

BUILDDIR = ../../..

PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

SAMPLE_SRC_DIR = $(SHARE_SRC)/sample/scripting/scriptpad
SAMPLE_DST_DIR = $(SAMPLEDIR)/scripting/scriptpad

SAMPLE_FILES =									\
	$(SAMPLE_DST_DIR)/build.xml						\
	$(SAMPLE_DST_DIR)/build.properties					\
	$(SAMPLE_DST_DIR)/README.txt             				\
	$(SAMPLE_DST_DIR)/nbproject/file-targets.xml				\
	$(SAMPLE_DST_DIR)/nbproject/jdk.xml					\
	$(SAMPLE_DST_DIR)/nbproject/netbeans-targets.xml			\
	$(SAMPLE_DST_DIR)/nbproject/project.xml					\
	$(SAMPLE_DST_DIR)/src/com/sun/sample/scriptpad/Main.java		\
	$(SAMPLE_DST_DIR)/src/META-INF/manifest.mf				\
	$(SAMPLE_DST_DIR)/src/resources/conc.js					\
	$(SAMPLE_DST_DIR)/src/resources/gui.js					\
	$(SAMPLE_DST_DIR)/src/resources/Main.js					\
	$(SAMPLE_DST_DIR)/src/resources/mm.js					\
	$(SAMPLE_DST_DIR)/src/resources/scriptpad.js				\
	$(SAMPLE_DST_DIR)/src/scripts/browse.js					\
	$(SAMPLE_DST_DIR)/src/scripts/insertfile.js				\
	$(SAMPLE_DST_DIR)/src/scripts/linewrap.js				\
	$(SAMPLE_DST_DIR)/src/scripts/mail.js					\
	$(SAMPLE_DST_DIR)/src/scripts/memmonitor.js				\
	$(SAMPLE_DST_DIR)/src/scripts/memory.bat				\
	$(SAMPLE_DST_DIR)/src/scripts/memory.sh					\
	$(SAMPLE_DST_DIR)/src/scripts/memory.js					\
	$(SAMPLE_DST_DIR)/src/scripts/README.txt				\
	$(SAMPLE_DST_DIR)/src/scripts/textcolor.js


all build: $(SAMPLE_FILES)

$(SAMPLE_DST_DIR)/%: $(SAMPLE_SRC_DIR)/%
	$(install-file)

clean clobber:
	$(RM) -r $(SAMPLE_DST_DIR)

.PHONY: all build clean clobber
