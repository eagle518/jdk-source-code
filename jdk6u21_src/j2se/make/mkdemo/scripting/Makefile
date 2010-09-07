#
# @(#)Makefile	1.2 10/03/23 16:59:11
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the scripting demo(s)
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = jconsole-plugin

all build clean clobber::
	$(SUBDIRS-loop)

all build :: $(ABS_OUTPUTDIR)/demo/scripting
