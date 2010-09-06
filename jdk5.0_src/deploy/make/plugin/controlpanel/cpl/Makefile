#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.8 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..

LIBRARY = jpicpl32
SRCDIR = $(PLUGIN_TOP)/src/plugin/win32/controlpanel/cpl
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

LIBRARY_SUFFIX = cpl

CPPINCPATH = $(SRCDIR) $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) $(PLUGIN_TOP)/src/plugin/win32/common

RSCINC = -i $(SRCDIR) -i $(subst $(LIBRARY),,$(TMPDIR))
LDLIBS += user32.lib shell32.lib
LFLAGS += -def:$(SRCDIR)/javacpl.def

vpath %.cpp $(SRCDIR):$(SRCDIR)/../../common
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::
	$(RM) $(OPT_OBJDIR)/*.res $(DBG_OBJDIR)/*.res
