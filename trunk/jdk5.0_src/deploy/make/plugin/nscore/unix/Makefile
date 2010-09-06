#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.1 02/07/23
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../../..

LIBRARY = javaplugin_nscp
SRCDIR = $(PLUGIN_TOP)/src/plugin/solaris/nscore
COMMONDIR=$(PLUGIN_TOP)/src/plugin/solaris/common
SHAREDDIR = $(PLUGIN_TOP)/src/plugin/share

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), linux)
LDLIBS = -lX11 -ldl -lc
else
LDLIBS = -lXt -lX11 -ldl -lCrun -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common \
             $(subst $(LIBRARY),,$(TMPDIR)) \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) \
             $(COMMONDIR) \
             $(PLUGIN_TOP)/src/plugin/share/jpiapi \
             $(SRCDIR)/../nsstub

ifeq ($(PLATFORM), linux)
EXTRA_CPPFLAGS	= -DLINUX -DRAPTOR_API -DNEW_STREAMING_API -DXP_UNIX
else
EXTRA_CPPFLAGS	= -DRAPTOR_API -DNEW_STREAMING_API -DXP_UNIX
endif

vpath %.c $(SRCDIR):$(SRCDIR)/../common:
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

clobber::

