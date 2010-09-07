#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.19 10/03/24
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../../..

LIBRARY = javacpl
SRCDIR = $(DEPLOY_TOPDIR)/src/plugin/win32/controlpanel/cpl
PCHFILENAME=$(OBJDIR)/$(LIBRARY).pch

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

LIBRARY_SUFFIX = cpl

CPPINCPATH = $(SRCDIR) $(subst $(LIBRARY),,$(TEMPDIR)) $(subst $(LIBRARY)/,,$(CLASSHDRDIR)) $(DEPLOY_TOPDIR)/src/plugin/win32/common $(DEPLOY_COMMON_WIN_SRC)

RSCINC = -i $(SRCDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))
LDLIBS += user32.lib shell32.lib
ifeq ($(VARIANT), DBG)
  ifeq ($(ARCH_DATA_MODEL), 64)
    LDLIBS += libcd.lib
  endif
endif
LFLAGS += -def:$(SRCDIR)/javacpl.def

#
# Override the CCP flags (set in plugin/common/Defs.gmk) so that this program
# statically linked.  As of Visual C .NET, the support libraries for dynamic
# linking (/MD - msvcp71.dll and msvcr71.dll) are NOT supposed to be present
# in the Windows system32 directory.
#
# For the specification requirement, see:
#     http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt_c_run.2d.time_libraries.asp
#

ifeq ($(VARIANT), DBG)
  CPPFLAGS1 = -MLd -W3 -Od -GX -Gm -Zi -D_DEBUG -Fd"$(OBJDIR)\debug"
  ifeq ($(ARCH_DATA_MODEL), 32)
    CPPFLAGS1 += -GZ -ZI
  endif
else
  CPPFLAGS1 = -ML -W3 -Ox -Os -GX -Zi -DNDEBUG -Fd"$(OBJDIR)\release"
endif
 
vpath %.cpp $(SRCDIR):$(SRCDIR)/../../common:$(DEPLOY_COMMON_WIN_SRC)
vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clobber::
	$(RM) $(OBJDIR)/*.res
