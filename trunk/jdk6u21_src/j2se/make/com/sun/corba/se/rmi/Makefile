#
# @(#)Makefile	1.45 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build rmi tools, and sun.rmi.* implementation classes

BUILDDIR = ../../../../..

include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS = 
else
TOOLS_SUBDIRS = rmic
endif

#
# Order is important -- you can't build stubs without building rmic first.
#
ifdef STANDALONE_CORBA_WS
SUBDIRS = rmic
else
SUBDIRS = $(TOOLS_SUBDIRS) 

endif # STANDALONE_CORBA_WS

all build clean clobber::
	$(SUBDIRS-loop)

clean clobber::
	$(RM) -r $(CLASSBINDIR)/sun/rmi $(CLASSBINDIR)/com/sun/rmi
