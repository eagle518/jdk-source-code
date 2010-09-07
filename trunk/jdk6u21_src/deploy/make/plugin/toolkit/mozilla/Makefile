# @(#)Makefile	1.28 10/04/16
#
# Copyright (c) 2006, 2010 Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# Makefile to build the npDeployHelper.dll
#
# To build npDeployHelper from this directory:
#    gnumake all
#

DEPLOY_TOPDIR = ../../../..

# Tell Defs.gmk that we want a static link of the MS runtime libraries
MS_RUNTIME_STATIC=true
include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

LIBRARY = npdeployJava1

SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/toolkit/mozilla
UTILSDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/toolkit/common

ifeq ($(ARCH), amd64)
  MOZHDRDIR = $(subst \,/,$(MOZILLA_HEADERS_PATH)/mozilla_headers_18.amd64/plugin) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_18.amd64/nspr) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_18.amd64/java)
else
  MOZHDRDIR = $(subst \,/,$(MOZILLA_HEADERS_PATH)/mozilla_headers_18.win32/plugin) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_18.win32/nspr) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_18.win32/java)
endif

OBJDIRNAME = obj$(SUFFIX)
OBJOUTPUTDIR=$(OUTPUTDIR)/tmp/deploy/plugin
OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY)/$(OBJDIRNAME)
DBG_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY)/obj$(DBG_SUFFIX)
OPT_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY)/obj$(OPT_SUFFIX)
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch

include ./FileList.gmk
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
FILES_cpp += atlthunk.cpp
endif

EXTRA_CPPFLAGS := -TP -Gy -Zi -Fp"$(PCHFILENAME)" -DUSRDLL -DHW_THREADS=1 -DWINVER=0x400 -DSTDC_HEADERS=1 -DWIN32_LEAN_AND_MEAN=1 -DOJI=1
ifeq ($(ARCH), amd64)
  EXTRA_CPPFLAGS += -D_AMD64_=1
else
  EXTRA_CPPFLAGS += -D_WIN32=1 -DXP_WIN32=1 -D_X86_=1 
endif

LFLAGS += /NOLOGO -def:$(SRCDIR)/$(LIBRARY).def /SUBSYSTEM:WINDOWS /NODEFAULTLIB:nafxcw.lib -opt:noref /LIBPATH:$(DEPLOY_MSSDK)/lib/$(LIBARCH)

LDLIBS += urlmon.lib wininet.lib version.lib shell32.lib kernel32.lib user32.lib gdi32.lib  winmm.lib wsock32.lib advapi32.lib comctl32.lib wintrust.lib crypt32.lib shell32.lib shlwapi.lib

CPPINCPATH = $(MOZHDRDIR) \
             $(DEPLOY_MSSDK)/include \
             $(OUTPUTDIR)/include \
             $(TEMPDIR) \
             $(subst $(LIBRARY),,$(TEMPDIR)) \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) \
	     $(SRCDIR) \
	     $(SRCDIR)/../../common \
             $(UTILSDIR)
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
CPPINCPATH += $(LOCAL_ATL_FILE_PATH)
endif

RSCINC = -i $(SRCDIR) -i $(TEMPDIR) -i $(OBJDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))

ifeq ($(ARCH), amd64)
$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1 -D_AMD64_=1 -DUSRDLL -Fp"$(PCHFILENAME)"
else
$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1 -D_X86_=1 -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h"
endif

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
vpath %.cpp $(SRCDIR):$(UTILSDIR):$(SRCDIR)/../../common:$(LOCAL_ATL_FILE_PATH)
else
vpath %.cpp $(SRCDIR):$(UTILSDIR):$(SRCDIR)/../../common
endif

vpath %.rc $(SRCDIR)

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clobber::
	$(RM) $(OBJDIR)/*.*




