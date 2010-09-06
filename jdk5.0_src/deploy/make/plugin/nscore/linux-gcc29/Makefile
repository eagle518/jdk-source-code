#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.6 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..

LIBRARY = javaplugin_nscp_gcc29
SRCDIR = $(PLUGIN_TOP)/src/plugin/solaris/nscore
COMMONDIR=$(PLUGIN_TOP)/src/plugin/solaris/common
SHAREDDIR = $(PLUGIN_TOP)/src/plugin/share

ALT_COMPILER_PATH = $(GCC29_COMPILER_PATH)bin
include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifdef ALT_GCC29_PLUGIN_LIB_PATH
all::
	$(CP) $(ALT_GCC29_PLUGIN_LIB_PATH)/lib$(LIBRARY).so \
		$(OUTPUTDIR)/lib/$(LIBARCH)
else
LDLIBS = -lX11 -ldl -lc

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common \
             $(subst $(LIBRARY),,$(TMPDIR)) \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) \
             $(COMMONDIR) \
             $(PLUGIN_TOP)/src/plugin/share/jpiapi \
             $(SRCDIR)/../nsstub

EXTRA_CPPFLAGS	= -B $(ALT_COMPILER_PATH)/../lib/gcc-lib/ -I $(ALT_COMPILER_PATH)/../lib/gcc-lib/i386-redhat-linux/egcs-2.9*/include -DRAPTOR_API -DNEW_STREAMING_API -DXP_UNIX -DLINUX

OBJDIRNAME = obj$(SUFFIX)
OBJOUTPUTDIR = $(OUTPUTDIR)/tmp/deploy/plugin
OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY)/$(OBJDIRNAME)
DBG_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY)/obj$(DBG_SUFFIX)
OPT_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY)/obj$(OPT_SUFFIX)

vpath %.c $(SRCDIR):$(SRCDIR)/../common:
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(SHAREDDIR)/jcomponent

include ../unix/FileList.gmk 
include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

endif

clobber::
