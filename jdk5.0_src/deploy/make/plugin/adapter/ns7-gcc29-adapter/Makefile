#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.3 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..
LIBRARY_OUTPUT = adapter/ns7-gcc29
SHAREDIR = $(PLUGIN_TOP)/src/plugin/share
SRCDIR = $(SHAREDIR)/adapter/ns7
ADAPTERCOMMONDIR = $(SHAREDIR)/adapter/common

ALT_COMPILER_PATH = $(GCC29_COMPILER_PATH)bin
include $(PLUGIN_TOP)/make/plugin/common/config.gmk

LIBRARY = javaplugin_oji
LIB_LOCATION=$(OUTPUTDIR)/plugin/$(LIBARCH)/ns7-gcc29
dummy:=$(shell $(MKDIR) -p $(LIB_LOCATION))

ifdef ALT_GCC29_PLUGIN_LIB_PATH
all::
	$(CP) $(ALT_GCC29_PLUGIN_LIB_PATH)/ns7/lib$(LIBRARY).so \
		$(LIB_LOCATION)/
else
OBJDIRNAME = obj$(SUFFIX)
OBJOUTPUTDIR=$(OUTPUTDIR)/tmp/deploy/plugin
OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/$(OBJDIRNAME)
DBG_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/obj$(DBG_SUFFIX)
OPT_OBJDIR = $(OBJOUTPUTDIR)/$(LIBRARY_OUTPUT)/obj$(OPT_SUFFIX)

LDLIBS = -lX11 -ldl -lc
EXTRA_CPPFLAGS += -B $(ALT_COMPILER_PATH)/../lib/gcc-lib/ -I $(ALT_COMPILER_PATH)/../lib/gcc-lib/i386-redhat-linux/egcs-2.9*/include

JNIDIR = $(OUTPUTDIR)/include/$(PLATFORM)
COREDIR = $(PLUGIN_TOP)/src/plugin/solaris/nscore
COMMONDIR=  $(PLUGIN_TOP)/src/plugin/solaris/common
MOZHDRDIR = $(subst \,/,$(MOZILLA_HEADERS_PATH)/mozilla_headers_ns7) \
			$(subst \,/, $(MOZILLA_HEADERS_PATH)/mozilla_headers_ns7/nspr)

include ../ns7-adapter/FileList.gmk 

FILES_cpp += GetFactory5.cpp \
	     GetLibrary.cpp \
	     CNSAdapter_NSPR.cpp

INTERFACESDIR = $(PLUGIN_TOP)/src/plugin/share/jpiapi
ADAPTERINCDIR = $(PLUGIN_TOP)/src/plugin/share/adapter/include

CPPINCPATH = $(OUTPUTDIR)/include $(JNIDIR) \
	     $(subst $(LIBRARY_OUTPUT),,$(TMPDIR)) \
	     $(COMMONDIR) \
	     $(SRCDIR) \
	     $(MOZHDRDIR) \
	     $(ADAPTERCOMMONDIR) \
	     $(SHAREDIR)/jpiapi \
	     $(SHAREDIR)/adapter/include \
	     $(COREDIR)

vpath %.c $(ADAPTERCOMMONDIR)
vpath %.cpp $(SRCDIR):$(ADAPTERCOMMONDIR):$(COMMONDIR)

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk
endif

clobber::
	$(RM) -rf $(LIB_LOCATION)
