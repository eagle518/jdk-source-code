#
# @(#)Makefile	1.23 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building various parts of javac.
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS_MAKEFLAGS += NEVER_ACT_AS_SERVER_CLASS_MACHINE=TRUE

SUBDIRS = javac recompile javadoc apt
all build clean clobber::
	$(SUBDIRS-loop)

