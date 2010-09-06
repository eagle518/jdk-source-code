#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.1 04/02/10 
#
# Makefile for "launcher" program, this is a general purpose
# launcher which can be used to launch and suppress dos windows
# on any Win Application.
#

INSTALL_BUILDDIR= ../../../../..
PRODUCT = sun.tools.launcher

TMPDIR=$(TEMP_DIR)/$(PRODUCT)

include $(INSTALL_BUILDDIR)/common/Defs.gmk

TARGET=launcher$(EXE_SUFFIX)
TARGET_G=launcher_g$(EXE_SUFFIX)
CXXFLAGS   = 
CXXFLAGS_G = 

ifeq ($(PLATFORM), windows) #PLATFORM-WINDOWS
# optimized flags
CXXFLAGS   += -nologo -O1 -MD -c
CXXFLAGS_G += -nologo -c
LNKFLAGS   += -nologo /incremental:no /opt:ref
# The following settings are ia32 arch specific:
ifeq ($(ARCH_DATA_MODEL), 32)
  CXXFLAGS += -G5
  LNKFLAGS += /pdbtype:sept
  WIN32LIBS = user32.lib
else
  WIN32LIBS =
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

CTARGDIR=

FILES_c=$(CTARGDIR)WinMain.cpp

FILES_o   = $(patsubst %.cpp, %.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))

FILES_g_o = $(patsubst %.cpp, %_g.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))


$(OBJDIR)/%.$(OBJECT_SUFFIX): %.cpp
	$(CXX) $(CXXFLAGS) $(OBJOUTPUT)$@ $<

$(OBJDIR)/%_g.$(OBJECT_SUFFIX): %.cpp
	$(CXX) $(CXXFLAGS_G) $(OBJOUTPUT)$@ $<

$(OBJDIR)/$(TARGET): $(FILES_o)
	$(LINK) $(LNKFLAGS) $(LNKOUTPUT)$(OBJDIR)/$(@F) $(FILES_o) $(WIN32LIBS) 

$(OBJDIR)/$(TARGET_G): $(FILES_g_o)
	$(LINK) $(LNKFLAGS) $(LNKOUTPUT)$(OBJDIR)/$(@F) $(FILES_g_o) $(WIN32LIBS) 

clean clobber:
	$(RM) $(OBJDIR)/*.$(OBJECT_SUFFIX) $(OBJDIR)/$(TARGET) $(OBJDIR)/*.pdb
	$(RM) -r $(OBJDIR) $(TMPDIR)

.PHONY: all clean clobber
