#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.8 04/05/27
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..
LIBRARY_OUTPUT = adapter/ns7
SRCDIR = $(PLUGIN_TOP)/src/plugin/share/adapter/ns7
ADAPTERCOMMONDIR = $(PLUGIN_TOP)/src/plugin/share/adapter/common

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

OBJDIRNAME = obj$(SUFFIX)
OBJOUTPUTDIR=$(OUTPUTDIR)/tmp/deploy/plugin
OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/$(OBJDIRNAME)
DBG_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/obj$(DBG_SUFFIX)
OPT_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/obj$(OPT_SUFFIX)

include ./FileList.gmk

ifeq ($(PLATFORM), windows)

LIBRARY = jpioji
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch
JNIDIR = $(OUTPUTDIR)/include/win32
COREDIR = $(PLUGIN_TOP)/src/plugin/win32/nscore
COMMONDIR = $(PLUGIN_TOP)/src/plugin/win32/common
MOZHDRDIR = $(subst \,/,$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns7.win32) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_ns7.win32/nspr)

FILES_cpp += DllMain.cpp \
	     CNS7Adapter.cpp \
	     clientLoadCOM.cpp \
	     oji_clsid.cpp
EXTRA_CPPFLAGS := -DUSRDLL -DXP_WIN32 -Fp"$(PCHFILENAME)" -YX"stdafx.h"
LFLAGS += -def:$(SRCDIR)/$(LIBRARY).def
$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -DXP_WIN32 -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h"

else

LIBRARY = javaplugin_oji
LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns7
dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifeq ($(PLATFORM), linux)
LDLIBS = -lX11 -ldl -lc
else
LDLIBS = -lX11 -ldl -lCrun -lc
endif

JNIDIR = $(OUTPUTDIR)/include/$(PLATFORM)
COREDIR = $(PLUGIN_TOP)/src/plugin/solaris/nscore
COMMONDIR=  $(PLUGIN_TOP)/src/plugin/solaris/common
MOZHDRDIR = $(subst \,/,$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns7) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_ns7/nspr)

FILES_cpp += GetFactory5.cpp \
	     GetLibrary.cpp \
	     CNSAdapter_NSPR.cpp
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

RSCINC = -i $(SRCDIR) -i $(subst $(LIBRARY),,$(TMPDIR))

vpath %.c $(ADAPTERCOMMONDIR)
vpath %.cpp $(SRCDIR):$(ADAPTERCOMMONDIR):$(COMMONDIR)
vpath %.rc $(SRCDIR)

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::
ifneq ($(PLATFORM), windows)
	$(RM) -rf $(LIB_LOCATION)
else
	$(RM) $(OPT_OBJDIR)/*.res $(DBG_OBJDIR)/*.res
endif
