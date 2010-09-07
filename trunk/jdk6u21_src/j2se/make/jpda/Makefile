#
# @(#)Makefile	1.56 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of JPDA, including JDWP, JDI, and
# example debuggers. See also included files.
#

BUILDDIR = ..
include $(BUILDDIR)/common/Defs.gmk

#
# Core.
#
SUBDIRS    = jdwpgen jdwp back front transport expr tty bdi gui
all build clean clobber::
	$(SUBDIRS-loop)

