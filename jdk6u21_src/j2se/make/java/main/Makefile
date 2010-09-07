#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building java command line tools
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

ifeq ($(PLATFORM), windows) 
SUBDIRS = java javaw
else 
SUBDIRS = java
endif 
all build clean clobber::
	$(SUBDIRS-loop)

