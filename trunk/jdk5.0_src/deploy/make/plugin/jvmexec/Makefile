#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.7 03/12/19
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

EXECUTABLE = java_vm
PROGRAM = java_vm
SRCDIR = $(PLUGIN_TOP)/src/plugin/solaris/jvm_exec

CPPINCPATH = $(SRCDIR) $(SRCDIR)/../common

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), linux)
LDLIBS = -lpthread -ldl -lc
endif

vpath %.cpp $(SRCDIR)
vpath %.c $(SRCDIR)

include ./FileList.gmk 

include $(PLUGIN_TOP)/make/plugin/common/exec.gmk
include $(PLUGIN_TOP)/make/plugin/common/rules.gmk
