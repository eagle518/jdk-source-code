# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.13 05/06/2008
#
# Makefile to build FilesInUse.exe
#

DEPLOY_TOPDIR = ../../..

EXECUTABLE = FilesInUse
PROGRAM = FilesInUse
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/filesinuse
PCHFILENAME=$(OBJDIR)/$(EXECUTABLE).pch
MS_RUNTIME_STATIC=true

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

CPPINCPATH = $(SRCDIR) $(subst $(EXECUTABLE),,$(TEMPDIR)) $(DEPLOY_TOPDIR)/src/plugin/win32/common $(DEPLOY_COMMON_WIN_SRC) 
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
CPPINCPATH += $(LOCAL_ATL_FILE_PATH)
endif

RSCINC = -i $(SRCDIR) -i $(subst $(EXECUTABLE),,$(TEMPDIR)) -i $(DEPLOY_TOPDIR)/src/plugin/win32
EXTRA_CPPFLAGS := -Fp"$(PCHFILENAME)" -Yu"stdafx.h" -I"$(OBJDIR)"
LDLIBS += comctl32.lib gdi32.lib Psapi.lib

ifeq ($(ARCH), amd64)
  LDLIBS += wintrust.lib bits.lib
endif
ifeq ($(ARCH_DATA_MODEL), 64)
  ifneq ($(BUILD_DEPLOY_AMD64), true)
    CXXFLAGS += -DBUILD_REGUTILS_ONLY
  endif
endif

$(OBJDIR)/common.obj: EXTRA_CPPFLAGS :=
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
$(OBJDIR)/atlthunk.obj: EXTRA_CPPFLAGS :=
endif

$(OBJDIR)/FilesInUse.obj::

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -Fp"$(PCHFILENAME)" -Yc"stdafx.h"

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(DEPLOY_TOPDIR)/src/plugin/win32/common:$(DEPLOY_TOPDIR)/src/common/windows/native:$(LOCAL_ATL_FILE_PATH)
else
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(DEPLOY_TOPDIR)/src/plugin/win32/common:$(DEPLOY_TOPDIR)/src/common/windows/native
endif
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
FILES_cpp += atlthunk.cpp
endif

include $(DEPLOY_TOPDIR)/make/plugin/common/exec.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

JAVALIB=

clobber::

build:
