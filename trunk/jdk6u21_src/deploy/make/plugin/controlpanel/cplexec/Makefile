#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.17 10/03/24
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../../..

EXECUTABLE = javacpl
PROGRAM = javacplexec
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/controlpanel/cplexec
PCHFILENAME=$(OBJDIR)/$(EXECUTABLE).pch

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_ALL_MANIFEST=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'

CPPINCPATH = $(SRCDIR) $(subst $(EXECUTABLE),,$(TEMPDIR)/../) $(subst $(EXECUTABLE),,$(TEMPDIR)) $(subst $(EXECUTABLE)/,,$(CLASSHDRDIR)) $(DEPLOY_TOPDIR)/src/plugin/win32/common $(DEPLOY_COMMON_WIN_SRC)

$(OBJDIR)/UserProfile.obj:: updatefiles

RSCINC = -i $(SRCDIR) -i $(OBJDIR) -i $(subst $(EXECUTABLE),,$(TEMPDIR)) -i $(subst $(EXECUTABLE),,$(TEMPDIR)/../)
LDLIBS += user32.lib

updatefiles:
	$(CAT) $(SRCDIR)/javacpl_proto.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/javacpl.exe.manifest

vpath %.cpp $(SRCDIR):$(SRCDIR)/../../common:$(DEPLOY_COMMON_WIN_SRC)
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(DEPLOY_TOPDIR)/make/plugin/common/exec.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clobber::
	$(RM) $(OBJDIR)/*.res

