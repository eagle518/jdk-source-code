#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build HPI (Host Porting Interface) libraries
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

#
# Build specified the HPI implementations
#
SUBDIRS = $(HPIS)
all build clean clobber::
	$(SUBDIRS-loop)

