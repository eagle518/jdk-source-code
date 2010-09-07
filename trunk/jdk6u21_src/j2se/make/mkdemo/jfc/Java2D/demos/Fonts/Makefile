#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Fonts sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Fonts
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/AttributedStr.java \
             $(PKGPATH)/Highlighting.java \
             $(PKGPATH)/Outline.java \
             $(PKGPATH)/Tree.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Fonts

include ../../DemoJava2D.gmk
