#
# @(#)Makefile	1.4 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all the samples under the nio subdirectory.
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = server
all build clean clobber::
	$(SUBDIRS-loop)

clobber clean ::
	$(RM) -r $(SAMPLEDIR)/nio
