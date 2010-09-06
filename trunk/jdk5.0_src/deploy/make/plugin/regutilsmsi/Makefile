#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.1 02/06/05
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = RegUtilsMSI
SRCDIR = $(PLUGIN_TOP)/src/plugin/win32/regutilsmsi
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch
DEPLOYCLASSHDRDIR = $(OUTPUTDIR)/tmp/deploy/deployClassHeaders

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

CPPINCPATH = $(SRCDIR) $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(DEPLOYCLASSHDRDIR)) $(PLUGIN_TOP)/src/plugin/win32/common

RSCINC = -i $(SRCDIR) -i $(subst $(LIBRARY),,$(TMPDIR))
EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yu"stdafx.h" -I"$(OBJDIR)"
LDLIBS += urlmon.lib wininet.lib version.lib shell32.lib $(OBJDIR)/Msi.lib
LFLAGS += -def:$(SRCDIR)/$(LIBRARY).def
CXXFLAGS += -I$(TMPDIR)

$(OBJDIR)/RegInstallMSI.obj:: copy-files

$(OBJDIR)/StdAfx.obj: EXTRA_CPPFLAGS := -DUSRDLL -Fp"$(PCHFILENAME)" -Yc"stdafx.h" 

copy-files:
	$(CP) $(DEPLOY_MSSDK)/include/msi.h $(OBJDIR)/msi.h ; \
	$(CP) $(DEPLOY_MSSDK)/include/msiquery.h $(OBJDIR)/msiquery.h ; \
	$(CP) $(DEPLOY_MSSDK)/lib/msi.lib $(OBJDIR)/msi.lib 


vpath %.cpp $(SRCDIR):$(SRCDIR)/../common
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::

optimized:
debug:
