#
# @(#)Makefile	1.27 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of java
#

BUILDDIR = ../../../..
PRODUCT = se
include $(BUILDDIR)/common/Defs.gmk

# IDL code generation is moved out to an extra step before the
# actual classes are compiled
# the files should be compiled in the following order, core to be built
# before connection

# Note that sources generates the logging wrappers, and pept compiles
# them, because pept is the earliest build target that can use
# the log wrappers.  Note that rmi is moved here after pept because
# the rmic iiop backend needs some classes that are part of the impl/util
# package.  These classes use log wrappers, so they must be built after
# pept and before rmi.
SUBDIRS = sources pept corespi core spi impl rmi
all build clean clobber::
	$(SUBDIRS-loop)

