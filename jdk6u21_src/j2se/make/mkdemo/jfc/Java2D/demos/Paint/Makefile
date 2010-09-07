#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build Java2D demos/Paint sources
#

BUILDDIR = ../../../../..
DEMONAME = demos/Paint
PKGPATH = $(DEMODST)/$(DEMONAME)
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(PKGPATH)/GradAnim.java \
             $(PKGPATH)/Gradient.java \
             $(PKGPATH)/Texture.java \
             $(PKGPATH)/TextureAnim.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/java2d/demos \
             $(DEMODST)/java2d/demos/Paint

include ../../DemoJava2D.gmk

