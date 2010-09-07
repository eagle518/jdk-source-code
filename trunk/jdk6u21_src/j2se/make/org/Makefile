#
# @(#)Makefile	1.31 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of java
#

BUILDDIR = ..
PRODUCT = org
include $(BUILDDIR)/common/Defs.gmk

ifdef STANDALONE_CORBA_WS
SUBDIRS = omg se
else
SUBDIRS = omg xml w3c ietf jcp
endif
all build clean clobber::
	$(SUBDIRS-loop)

