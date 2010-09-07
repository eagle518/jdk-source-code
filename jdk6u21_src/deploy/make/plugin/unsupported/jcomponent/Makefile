#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.10 10/03/24
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../..

LIBRARY = jcomponent
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/src/share/jcomponent

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

ifeq ($(PLATFORM), linux)
else
LDFLAGS += -z defs
LDLIBS = -lsocket -lnsl -ldl -lCrun -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/public-include $(SRCDIR)/../common $(subst $(LIBRARY),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))

vpath %.cpp $(SRCDIR):$(SRCDIR)/../common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

