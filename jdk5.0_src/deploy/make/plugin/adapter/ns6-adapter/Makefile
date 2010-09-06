#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.7 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..
LIBRARY_OUTPUT = adapter/ns6
SRCDIR = $(PLUGIN_TOP)/src/plugin/share/adapter/ns6
ADAPTERCOMMONDIR = $(PLUGIN_TOP)/src/plugin/share/adapter/common

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

OBJDIRNAME = obj$(SUFFIX)
OBJOUTPUTDIR=$(OUTPUTDIR)/tmp/deploy/plugin
OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/$(OBJDIRNAME)
DBG_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/obj$(DBG_SUFFIX)
OPT_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/obj$(OPT_SUFFIX)

include ./FileList.gmk 

ifeq ($(PLATFORM), windows) 

LIBRARY = jpins6
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch
JNIDIR = $(OUTPUTDIR)/include/win32
COREDIR = $(PLUGIN_TOP)/src/plugin/win32/nscore
COMMONDIR = $(PLUGIN_TOP)/src/plugin/win32/common
MOZHDRDIR = $(subst \,/,$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610.win32) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610.win32/nspr)

FILES_cpp += DllMain.cpp \
	     CNS6Adapter.cpp \
	     clientLoadCOM.cpp \
	     oji_clsid.cpp

EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -YX"stdafx.h"
LFLAGS += -def:$(SRCDIR)/$(LIBRARY).def
$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h"

else

LIBRARY = javaplugin_oji
LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns610
dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifeq ($(PLATFORM), linux)
LDLIBS = -lX11 -ldl -lc
else
LDLIBS = -lX11 -ldl -lCrun -lc
endif

JNIDIR = $(OUTPUTDIR)/include/$(PLATFORM)
COREDIR = $(PLUGIN_TOP)/src/plugin/solaris/nscore
COMMONDIR=  $(PLUGIN_TOP)/src/plugin/solaris/common
MOZHDRDIR = $(subst \,/,$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_ns610/nspr)

FILES_cpp += GetFactory5.cpp \
	     GetLibrary.cpp \
	     CNSAdapter_NSPR.cpp
EXTRA_CPPFLAGS := -DNS6_ADAPTER	
endif

INTERFACESDIR = $(PLUGIN_TOP)/src/plugin/share/jpiapi
ADAPTERINCDIR = $(PLUGIN_TOP)/src/plugin/share/adapter/include

CPPINCPATH = $(OUTPUTDIR)/include $(JNIDIR) \
	     $(subst $(LIBRARY_OUTPUT),,$(TMPDIR)) \
	     $(COMMONDIR) \
	     $(SRCDIR) \
	     $(MOZHDRDIR) \
	     $(ADAPTERCOMMONDIR) \
	     $(ADAPTERINCDIR) \
	     $(COREDIR) \
	     $(INTERFACESDIR) 

vpath %.c $(ADAPTERCOMMONDIR)
vpath %.cpp $(SRCDIR):$(ADAPTERCOMMONDIR):$(COMMONDIR)

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::
ifneq ($(PLATFORM), windows)
	$(RM) -rf $(LIB_LOCATION)
endif
