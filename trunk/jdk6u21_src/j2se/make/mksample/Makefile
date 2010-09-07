#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all the samples
#

BUILDDIR = ..
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = nio jmx scripting javac webservices nbproject

# include dtrace samples in Solaris builds
ifeq ($(PLATFORM), solaris) 
	SUBDIRS += dtrace
endif

all build clean clobber::
	$(SUBDIRS-loop)

clobber clean ::
	$(RM) -r $(SAMPLEDIR)
