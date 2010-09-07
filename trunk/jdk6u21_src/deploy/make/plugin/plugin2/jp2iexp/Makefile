#
# @(#)Makefile	1.7 07/12/05
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

DEPLOY_TOPDIR = ../../../..

LIBRARY            = jp2iexp
SRCDIR_PD          = $(DEPLOY_TOPDIR)/src/plugin/win32/plugin2/$(LIBRARY)
PLUGINSRCDIR_PD    = $(DEPLOY_TOPDIR)/src/plugin/win32
PLUGIN2SRCDIR      = $(DEPLOY_TOPDIR)/src/plugin/share/plugin2

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

# TEMPDIR/.. is for the generated PluginVersion.h
CPPINCPATH = $(SRCDIR_PD)                             \
             $(PLUGINSRCDIR_PD)/plugin2/common        \
             $(PLUGINSRCDIR_PD)/common                \
             $(PLUGINSRCDIR_PD)/eula                  \
             $(PLUGIN2SRCDIR)/common                  \
             $(DEPLOY_COMMON_WIN_SRC)                 \
             $(TEMPDIR)/..                            \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR))
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
CPPINCPATH += $(LOCAL_ATL_FILE_PATH)
endif

RSCINC = -i $(SRCDIR_PD) -i $(TEMPDIR) -i $(OBJDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))

LDLIBS += shell32.lib

LFLAGS += -def:$(SRCDIR_PD)/$(LIBRARY).def -implib:"$(OBJDIR)/$(LIBRARY).lib"

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h"

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
vpath %.cpp $(SRCDIR_PD)              \
            $(PLUGINSRCDIR_PD)/common \
            $(PLUGINSRCDIR_PD)/plugin2/common \
            $(PLUGINSRCDIR_PD)/eula   \
            $(PLUGIN2SRCDIR)/common   \
            $(DEPLOY_COMMON_WIN_SRC)  \
            $(LOCAL_ATL_FILE_PATH)
else
vpath %.cpp $(SRCDIR_PD)              \
            $(PLUGINSRCDIR_PD)/common \
            $(PLUGINSRCDIR_PD)/plugin2/common \
            $(PLUGINSRCDIR_PD)/eula   \
            $(PLUGIN2SRCDIR)/common   \
            $(DEPLOY_COMMON_WIN_SRC) 
endif
vpath %.c   $(PLUGINSRCDIR_PD)/common \
            $(PLUGINSRCDIR_PD)/plugin2/common
vpath %.rc $(SRCDIR_PD)

include ./FileList.gmk 

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
FILES_cpp += atlthunk.cpp
endif

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clean:: clobber

clobber::
	$(RM) $(OBJDIR)/*.obj
