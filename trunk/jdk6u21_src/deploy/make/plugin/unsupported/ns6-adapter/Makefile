#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.1 02/07/23
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../..

LIBRARY = ns6-adapter
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/src/share/ns6-adapter

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns6-adapter

dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifeq ($(PLATFORM), linux)
LDLIBS = -lXt -lX11 -ldl -lc
else
LDLIBS = -ldl -lCrun -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../adapter-common $(SRCDIR)/../jcomponent/public-include $(subst $(LIBRARY),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))
EXTRA_CPPFLAGS = -I$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610 -I$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610/nspr -I$(MOZILLA_LIBS_PATH)/../jpi/X11/include -I$(MOZILLA_LIBS_PATH)/../jpi/X11/lib/glib/include -DRAPTOR_API -DNEW_STREAMING_API

vpath %.c $(SRCDIR):$(SRCDIR)/../adapter-common:$(DEPLOY_TOPDIR)/src/plugin/oji-plugin/src/motif/common
vpath %.cpp $(SRCDIR):$(SRCDIR)/../adapter-common:$(DEPLOY_TOPDIR)/src/plugin/oji-plugin/src/motif/common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clobber::
	$(RM) -rf $(LIB_LOCATION)
