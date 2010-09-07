#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.4 05/01/26
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../../..
LIBRARY_OUTPUT = adapter/ns7-gcc29
SHAREDIR = $(DEPLOY_TOPDIR)/src/plugin/share
SRCDIR = $(SHAREDIR)/adapter/oji
ADAPTERCOMMONDIR = $(SHAREDIR)/adapter/common

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

COMPILER_PATH = $(GCC29_COMPILER_PATH)bin/

LIBRARY = javaplugin_oji
LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns7-gcc29
dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifdef ALT_GCC29_PLUGIN_LIB_PATH
all::
	$(CP) $(ALT_GCC29_PLUGIN_LIB_PATH)/ns7/lib$(LIBRARY).so \
		$(LIB_LOCATION)/
else
OBJDIRNAME = obj
OBJOUTPUTDIR=$(OUTPUTDIR)/tmp/deploy/plugin
OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/$(OBJDIRNAME)

LDLIBS = -lX11 -ldl -lc
EXTRA_CPPFLAGS += -B $(COMPILER_PATH)/../lib/gcc-lib/ -I $(COMPILER_PATH)/../lib/gcc-lib/i386-redhat-linux/egcs-2.9*/include

JNIDIR = $(OUTPUTDIR)/include/$(PLATFORM)
COREDIR = $(DEPLOY_TOPDIR)/src/plugin/solaris/nscore
COMMONDIR=  $(DEPLOY_TOPDIR)/src/plugin/solaris/common
# Force this evaluation now so later CPPINCPATH sees list not shell command
MOZHDRDIRS := $(shell $(ECHO) $(subst \,/,$(MOZILLA_HEADERS_PATH))/mozilla_headers_18/*)

include ../oji-adapter/FileList.gmk 

FILES_cpp += GetFactory5.cpp \
	     GetLibrary.cpp \
	     CNSAdapter_NSPR.cpp

INTERFACESDIR = $(DEPLOY_TOPDIR)/src/plugin/share/jpiapi
ADAPTERINCDIR = $(DEPLOY_TOPDIR)/src/plugin/share/adapter/include

CPPINCPATH = $(OUTPUTDIR)/include $(JNIDIR) \
	     $(subst $(LIBRARY_OUTPUT),,$(TEMPDIR)) \
	     $(COMMONDIR) \
	     $(SRCDIR) \
	     $(MOZHDRDIRS) \
	     $(ADAPTERCOMMONDIR) \
	     $(SHAREDIR)/jpiapi \
	     $(SHAREDIR)/adapter/include \
	     $(COREDIR)

vpath %.c $(ADAPTERCOMMONDIR)
vpath %.cpp $(SRCDIR):$(ADAPTERCOMMONDIR):$(COMMONDIR)

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk
endif

clobber::
	$(RM) -rf $(LIB_LOCATION)
