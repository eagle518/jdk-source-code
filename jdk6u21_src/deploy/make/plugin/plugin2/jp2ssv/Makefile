#
# @(#)Makefile	1.6 10/03/24
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

DEPLOY_TOPDIR = ../../../..

LIBRARY         = jp2ssv

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

PLUGINSRCDIR_PD = $(DEPLOY_TOPDIR)/src/plugin/win32
SRCDIR_PD       = $(PLUGINSRCDIR_PD)/plugin2/$(LIBRARY)
PLUGINCOMMONDIR_PD = $(DEPLOY_TOPDIR)/src/plugin/win32/common
SSVCOMMONSRCDIR = $(PLUGINSRCDIR_PD)/ssv_common

# TEMPDIR/.. is for the generated PluginVersion.h
CPPINCPATH = $(SRCDIR_PD)                             \
             $(DEPLOY_COMMON_WIN_SRC)                 \
             $(PLUGINCOMMONDIR_PD)                    \
             $(SSVCOMMONSRCDIR)                       \
             $(TEMPDIR)/..                            \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR))
ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
CPPINCPATH += $(LOCAL_ATL_FILE_PATH)
endif

RSCINC = -i $(TEMPDIR) -i $(SSVCOMMONSRCDIR) -i $(OBJDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))

LFLAGS += -def:$(SRCDIR_PD)/$(LIBRARY).def -implib:"$(OBJDIR)/$(LIBRARY).lib"

LDLIBS += shell32.lib

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h"

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
vpath %.cpp $(SRCDIR_PD)		\
            $(DEPLOY_COMMON_WIN_SRC)	\
            $(PLUGINCOMMONDIR_PD)	\
            $(SSVCOMMONSRCDIR)          \
            $(LOCAL_ATL_FILE_PATH)
else
vpath %.cpp $(SRCDIR_PD)		\
            $(DEPLOY_COMMON_WIN_SRC)	\
            $(PLUGINCOMMONDIR_PD)	\
            $(SSVCOMMONSRCDIR) 
endif
vpath %.c   $(PLUGINSRCDIR_PD)/common
vpath %.rc  $(SRCDIR_PD)

include ./FileList.gmk 

ifeq ($(USE_LOCAL_COPY_OF_ATL), true)
FILES_cpp += atlthunk.cpp
endif

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clean:: clobber

clobber::
	$(RM) $(OBJDIR)/*.obj
