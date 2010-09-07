#
# @(#)Makefile	1.3 10/03/24
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

DEPLOY_TOPDIR = ../../../..

EXECUTABLE = jp2launcher
PROGRAM = jp2launcher

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

PLUGINSRCDIR_PD = $(DEPLOY_TOPDIR)/src/plugin/win32
SRCDIR_PD       = $(PLUGINSRCDIR_PD)/plugin2/$(PROGRAM)

CPPINCPATH = $(SRCDIR_PD) \
             $(DEPLOY_COMMON_WIN_SRC)

IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_ALL_MANIFEST=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'

RSCINC = -i $(OBJDIR)

updatefiles:
	$(CAT) $(SRCDIR_PD)/jp2launcher_proto.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/jp2launcher.exe.manifest

$(OBJDIR)/JP2Launcher.obj:: updatefiles

LFLAGS += /SUBSYSTEM:WINDOWS

vpath %.cpp $(SRCDIR_PD) \
            $(DEPLOY_COMMON_WIN_SRC)
vpath %.rc  $(SRCDIR_PD)

include ./FileList.gmk 

include $(DEPLOY_TOPDIR)/make/plugin/common/exec.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clean:: clobber

clobber::
	$(RM) $(OBJDIR)/*.obj
