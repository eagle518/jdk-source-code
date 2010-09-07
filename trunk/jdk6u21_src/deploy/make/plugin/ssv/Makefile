#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# Makefile to build the ssv
#
#

DEPLOY_TOPDIR = ../../..

LIBRARY = ssv

SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/ssv
COMMONSRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/ssv_common
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

IDL = $(SRCDIR)/$(LIBRARY).idl
TLB = $(TEMPDIR)/$(LIBRARY).tlb
GENH = $(TEMPDIR)/$(LIBRARY).h
GENC = $(TEMPDIR)/$(LIBRARY)_i.c
GENCP = $(TEMPDIR)/$(LIBRARY)_p.c
DLLDATA = $(TEMPDIR)/dlldata.c

CPPINCPATH = $(SRCDIR) $(COMMONSRCDIR) $(TEMPDIR) $(subst $(LIBRARY),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) $(DEPLOY_MSSDK)/include $(DEPLOY_TOPDIR)/src/plugin/win32/common $(DEPLOY_TOPDIR)/src/plugin/win32/regutils $(DEPLOY_COMMON_WIN_SRC)
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
CPPINCPATH += $(LOCAL_ATL_FILE_PATH)
endif

RSCINC = -i $(SRCDIR) -i $(COMMONSRCDIR) -i $(TEMPDIR) -i $(OBJDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))

EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1

ifeq ($(ARCH), amd64)
LDLIBS += urlmon.lib wininet.lib shlwapi.lib version.lib comctl32.lib gdi32.lib shell32.lib bufferoverflowu.lib
else
LDLIBS += urlmon.lib wininet.lib shlwapi.lib version.lib comctl32.lib gdi32.lib
endif

LFLAGS += -def:$(SRCDIR)/$(LIBRARY).def -implib:"$(OBJDIR)/$(LIBRARY).lib" $(OBJDIR)/*.res /LIBPATH:$(DEPLOY_MSSDK)/liba /machine:$(LIBARCH)

MTL_SWITCHES=-tlb "$(TLB)" -h "$(GENH)" -iid "$(GENC)" -proxy "$(GENCP)" -dlldata "$(DLLDATA)" -Oicf

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1 -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h" 

vpath %.idl $(SRCDIR)
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
vpath %.cpp $(SRCDIR):$(COMMONSRCDIR):$(SRCDIR)/../common:$(SRCDIR)/../regutils:$(DEPLOY_COMMON_WIN_SRC):$(LOCAL_ATL_FILE_PATH)
else
vpath %.cpp $(SRCDIR):$(COMMONSRCDIR):$(SRCDIR)/../common:$(SRCDIR)/../regutils:$(DEPLOY_COMMON_WIN_SRC)
endif
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
FILES_cpp += atlthunk.cpp
endif

all optimized debug: $(TLB)

$(TLB):: 
	@$(prep-target)
	$(MTL) $(MTL_SWITCHES) $(IDL)

include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk
include $(J2SE_TOPDIR)/make/common/Library.gmk

clobber::
	$(RM) $(TEMPDIR)/*/*.res $(TEMPDIR)/*/*.pch $(TEMPDIR)/*/*.idb $(TEMPDIR)/*.h $(TEMPDIR)/*.c $(TEMPDIR)/*.tlb
