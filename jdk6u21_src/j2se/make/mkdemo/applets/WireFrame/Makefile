#
# @(#)Makefile	1.9 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build applet demo.
#

BUILDDIR = ../../..
DEMONAME = WireFrame
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(DEMODST)/Matrix3D.java \
             $(DEMODST)/ThreeD.java

DEMO_dirs  = $(DEMODST) $(DEMODST)/models

DEMO_other = $(DEMODST)/example1.html \
             $(DEMODST)/example2.html \
             $(DEMODST)/example3.html \
             $(DEMODST)/example4.html \
             $(DEMODST)/models/cube.obj \
             $(DEMODST)/models/dinasaur.obj \
             $(DEMODST)/models/hughes_500.obj \
             $(DEMODST)/models/knoxS.obj

include ../DemoApplets.gmk

