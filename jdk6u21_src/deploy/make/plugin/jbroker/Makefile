#
# @(#)Makefile	1.4 08/01/22
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# Makefile to build the jbroker
#

DEPLOY_TOPDIR = ../../..

EXECUTABLE = jbroker
PROGRAM = jbroker
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/jbroker
PCHFILENAME=$(OBJDIR)/$(EXECUTABLE).pch

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_ALL_MANIFEST=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'

CPPINCPATH = $(SRCDIR) $(TEMPDIR) $(subst $(EXECUTABLE),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) $(DEPLOY_MSSDK)/include $(DEPLOY_TOPDIR)/src/plugin/win32/regutils $(DEPLOY_COMMON_WIN_SRC)
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
CPPINCPATH += $(LOCAL_ATL_FILE_PATH)
endif

RSCINC = -i $(DEPLOY_MSSDK)/include -i $(SRCDIR) -i $(TEMPDIR) -i $(OBJDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))

# JAVADOTVERSION value must match the Java system property "java.version"
EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1 /DJAVADOTVERSION=$(RELEASE)

LDLIBS += urlmon.lib wininet.lib shlwapi.lib version.lib comctl32.lib gdi32.lib shell32.lib

$(OBJDIR)/jbroker.obj:: updatefiles

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := $(GX_OPTION) -DWIN32_LEAN_AND_MEAN=1 -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h" 
$(OBJDIR)/maps.obj: EXTRA_CPPFLAGS += -EHsc

updatefiles:
	$(CAT) $(SRCDIR)/jbroker_proto.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/jbroker.exe.manifest

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(SRCDIR)/../regutils:$(DEPLOY_COMMON_WIN_SRC):$(LOCAL_ATL_FILE_PATH)
else
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(SRCDIR)/../regutils:$(DEPLOY_COMMON_WIN_SRC)
endif

vpath %.rc $(SRCDIR)

include ./FileList.gmk 

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
FILES_cpp += atlthunk.cpp
endif

include $(DEPLOY_TOPDIR)/make/plugin/common/exec.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clean::
	$(RM) $(OBJDIR)/*

.PHONY: all clean
