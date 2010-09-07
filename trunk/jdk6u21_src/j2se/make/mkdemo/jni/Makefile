#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the jni demo(s)
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS =

ifeq ($(PLATFORM),solaris)
  SUBDIRS += Poller
endif

all build clean clobber::
	$(SUBDIRS-loop)

