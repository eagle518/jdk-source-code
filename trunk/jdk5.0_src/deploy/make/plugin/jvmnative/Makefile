#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.10 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

LIBRARY = javaplugin_jni
SRCDIR = $(PLUGIN_TOP)/src/plugin/solaris/jvm_natives

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), linux)
LDLIBS = -lX11
EXTRA_CPPFLAGS = -DLINUX
else
LDLIBS = /usr/lib/libm.so -ldl -lXt -lX11 -lsocket -lnsl -lintl -lc
endif

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common $(subst $(LIBRARY),,$(TMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR))

vpath %.c $(SRCDIR):$(SRCDIR)/../common

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk

