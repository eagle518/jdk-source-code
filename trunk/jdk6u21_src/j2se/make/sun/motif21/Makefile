#
# @(#)Makefile	1.28 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
MOTIF_VERSION = 2
MOTIF_VERSION_STRING=21
LIB_LOCATION = $(LIBDIR)/$(LIBARCH)/motif$(MOTIF_VERSION_STRING)

PACKAGE = sun.awt
LIBRARY = mawt
LIBRARY_OUTPUT = motif21
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

ifdef BUILD_MOTIF21
  include $(BUILDDIR)/sun/awt/mawt.gmk
else
  all build clean clobber ::
  # do nothing case if older OS and new motif 2.1 not found in the boot dir
endif
