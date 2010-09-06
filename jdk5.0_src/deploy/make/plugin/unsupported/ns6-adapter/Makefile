#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.1 02/07/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = ns6-adapter
SRCDIR = $(PLUGIN_TOP)/src/plugin/src/share/ns6-adapter

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns6-adapter

dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifeq ($(PLATFORM), linux)
LDLIBS = -lXt -lX11 -ldl -lc
else
LDLIBS = -ldl -lCrun -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../adapter-common $(SRCDIR)/../jcomponent/public-include $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))
EXTRA_CPPFLAGS = -I$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610 -I$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610/nspr -I$(MOZILLA_LIBS_PATH)/../jpi/X11/include -I$(MOZILLA_LIBS_PATH)/../jpi/X11/lib/glib/include -DRAPTOR_API -DNEW_STREAMING_API

vpath %.c $(SRCDIR):$(SRCDIR)/../adapter-common:$(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/common
vpath %.cpp $(SRCDIR):$(SRCDIR)/../adapter-common:$(PLUGIN_TOP)/src/plugin/oji-plugin/src/motif/common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::
	$(RM) -rf $(LIB_LOCATION)
