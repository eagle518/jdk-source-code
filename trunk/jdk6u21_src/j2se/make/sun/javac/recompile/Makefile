#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for re-building any classes that might have been built with
# a bootstrap compiler.
#

BUILDDIR = ../../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS  = library new-javac
all build clean clobber::
	$(SUBDIRS-loop)

