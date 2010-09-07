#
# @(#)Makefile	1.7 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../../../../..
PRODUCT = impl

include $(BUILDDIR)/common/Defs.gmk

# order is important

SUBDIRS = poa toa
all build clean clobber::
	$(SUBDIRS-loop)

