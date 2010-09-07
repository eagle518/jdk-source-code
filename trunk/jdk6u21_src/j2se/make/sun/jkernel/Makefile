#
# @(#)Makefile	1.7 10/03/23
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = sun.jkernel
LIBRARY = jkernel
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Use highest optimization level
#
_OPT = $(CC_HIGHEST_OPT)

# This re-directs all the class files to a separate location
CLASSDESTDIR = $(TEMPDIR)/classes


#
# Java source files
#
include FILES_java.gmk
AUTO_FILES_JAVA_DIRS = sun/jkernel

LOCALE_SUFFIXES = $(J2RE_LOCALES)

#
# Native source files
#

ifeq ($(ARCH_DATA_MODEL), 32) 

ifeq ($(PLATFORM), windows)

include FILES_c_windows.gmk

CPPFLAGS += -I$(PLATFORM_SRC)/native/common

vpath %.cpp   $(PLATFORM_SRC)/native/sun/jkernel \
              $(PLATFORM_SRC)/native/common

VERSIONINFO_RESOURCE = $(PLATFORM_SRC)/native/sun/jkernel/kernel.rc

LDLIBS += urlmon.lib wininet.lib shlwapi.lib version.lib comctl32.lib gdi32.lib -def:$(PLATFORM_SRC)/native/sun/jkernel/kernel.def

include $(BUILDDIR)/common/Library.gmk

endif

endif

#
# Resources
#
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

# If extra compression is going to be available in the deploy build enable 
# its use here by defining the JRE-relative pathname of the shared library

ifeq ($(PLATFORM), windows)
  ifdef EXTRA_COMP_INSTALL_PATH
    CPPFLAGS += -DEXTRA_COMP_INSTALL_PATH=$(EXTRA_COMP_INSTALL_PATH)
  endif
endif
