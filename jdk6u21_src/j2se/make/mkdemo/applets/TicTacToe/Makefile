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
DEMONAME = TicTacToe
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(DEMODST)/TicTacToe.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/audio \
             $(DEMODST)/images

DEMO_other = $(DEMODST)/example1.html \
             $(DEMODST)/audio/beep.au \
             $(DEMODST)/audio/ding.au \
             $(DEMODST)/audio/return.au \
             $(DEMODST)/audio/yahoo1.au \
             $(DEMODST)/audio/yahoo2.au \
             $(DEMODST)/images/cross.gif \
             $(DEMODST)/images/not.gif

include ../DemoApplets.gmk

