#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# 
# Makefile for com.sun.security packages.
#

BUILDDIR = ../../..
include $(BUILDDIR)/common/Defs.gmk

SUBDIRS = jgss sasl auth auth/module
all build clean clobber::
	$(SUBDIRS-loop)

clean clobber::
	$(RM) -r $(CLASSDESTDIR)/com/sun/security
