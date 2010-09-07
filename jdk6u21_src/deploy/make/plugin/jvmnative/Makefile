#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.17 10/03/24
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../..

LIBRARY = javaplugin_jni
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/solaris/jvm_natives

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

ifeq ($(PLATFORM), linux)
EXTRA_CPPFLAGS = -DLINUX
else
LDLIBS = /usr/lib/libm.so -ldl -lXt -lX11 -lsocket -lnsl -lintl -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common $(subst $(LIBRARY),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))

vpath %.c $(SRCDIR):$(SRCDIR)/../common

include ./FileList.gmk 

# Use mapfile on Solaris/Linux
#FILES_m = mapfile-vers
#ifdef FILES_m
#include $(J2SE_TOPDIR)/make/common/Mapfile-vers.gmk
#endif

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

