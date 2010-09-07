#
# @(#)Makefile	1.2 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for the dtrace sample code
#

BUILDDIR = ../..

PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

SAMPLE_SRC_DIR = $(PLATFORM_SRC)/sample/dtrace
SAMPLE_DST_DIR = $(SAMPLEDIR)/dtrace

SAMPLE_FILES =									\
	$(SAMPLE_DST_DIR)/README.txt             				\
	$(SAMPLE_DST_DIR)/helpers/dtrace_helper.d				\
	$(SAMPLE_DST_DIR)/hotspot_jni/README.txt				\
	$(SAMPLE_DST_DIR)/hotspot_jni/CriticalSection.d				\
	$(SAMPLE_DST_DIR)/hotspot_jni/CriticalSection_slow.d			\
	$(SAMPLE_DST_DIR)/hotspot_jni/hotspot_jni_calls_stat.d			\
	$(SAMPLE_DST_DIR)/hotspot_jni/hotspot_jni_calls_tree.d			\
	$(SAMPLE_DST_DIR)/hotspot/README.txt					\
	$(SAMPLE_DST_DIR)/hotspot/class_loading_stat.d				\
	$(SAMPLE_DST_DIR)/hotspot/gc_time_stat.d				\
	$(SAMPLE_DST_DIR)/hotspot/hotspot_calls_tree.d				\
	$(SAMPLE_DST_DIR)/hotspot/method_compile_stat.d				\
	$(SAMPLE_DST_DIR)/hotspot/method_invocation_stat.d			\
	$(SAMPLE_DST_DIR)/hotspot/method_invocation_stat_filter.d		\
	$(SAMPLE_DST_DIR)/hotspot/method_invocation_tree.d			\
	$(SAMPLE_DST_DIR)/hotspot/monitors.d					\
	$(SAMPLE_DST_DIR)/hotspot/object_allocation_stat.d


all build: $(SAMPLE_FILES)

$(SAMPLE_DST_DIR)/%: $(SAMPLE_SRC_DIR)/%
	$(install-file)

clean clobber:
	$(RM) -r $(SAMPLE_DST_DIR)

.PHONY: all build clean clobber
