#
# @(#)Makefile	1.4 10/03/24
#
# Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# Makefile to build the ssvagent
#

DEPLOY_TOPDIR = ../../..

EXECUTABLE = ssvagent
PROGRAM = ssvagent
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/ssvagent
SSVCOMMONSRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/ssv_common
PCHFILENAME=$(OBJDIR)/$(EXECUTABLE).pch

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_ALL_MANIFEST=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'

CPPINCPATH = $(SRCDIR) $(SSVCOMMONSRCDIR) $(TEMPDIR) $(subst $(EXECUTABLE),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) $(DEPLOY_MSSDK)/include $(DEPLOY_TOPDIR)/src/plugin/win32/common $(DEPLOY_TOPDIR)/src/plugin/win32/regutils $(DEPLOY_COMMON_WIN_SRC)

RSCINC = -i $(SRCDIR) -i $(TEMPDIR) -i $(OBJDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))

EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1

LDLIBS += urlmon.lib wininet.lib shlwapi.lib version.lib comctl32.lib gdi32.lib user32.lib shell32.lib

LFLAGS += /SUBSYSTEM:WINDOWS

$(OBJDIR)/ssvagent.obj:: updatefiles

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1 -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h" 

updatefiles:
	$(CAT) $(SRCDIR)/ssvagent_proto.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/ssvagent.exe.manifest

vpath %.cpp $(SRCDIR):$(DEPLOY_COMMON_WIN_SRC)
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(DEPLOY_TOPDIR)/make/plugin/common/exec.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clean::
	$(RM) $(OBJDIR)/*

.PHONY: all clean
