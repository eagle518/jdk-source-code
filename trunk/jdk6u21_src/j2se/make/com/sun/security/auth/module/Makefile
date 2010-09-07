# 
# @(#)Makefile	1.31 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for building auth modules.

BUILDDIR = ../../../../..
PACKAGE = com.sun.security.auth.module
PRODUCT = sun

# LIBRARY name changed later below, but it is important to tell Defs some name
#    upfront so that it knows it is building a library.
LIBRARY = jaas  

# Force name of temp directory (needed due to LIBRARY change below)
LIBRARY_OUTPUT = jaas  

include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#

include FILES_java.gmk


ifeq ($(PLATFORM), windows)

LIBRARY = jaas_nt
  include FILES_export_windows.gmk
  include FILES_c_windows.gmk
# We need some extra libs for windows 
  ifneq ($(LIBRARY),fdlibm)
    EXTRA_LIBS += netapi32.lib user32.lib mpr.lib
  endif #fdlibm
  # code generates errors when compiled at warning level 3 and warnings are fatal
  ifeq ($(ARCH_DATA_MODEL), 64)
    COMPILER_WARNINGS_FATAL=false
  endif # ARCH_DATA_MODEL
endif # windows

ifeq ($(PLATFORM), solaris)
  LIBRARY = jaas_unix
  FILES_m = mapfile-vers
  include $(BUILDDIR)/common/Mapfile-vers.gmk
  include FILES_export_unix.gmk
  include FILES_export_solaris.gmk
  include FILES_c_unix.gmk
  include FILES_c_solaris.gmk
endif # solaris

ifeq ($(PLATFORM), linux)
  LIBRARY = jaas_unix
  include FILES_export_unix.gmk
  include FILES_c_unix.gmk
endif # linux 



JAVAHFLAGS += -jni

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

#
# JVMDI implementation lives in the VM.
#
OTHER_LDLIBS = $(JVMLIB)
