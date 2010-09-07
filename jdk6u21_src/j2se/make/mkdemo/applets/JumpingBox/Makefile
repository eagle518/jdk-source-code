#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build applet demo.
#

BUILDDIR = ../../..
DEMONAME = JumpingBox
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(DEMODST)/JumpingBox.java 

DEMO_dirs  = $(DEMODST) $(DEMODST)/sounds

DEMO_other = $(DEMODST)/example1.html \
             $(DEMODST)/sounds/adapt-or-die.au \
             $(DEMODST)/sounds/ip.au \
             $(DEMODST)/sounds/that.hurts.au \
             $(DEMODST)/sounds/cannot.be.completed.au \
             $(DEMODST)/sounds/thin.bell.au \
             $(DEMODST)/sounds/danger.au \
             $(DEMODST)/sounds/tiptoe.thru.the.tulips.au

include ../DemoApplets.gmk

