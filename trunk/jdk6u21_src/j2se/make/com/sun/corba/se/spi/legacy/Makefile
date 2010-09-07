#
# @(#)Makefile	1.7 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of java
#

BUILDDIR = ../../../../../..
PRODUCT = legacy

include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = connection interceptor
all build clean clobber::
	$(SUBDIRS-loop)

