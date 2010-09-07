#
# @(#)Makefile	1.29 10/03/24
# 
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for ActiveX Win32 installer component

DEPLOY_TOPDIR = ../../..

MS_RUNTIME_STATIC=true

include $(DEPLOY_TOPDIR)/make/common/Defs.gmk

SRCDIR = $(DEPLOY_TOPDIR)/src/javaws/win32/activex
TEMPDIR = $(OUTPUTDIR)/tmp/deploy/javaws/activex
PROGNAME = wsdetect
DLL = $(BIN_DIR)/$(PROGNAME).dll
IDL = $(SRCDIR)/$(PROGNAME).idl
TLB = $(TEMPDIR)/$(PROGNAME).tlb
GENH = $(TEMPDIR)/$(PROGNAME).h
GENC = $(TEMPDIR)/$(PROGNAME)_i.c
GENCP = $(TEMPDIR)/$(PROGNAME)_p.c
DLLDATA = $(TEMPDIR)/dlldata.c

FILES_cpp = \
isInstalled.cpp \
JavaWebStart.cpp \
StdAfx.cpp 

FILES_obj = $(FILES_cpp:%.cpp=$(OBJDIR)/%.obj)

RSC_PROJ= $(RC_FLAGS) /i "$(TEMPDIR)" /i "$(GENHDIR)"
CPP_PROJ=-nologo $(MS_RUNTIME_OPTION) -W3 -O2 -D "WIN32" -D "NDEBUG" -D "_WINDOWS" -D "_MBCS" -D "_USRDLL" -D "_ATL_STATIC_REGISTRY" -I"$(TEMPDIR)" -Fo"$(OBJDIR)\\" -c
CPP_PROJ_AFX=-nologo $(MS_RUNTIME_OPTION) -W3 -O2 -D "WIN32" -D "NDEBUG" -D "_WINDOWS" -D "_MBCS" -D "_USRDLL" -D "_ATL_STATIC_REGISTRY" -I"$(TEMPDIR)" -Fp"$(TEMPDIR)\$(PROGNAME).pch" -Yc"stdafx.h" -Fo"$(OBJDIR)\\" -Fd"$(TEMPDIR)\\" -FD -c
MTL_SWITCHES=-tlb "$(TLB)" -h "$(GENH)" -iid "$(GENC)" -proxy "$(GENCP)" -dlldata "$(DLLDATA)" -Oicf

ifeq ($(ARCH), amd64)
    LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib bufferoverflowu.lib -nologo -subsystem:windows -dll -incremental:no -pdb:"$(BIN_DIR)\\wsdetect.pdb" -def:"$(SRCDIR)\\wsdetect.def" -implib:"$(BIN_DIR)\\wsdetect.lib"
else
    LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib -nologo -subsystem:windows -dll -incremental:no -pdb:"$(BIN_DIR)\\wsdetect.pdb" -def:"$(SRCDIR)\\wsdetect.def" -implib:"$(BIN_DIR)\\wsdetect.lib"
endif


RES_SRC=$(SRCDIR)/resource-activex.rc
RES_FILE=$(OBJDIR)/resource-activex.res


all build: $(PROTO_DIR) $(DLL)

$(PROTO_DIR) $(TEMPDIR) $(OBJDIR):
	$(MKDIR) -p $@

$(DLL): $(OBJDIR) $(TLB) $(FILES_obj) $(RES_FILE)
	$(LINK) $(LINK32_FLAGS) /out:"$@" $(FILES_obj) $(RES_FILE)

$(TLB): $(TEMPDIR) $(IDL)
	$(MTL) $(MTL_SWITCHES) $(IDL)

$(RES_FILE): $(OBJDIR) $(RES_SRC)
	@$(prep-target)
	$(RSC) $(RSC_PROJ) /fo"$@" $(RES_SRC)

$(OBJDIR)/StdAfx.obj: $(SRCDIR)/StdAfx.cpp
	$(CPP) $(CPP_PROJ_AFX) $?

$(OBJDIR)/%.obj: $(SRCDIR)/%.cpp
	$(CPP) $(CPP_PROJ) $?

clean:
	$(RM) -r $(TLB) $(GENH) $(GENC) $(GENCP) $(DLLDATA) $(FILES_obj) \
		$(DLL) $(RES_FILE) $(TEMPDIR)

.PHONY: all clean build
