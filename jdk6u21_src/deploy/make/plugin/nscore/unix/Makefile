#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.16 10/03/24 
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../../..

LIBRARY = javaplugin_nscp
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/solaris/nscore
COMMONDIR=$(DEPLOY_TOPDIR)/src/plugin/solaris/common
SHAREDDIR = $(DEPLOY_TOPDIR)/src/plugin/share

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

ifeq ($(PLATFORM), linux)
LDLIBS = -lX11 -ldl -lc
else
LDLIBS = -lXt -lX11 -ldl -lCrun -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common \
             $(subst $(LIBRARY),,$(TEMPDIR)) \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) \
             $(COMMONDIR) \
             $(DEPLOY_TOPDIR)/src/plugin/share/jpiapi \
             $(SRCDIR)/../nsstub

ifeq ($(PLATFORM), linux)
EXTRA_CPPFLAGS	= -DLINUX -DRAPTOR_API -DNEW_STREAMING_API -DXP_UNIX
else
EXTRA_CPPFLAGS	= -DRAPTOR_API -DNEW_STREAMING_API -DXP_UNIX
endif

# Use a mapfile on Solaris/Linux
#FILES_m = mapfile-vers
#ifdef FILES_m
#include $(J2SE_TOPDIR)/make/common/Mapfile-vers.gmk
#endif

vpath %.c $(SRCDIR):$(SRCDIR)/../common:
vpath %.cpp $(SRCDIR):$(SRCDIR)/../common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clobber::

