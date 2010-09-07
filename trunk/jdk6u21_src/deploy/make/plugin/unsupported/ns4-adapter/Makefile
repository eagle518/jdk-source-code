#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.10 10/03/24
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../..

LIBRARY = ns4-adapter
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/src/share/ns4-adapter

include $(DEPLOY_TOPDIR)/src/plugin/common/Defs.gmk

LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns4-adapter

dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifeq ($(PLATFORM), linux)
LDLIBS = -lXt -lX11 -ldl -lnsl -lc
else
LDLIBS = -lXt -lX11 -ldl -lnsl -lCrun -lc
LDFLAGS += -z defs
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../adapter-common $(SRCDIR)/../jcomponent/public-include $(DEPLOY_TOPDIR)/oji-plugin/src/motif/common $(DEPLOY_TOPDIR)/src/plugin/oji-plugin/src/motif/common $(DEPLOY_TOPDIR)/src/plugin/oji-plugin/src/motif/badapter $(subst $(LIBRARY),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))  

vpath %.c $(SRCDIR):$(SRCDIR)/../adapter-common:$(DEPLOY_TOPDIR)/src/plugin/oji-plugin/src/motif/common
vpath %.cpp $(SRCDIR):$(SRCDIR)/../adapter-common:$(DEPLOY_TOPDIR)/src/plugin/oji-plugin/src/motif/common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clobber::
	$(RM) -rf $(LIB_LOCATION)

