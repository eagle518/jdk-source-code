#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.5 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = ns4-adapter
SRCDIR = $(PLUGIN_TOP)/src/plugin/src/share/ns4-adapter

include $(PLUGIN_TOP)/src/plugin/common/config.gmk

LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns4-adapter

dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifeq ($(PLATFORM), linux)
LDLIBS = -lXt -lX11 -ldl -lnsl -lc
else
LDLIBS = -lXt -lX11 -ldl -lnsl -lCrun -lc
LDFLAGS += -z defs
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../adapter-common $(SRCDIR)/../jcomponent/public-include $(PLUGIN_TOP)/oji-plugin/src/motif/common $(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/common $(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/badapter $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))  

vpath %.c $(SRCDIR):$(SRCDIR)/../adapter-common:$(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/common
vpath %.cpp $(SRCDIR):$(SRCDIR)/../adapter-common:$(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::
	$(RM) -rf $(LIB_LOCATION)

