#
# @(#)Makefile	1.9 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of java
#

BUILDDIR = ../../../..
PRODUCT = plugin
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = dom net
all build clean clobber::
	$(SUBDIRS-loop)

