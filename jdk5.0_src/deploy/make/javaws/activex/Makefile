#
# @(#)Makefile	1.20 04/05/19
# 
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for ActiveX Win32 installer component

DEPLOY_BUILDDIR = ../..
JAVAWS_TOPDIR = ../../..
DEPLOY_TOPDIR = ../../..
include $(DEPLOY_BUILDDIR)/common/Defs.gmk

SRCDIR = $(JAVAWS_TOPDIR)/src/javaws/win32/activex
TMPDIR = $(OUTPUTDIR)/tmp/deploy/javaws/activex
PROGNAME = JavaWebStart
DLL = $(BIN_DIR)/$(PROGNAME).dll
IDL = $(SRCDIR)/$(PROGNAME).idl
TLB = $(TMPDIR)/$(PROGNAME).tlb
GENH = $(TMPDIR)/$(PROGNAME).h
GENC = $(TMPDIR)/$(PROGNAME)_i.c
GENCP = $(TMPDIR)/$(PROGNAME)_p.c
DLLDATA = $(TMPDIR)/dlldata.c

FILES_cpp = \
isInstalled.cpp \
JavaWebStart.cpp \
StdAfx.cpp 

FILES_obj = $(FILES_cpp:%.cpp=$(OBJDIR)/%.obj)

RSC_PROJ= $(RC_FLAGS) /i "$(TMPDIR)" /i "$(GENHDIR)"
CPP_PROJ=-nologo -MT -W3 -O2 -D "WIN32" -D "NDEBUG" -D "_WINDOWS" -D "_MBCS" -D "_USRDLL" -D "_ATL_STATIC_REGISTRY" -I"$(TMPDIR)" -Fo"$(OBJDIR)\\" -c
CPP_PROJ_AFX=-nologo -MT -W3 -O2 -D "WIN32" -D "NDEBUG" -D "_WINDOWS" -D "_MBCS" -D "_USRDLL" -D "_ATL_STATIC_REGISTRY" -I"$(TMPDIR)" -Fp"$(TMPDIR)\$(PROGNAME).pch" -Yc"stdafx.h" -Fo"$(OBJDIR)\\" -Fd"$(TMPDIR)\\" -FD -c
MTL_SWITCHES=-tlb "$(TLB)" -h "$(GENH)" -iid "$(GENC)" -proxy "$(GENCP)" -dlldata "$(DLLDATA)" -Oicf
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib -nologo -subsystem:windows -dll -incremental:no -pdb:"$(BIN_DIR)\\JavaWebStart.pdb" -machine:I386 -def:"$(SRCDIR)\\JavaWebStart.def" -implib:"$(BIN_DIR)\\JavaWebStart.lib"



RES_SRC=$(SRCDIR)/resource-activex.rc
RES_FILE=$(OBJDIR)/resource-activex.res


all optimized debug: $(PROTO_DIR) $(DLL)

$(PROTO_DIR) $(TMPDIR) $(OBJDIR):
	$(MKDIR) -p $@

$(DLL): $(OBJDIR) $(TLB) $(FILES_obj) $(RES_FILE)
	$(LINK) $(LINK32_FLAGS) /out:"$@" $(FILES_obj) $(RES_FILE)

$(TLB): $(TMPDIR) $(IDL)
	$(MTL) $(MTL_SWITCHES) $(IDL)

$(RES_FILE): $(OBJDIR) $(RES_SRC)
	$(RSC) $(RSC_PROJ) /fo"$@" $(RES_SRC)

$(OBJDIR)/StdAfx.obj: $(SRCDIR)/StdAfx.cpp
	$(CPP) $(CPP_PROJ_AFX) $?

$(OBJDIR)/%.obj: $(SRCDIR)/%.cpp
	$(CPP) $(CPP_PROJ) $?

clean:
	$(RM) -r $(TLB) $(GENH) $(GENC) $(GENCP) $(DLLDATA) $(FILES_obj) \
		$(DLL) $(RES_FILE) $(TMPDIR)

.PHONY: all clean optimized debug
