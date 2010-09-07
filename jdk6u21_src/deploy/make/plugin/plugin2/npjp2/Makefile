#
# @(#)Makefile	1.15 10/03/24 12:01:14
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


DEPLOY_TOPDIR = ../../../..

# Might want to consider a more descriptive name, in particular on Unix platforms ("javaplugin_npruntime"?)
LIBRARY 	= npjp2
LIBRARY_OUTPUT  = $(LIBRARY)

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

ifeq ($(PLATFORM), windows)
  SRC_SUBDIR_PD=win32
  # NOTE that on the Windows platform the old and new plug-ins can not coexist
  # in the same directory because Firefox is responsible for scanning the JRE
  # for the Java Plug-In. Under Bugzilla bug 406040 the capability was added
  # to Firefox 3 to scan the "new_plugin" subdirectory if it exists and if
  # the UseNewJavaPlugin registry key is set. Until we remove the old plug-in,
  # therefore, we build our Firefox plugin on Windows into the new_plugin
  # subdirectory.
  #
  # NOTE AGAIN that this directory name is known to Firefox. Do not change it.
  BINDIR = $(OUTPUTDIR)/bin/new_plugin
endif # PLATFORM
ifeq ($(PLATFORM), solaris)
  SRC_SUBDIR_PD=solaris
endif # PLATFORM
ifeq ($(PLATFORM), linux)
  SRC_SUBDIR_PD=solaris
endif # PLATFORM

PLUGINSRCDIR    = $(DEPLOY_TOPDIR)/src/plugin/share
PLUGINSRCDIR_PD = $(DEPLOY_TOPDIR)/src/plugin/$(SRC_SUBDIR_PD)
SRCDIR          = $(PLUGINSRCDIR)/plugin2/$(LIBRARY)
SRCDIR_PD       = $(PLUGINSRCDIR_PD)/plugin2/$(LIBRARY)

# In the future(Firefox 3.x), once we ammend NPAPI for cookie/proxy service, we 
# should no longer need to depend on Mozilla headers.
ifeq ($(PLATFORM), windows)
  ifeq ($(ARCH), amd64)
    MOZHDRDIRS := $(shell $(ECHO) $(subst \,/,$(MOZILLA_HEADERS_PATH))/mozilla_headers_18.amd64/*)
  else
    MOZHDRDIRS := $(shell $(ECHO) $(subst \,/,$(MOZILLA_HEADERS_PATH))/mozilla_headers_18.win32/*)
  endif
else
MOZHDRDIRS := $(shell $(ECHO) $(subst \,/,$(MOZILLA_HEADERS_PATH))/mozilla_headers_18/*)
endif

# We need to pull in the latest versions of the NPAPI and NPRuntime headers as well
PLUGIN2_MOZHDRDIRS := $(shell $(ECHO) $(subst \,/,$(MOZILLA_HEADERS_PATH))/plugin2_6u14_mozilla_headers)

# TEMPDIR/.. is for the generated PluginVersion.h
CPPINCPATH = $(SRCDIR)                            \
             $(PLUGINSRCDIR)/plugin2/common       \
             $(PLUGINSRCDIR)/plugin2/npjp2        \
             $(PLUGINSRCDIR_PD)/plugin2/common    \
             $(PLUGINSRCDIR_PD)/plugin2/npjp2     \
             $(TEMPDIR)/..                        \
             $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) 

ifeq ($(PLATFORM), windows)
  RSCINC = -i $(SRCDIR_PD) -i $(subst $(LIBRARY_OUTPUT),,$(TEMPDIR))
  LFLAGS += -def:$(SRCDIR_PD)/$(LIBRARY).def
  OTHER_LDLIBS = user32.lib gdi32.lib
  # For debug.h
  CPPINCPATH += $(DEPLOY_TOPDIR)/src/plugin/win32/common
  # For common.h
  CPPINCPATH += $(DEPLOY_COMMON_WIN_SRC)
endif # PLATFORM

# We need to use this mechanism to pick up these headers as opposed to
# the CPPINCPATH because otherwise we will override the definitions of
# key structures like JNIEnv* with the incorrect Mozilla versions.
# Note also that the order here is important; we need to pick up the
# PLUGIN2_MOZHDRDIRS first. Ideally we would not have any dependencies
# on the OJI headers.
VPATH.h += $(PLUGIN2_MOZHDRDIRS) $(MOZHDRDIRS)
vpath %.h $(VPATH.h)

VPATH.cpp = $(SRCDIR)                         \
	    $(SRCDIR_PD)                      \
	    $(PLUGINSRCDIR)/plugin2/common    \
	    $(PLUGINSRCDIR_PD)/plugin2/common

VPATH.c = $(PLUGINSRCDIR_PD)/plugin2/common
ifeq ($(PLATFORM), windows)
VPATH.c   += $(PLUGINSRCDIR_PD)/common
VPATH.cpp += $(DEPLOY_COMMON_WIN_SRC)
endif # PLATFORM

vpath %.cpp $(VPATH.cpp)
vpath %.c   $(VPATH.c)

ifeq ($(PLATFORM), windows)
vpath %.rc  $(SRCDIR_PD)
endif # PLATFORM

include ./FileList.gmk 

clean:: clobber

clobber::
	$(RM) $(OBJDIR)/*.*

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

ifeq ($(PLATFORM), windows)
# We need a copy of msvcr71.dll in the same directory as npjp2.dll
copy-msvcr71::
ifneq ($(ARCH), amd64)
	$(CP) -f $(OUTPUTDIR)/bin/msvcr71.dll $(OUTPUTDIR)/bin/new_plugin/
else
	$(CP) -f $(OUTPUTDIR)/bin/msvcrt.dll $(OUTPUTDIR)/bin/new_plugin/
endif

all: build copy-msvcr71
endif
