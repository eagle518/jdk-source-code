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
DEMONAME = MoleculeViewer
include $(BUILDDIR)/common/Defs.gmk

DEMO_java  = $(DEMODST)/Matrix3D.java \
             $(DEMODST)/XYZApp.java

DEMO_dirs  = $(DEMODST) \
             $(DEMODST)/models

DEMO_other = $(DEMODST)/example1.html \
             $(DEMODST)/example2.html \
             $(DEMODST)/example3.html \
             $(DEMODST)/models/benzene.xyz \
             $(DEMODST)/models/buckminsterfullerine.xyz \
             $(DEMODST)/models/HyaluronicAcid.xyz \
             $(DEMODST)/models/cyclohexane.xyz \
             $(DEMODST)/models/ethane.xyz \
             $(DEMODST)/models/water.xyz

include ../DemoApplets.gmk

