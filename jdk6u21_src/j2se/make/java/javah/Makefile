#
# @(#)Makefile	1.48 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# An alias for the real javah build.
#

BUILDDIR=../..
PRODUCT=java
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = ../../com/sun/javah
all build clean clobber::
	$(SUBDIRS-loop)

