#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.13 10/03/24
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../../..

LIBRARY = javaplugin_nscp_gcc29
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/solaris/nscore
COMMONDIR=$(DEPLOY_TOPDIR)/src/plugin/solaris/common
SHAREDDIR = $(DEPLOY_TOPDIR)/src/plugin/share

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

COMPILER_PATH = $(GCC29_COMPILER_PATH)bin/

ifdef ALT_GCC29_PLUGIN_LIB_PATH
all::
	$(CP) $(ALT_GCC29_PLUGIN_LIB_PATH)/lib$(LIBRARY).so \
		$(OUTPUTDIR)/lib/$(LIBARCH)
else
LDLIBS = -lX11 -ldl -lc

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common \
             $(subst $(LIBRARY),,$(TEMPDIR)) \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) \
             $(COMMONDIR) \
             $(DEPLOY_TOPDIR)/src/plugin/share/jpiapi \
             $(SRCDIR)/../nsstub

EXTRA_CPPFLAGS	= -B $(COMPILER_PATH)/../lib/gcc-lib/ -I $(COMPILER_PATH)/../lib/gcc-lib/i386-redhat-linux/egcs-2.9*/include -DRAPTOR_API -DNEW_STREAMING_API -DXP_UNIX -DLINUX

OBJDIRNAME = obj
OBJOUTPUTDIR = $(OUTPUTDIR)/tmp/deploy/plugin
OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY)/$(OBJDIRNAME)

vpath %.c $(SRCDIR):$(SRCDIR)/../common:
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common:$(SHAREDDIR)/jcomponent

include ../unix/FileList.gmk 
include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

endif

clobber::
