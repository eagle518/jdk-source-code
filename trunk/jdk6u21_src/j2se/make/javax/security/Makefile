#
# @(#)Makefile	1.5 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
# 
# Makefile for javax.security packages.

BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = auth sasl
all build clean clobber::
	$(SUBDIRS-loop)

clean clobber::
	$(RM) -r $(CLASSDESTDIR)/javax/security
