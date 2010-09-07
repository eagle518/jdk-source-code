#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JNDI RMI registry service provider
#

BUILDDIR = ../../../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = registry
all build clean clobber::
	$(SUBDIRS-loop)

clean clobber::
	$(RM) -r $(CLASSDESTDIR)/com/sun/jndi/rmi
