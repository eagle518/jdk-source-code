#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.10 10/03/23 
#
# Makefile for "launcher" program, this is a general purpose
# launcher which can be used to launch and suppress dos windows
# on any Win Application.
#

INSTALL_BUILDDIR= ../../../../..
PRODUCT = sun.tools.launcher

TEMPDIR=$(TEMP_DIR)/$(PRODUCT)

# Tell Defs.gmk that we want a static link of the MS runtime libraries
MS_RUNTIME_STATIC=true

include $(INSTALL_BUILDDIR)/common/Defs.gmk

EXEVALUE=launcher.exe
IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_EXE_FILENAME=$(SED) -e 's%EXEFILENAME%$(EXEVALUE)%g'
SED_APP_VERSION=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'
SED_ALL_MANIFEST=$(SED_APP_VERSION) | $(SED_EXE_FILENAME)

TARGET=launcher$(EXE_SUFFIX)
TARGET_G=launcher_g$(EXE_SUFFIX)
CXXFLAGS   = 
CXXFLAGS_G = 

ifeq ($(PLATFORM), windows) #PLATFORM-WINDOWS
# optimized flags
CXXFLAGS   += -nologo -O1 $(MS_RUNTIME_OPTION) -c
CXXFLAGS_G += -nologo -c
LNKFLAGS   += -nologo /incremental:no /opt:ref
# The following settings are ia32 arch specific:
ifeq ($(ARCH_DATA_MODEL), 32)
  CXXFLAGS += -G5
  LNKFLAGS += /pdbtype:sept
  WIN32LIBS = user32.lib
else
  WIN32LIBS =
  LNKFLAGS += /machine:$(LIBARCH)
endif

OBJOUTPUT = -Fo
LNKOUTPUT = -out:
else #PLATFORM-UNIX
CXXFLAGS_G += -g -c
ifeq ($(PLATFORM), solaris)
CXXFLAGS   += -xO4 -xspace -nolib -lCrun -lc -c
else #PLATFORM-LINUX 
CXXFLAGS   += -O -c
endif #PLATFORM_LINUX
LNKFLAGS +=
OBJOUTPUT = -o #Caution:the space after the -o is significant!.
LNKOUTPUT = $(OBJOUTPUT)
LINK = $(CXX)
endif #PLATFORM

all: $(OBJDIR) $(OBJDIR)/$(TARGET) $(OBJDIR)/$(TARGET_G)

$(OBJDIR):
	$(MKDIR) -p $(OBJDIR)
	$(CAT)  launcher_proto.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/launcher.exe.manifest

CTARGDIR=

FILES_rc = launcher.rc

FILES_c=$(CTARGDIR)WinMain.cpp

FILES_o   = $(patsubst %.cpp, %.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))

FILES_g_o = $(patsubst %.cpp, %_g.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))

FILES_res = $(patsubst %.rc, %.res, $(addprefix $(OBJDIR)/, $(notdir $(FILES_rc))))

$(OBJDIR)/%.res: %.rc
	@$(RM) $@
	@$(MKDIR) -p $(@D)
ifeq ($(VARIANT), DBG)
	$(RC) -l 0x409 -d _DEBUG -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
else
	$(RC) -l 0x409  -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
endif

$(OBJDIR)/%.$(OBJECT_SUFFIX): %.cpp
	$(CXX) $(CXXFLAGS) $(OBJOUTPUT)$@ $<

$(OBJDIR)/%_g.$(OBJECT_SUFFIX): %.cpp
	$(CXX) $(CXXFLAGS_G) $(OBJOUTPUT)$@ $<

$(OBJDIR)/$(TARGET): $(FILES_res) $(FILES_o)
	$(LINK) $(LNKFLAGS) $(LNKOUTPUT)$(OBJDIR)/$(@F) $(FILES_res) $(FILES_o) $(WIN32LIBS)

$(OBJDIR)/$(TARGET_G): $(FILES_g_o)
	$(LINK) $(LNKFLAGS) $(LNKOUTPUT)$(OBJDIR)/$(@F) $(FILES_g_o) $(WIN32LIBS) 

clean clobber:
	$(RM) $(OBJDIR)/*.$(OBJECT_SUFFIX) $(OBJDIR)/$(TARGET) $(OBJDIR)/*.pdb
	$(RM) -r $(OBJDIR) $(TEMPDIR)

.PHONY: all clean clobber
