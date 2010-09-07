#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Clipping sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Clipping
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/Areas.java \
             $(PKGPATH)/ClipAnim.java \
             $(PKGPATH)/Intersection.java \
             $(PKGPATH)/Text.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Clipping

include ../../DemoJava2D.gmk
