#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.9 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..

EXECUTABLE = javacpl
PROGRAM = javacplexec
SRCDIR = $(PLUGIN_TOP)/src/plugin/win32/controlpanel/cplexec
PCHFILENAME=$(OBJDIR)/$(EXECUTABLE).pch

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

CPPINCPATH = $(SRCDIR) $(subst $(EXECUTABLE),,$(TMPDIR)/../) $(subst $(EXECUTABLE),,$(TMPDIR)) $(subst $(EXECUTABLE)/,,$(CLASSHDRDIR)) $(PLUGIN_TOP)/src/plugin/win32/common

RSCINC = -i $(SRCDIR) -i $(subst $(EXECUTABLE),,$(TMPDIR)) -i $(subst $(EXECUTABLE),,$(TMPDIR)/../)
LDLIBS += user32.lib


vpath %.cpp $(SRCDIR):$(SRCDIR)/../../common
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(PLUGIN_TOP)/make/plugin/common/exec.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::
	$(RM) $(OPT_OBJDIR)/*.res $(DBG_OBJDIR)/*.res

