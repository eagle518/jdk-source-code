#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# Makefile to build the jqs_plugin for Internet Explorer.  When the browser
# starts up, this plugin will notify JQS to immediately prefetch JRE 
# files/pages.
#
#

DEPLOY_TOPDIR = ../../..

LIBRARY = jqs_plugin
PRODUCT = deploy
PROGRAM = jqs


#
# Source directories
#

IDLDIR =  $(DEPLOY_TOPDIR)/src/jqs/windows/ie

SRCDIR =  $(DEPLOY_TOPDIR)/src/jqs/share \
	  $(DEPLOY_TOPDIR)/src/jqs/windows \
	  $(DEPLOY_TOPDIR)/src/jqs/windows/resources \
	  $(IDLDIR)

#
# Include standard build definitions and environment settings
#

include $(DEPLOY_TOPDIR)/make/common/Defs.gmk

OBJDIR = $(OUTPUTDIR)/tmp/deploy/jqs/$(LIBRARY)


#
# RPC interface files
#

IDL = $(IDLDIR)/jqsiestartdetector.idl
TLB = $(OBJDIR)/jqsiestartdetector.tlb
GENH = $(OBJDIR)/jqsiestartdetector.h
GENC = $(OBJDIR)/jqsiestartdetector_i.c
GENCP = $(OBJDIR)/jqsiestartdetector_p.c
DLLDATA = $(OBJDIR)/dlldata.c
GENC_OBJ = $(OBJDIR)/jqsiestartdetector_i.obj


#
# IDL compiler options
#

MTL_SWITCHES= -env win32 -Oicf -tlb "$(TLB)" -h "$(GENH)" -iid "$(GENC)" -proxy "$(GENCP)" -dlldata "$(DLLDATA)"


#
# Additions to standard build variables.
#

CPPINCPATH = $(SRCDIR) $(OBJDIR)

RSCINC = -i $(IDLDIR) -i $(OBJDIR)

EXTRA_CPPFLAGS := -DWIN32 -DWINDOWS -DWINDLL -DUSRDLL -DMBCS -Gm -EHsc -GS -DCRT_SECURE_NO_DEPRECATE -DJ2SE_BUILD_ID="\"$(FULL_VERSION)\""

LFLAGS += -def:"$(IDLDIR)/jqsiestartdetector.def" $(OBJDIR)/jqsiestartdetector_i.obj

LDLIBS += ws2_32.lib ole32.lib uuid.lib oleaut32.lib


#
# Source file search paths and file lists
#

vpath %.idl $(SRCDIR)
vpath %.cpp $(SRCDIR)
vpath %.rc  $(SRCDIR)

include ./FileList.gmk 


#
# Special build targets for IDL and RPC stub compilation
#

all optimized debug: $(TLB) $(GENC_OBJ)

$(TLB)::
	$(MKDIR) -p $(OBJDIR)
	@$(prep-target)
	$(MTL) $(MTL_SWITCHES) $(IDL)

$(GENC_OBJ):: $(GENC)
	$(CXX) $(CPPFLAGS) -Fo$@ $(GENC)


#
# Include standard build targets
#

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk


clobber::
	$(RM) -r $(OBJDIR)

.PHONY: all clean
