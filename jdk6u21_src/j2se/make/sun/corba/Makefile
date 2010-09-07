#
# @(#)Makefile	1.13 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build corba core classes that are required for rmic compiler
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = org core
all build clean clobber::
	$(SUBDIRS-loop)
	$(RM) -r $(CLASSBINDIR)/com/sun/corba/se/internal/io

