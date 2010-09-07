#
# @(#)Makefile	1.14 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
HEADLESS = true
NO_ROBOT = true
MOTIF_VERSION = none
MOTIF_VERSION_STRING=none
LIB_LOCATION = $(LIBDIR)/$(LIBARCH)/headless

PACKAGE = sun.awt
LIBRARY = mawt
LIBRARY_OUTPUT = headless
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

ifdef BUILD_HEADLESS
  include $(BUILDDIR)/sun/awt/mawt.gmk
else
  all build clean clobber ::
  # do nothing case
endif

