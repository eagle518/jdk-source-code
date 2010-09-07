#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.13 06/06/08
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../..

LIBRARY = RegUtilsMSI
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/regutilsmsi
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch
DEPLOYCLASSHDRDIR = $(OUTPUTDIR)/tmp/deploy/deployClassHeaders

# Tell Defs.gmk that we want a static link of the MS runtime libraries
MS_RUNTIME_STATIC=true

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

CPPINCPATH = $(SRCDIR) $(subst $(LIBRARY),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(DEPLOYCLASSHDRDIR)) $(DEPLOY_TOPDIR)/src/plugin/win32/common $(DEPLOY_COMMON_WIN_SRC) $(DEPLOY_TOPDIR)/src/plugin/win32/regutils
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
CPPINCPATH += $(LOCAL_ATL_FILE_PATH)
endif

RSCINC = -i $(SRCDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))
EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yu"stdafx.h" -I"$(OBJDIR)"
LDLIBS += urlmon.lib wininet.lib version.lib shell32.lib $(OBJDIR)/Msi.lib
ifeq ($(ARCH), amd64)
  LDLIBS += wintrust.lib bits.lib
endif
LFLAGS += -def:$(SRCDIR)/$(LIBRARY).def
CXXFLAGS += -I$(TEMPDIR) -Gy

JAVAFX_PRELOAD_JNLP_URL=http://dl.javafx.com/javafx-cache.jnlp

ifdef ALT_JAVAFX_PRELOAD_JNLP_URL
  JAVAFX_PRELOAD_JNLP_URL=$(ALT_JAVAFX_PRELOAD_JNLP_URL)
endif

ifdef JAVAFX_PRELOAD_JNLP_URL
  ifndef DISABLE_JAVAFX_PRELOAD
    CXXFLAGS += -DJAVAFX_PRELOAD_JNLP_URL="$(JAVAFX_PRELOAD_JNLP_URL)"
  endif
endif


ifeq ($(ARCH_DATA_MODEL), 64)
  ifneq ($(BUILD_DEPLOY_AMD64), true)
    CXXFLAGS += -DBUILD_REGUTILS_ONLY
  endif
endif

$(OBJDIR)/RegInstallMSI.obj:: copy-files

$(OBJDIR)/UserProfile.obj $(OBJDIR)/common.obj: EXTRA_CPPFLAGS :=
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
$(OBJDIR)/atlthunk.obj: EXTRA_CPPFLAGS :=
endif


$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h"

copy-files:
	$(CP) $(DEPLOY_MSSDK)/include/msi.h $(OBJDIR)/msi.h ; \
	$(CP) $(DEPLOY_MSSDK)/include/msiquery.h $(OBJDIR)/msiquery.h ; \
	$(CP) $(DEPLOY_MSSDK_LIB)/msi.lib $(OBJDIR)/msi.lib 


ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(DEPLOY_TOPDIR)/src/plugin/win32/common:$(DEPLOY_TOPDIR)/src/common/windows/native:$(DEPLOY_TOPDIR)/src/plugin/win32/regutils:$(LOCAL_ATL_FILE_PATH)
else
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(DEPLOY_TOPDIR)/src/plugin/win32/common:$(DEPLOY_TOPDIR)/src/common/windows/native:$(DEPLOY_TOPDIR)/src/plugin/win32/regutils
endif
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
FILES_cpp += atlthunk.cpp
endif

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

JAVALIB=

clobber::

build:
