#
# @(#)Makefile	1.12 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Mix sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Mix
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/Balls.java \
             $(PKGPATH)/BezierScroller.java \
             $(PKGPATH)/JavaMedia.java \
             $(PKGPATH)/Stars3D.java


DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Mix

include ../../DemoJava2D.gmk
