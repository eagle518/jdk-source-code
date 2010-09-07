#
# @(#)Makefile	1.45 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the jfc demos
#

BUILDDIR = ../..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

SWING_DEMOS = SwingSet2 FileChooserDemo Metalworks \
          Notepad Stylepad SampleTree SwingApplet TableExample \
          Laffy SwingSet3

OTHER_DEMOS = Java2D Font2DTest CodePointIM

SUBDIRS = $(SWING_DEMOS) $(OTHER_DEMOS)
all build clean clobber::
	$(SUBDIRS-loop)

OTHERSUBDIRS=$(SWING_DEMOS)
OTHERSUBDIRS_MAKEFLAGS=SWING_DEMODIR=demo/jfc
swing-demo ::
	$(OTHERSUBDIRS-loop)

