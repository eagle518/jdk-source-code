#
# @(#)Makefile	1.34 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of org.omg.*
#

BUILDDIR = ../..
PACKAGE = org.omg
PRODUCT = org
include $(BUILDDIR)/common/Defs.gmk

ifdef STANDALONE_CORBA_WS
# idl compiler needs to be built before any other idl files can be compiled
SUBDIRS = idl sources PortableServer CosTransactions CosTSPortability CORBA CosNaming DynamicAny PortableInterceptor
else
SUBDIRS = idl sources PortableServer CORBA CosNaming DynamicAny PortableInterceptor
endif
all build clean clobber::
	$(SUBDIRS-loop)

