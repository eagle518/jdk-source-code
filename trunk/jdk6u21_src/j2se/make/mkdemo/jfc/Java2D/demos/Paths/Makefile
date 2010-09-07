#
# @(#)Makefile	1.13 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Paths sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Paths
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/Append.java \
             $(PKGPATH)/CurveQuadTo.java \
             $(PKGPATH)/FillStroke.java \
             $(PKGPATH)/WindingRule.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Paths

include ../../DemoJava2D.gmk

