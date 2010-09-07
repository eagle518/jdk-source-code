#
# @(#)Makefile	1.18 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the demo applets
#

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = Animator ArcTest BarChart Blink CardTest Clock DitherTest DrawTest \
          Fractal GraphLayout GraphicsTest ImageMap JumpingBox MoleculeViewer \
          NervousText SimpleGraph SortDemo SpreadSheet TicTacToe WireFrame
all build clean clobber::
	$(SUBDIRS-loop)

clobber clean::
	$(RM) -r $(DEMODIR)/applets
