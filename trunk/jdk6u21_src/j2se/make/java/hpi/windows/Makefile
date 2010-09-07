#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.23 10/03/23
#
# Makefile for Windows HPI DLL
#
BUILDDIR = ../../..
LIBRARY   = hpi
PRODUCT      = java
THREADDIR    = windows_threads
LIB_LOCATION = $(BINDIR)

include $(BUILDDIR)/common/Defs.gmk

# windows compiler flags
ifeq ($(PLATFORM),windows)
  CPPFLAGS_DBG += -DLOGGING
  # Files built here do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif

FILES_c = \
    $(TARGDIR)linker_md.c \
    $(TARGDIR)memory_md.c \
    $(TARGDIR)monitor_md.c \
    $(TARGDIR)path_md.c \
    $(TARGDIR)socket_md.c \
    $(TARGDIR)sys_api_md.c \
    $(TARGDIR)system_md.c \
    $(TARGDIR)threads_md.c \
    $(TARGDIR)hpi.c # trailing blank required!

JVMLIB = 
JAVALIB =
OTHER_LCF = -export:DLL_Initialize
EXTRA_LIBS =


#
# Other files/flags shared between the HPIs.
#
include $(BUILDDIR)/java/hpi/hpi_common.gmk

#
# Rules for the .so file.
#
include $(BUILDDIR)/common/Library.gmk

