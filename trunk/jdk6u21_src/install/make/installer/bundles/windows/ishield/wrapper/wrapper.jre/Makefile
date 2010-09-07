#
# Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# Makefile for building JRE offline installer with wrapper code. 
#

INSTALL_BUILDDIR= ../../../../../..
PLUGIN_SRC= $(INSTALL_BUILDDIR)/../../deploy/src/plugin
PLUGIN_TMP= $(OUTPUTDIR)/tmp/deploy/plugin
PRODUCT = wrapper
SRCDIR=./
TEMPDIR=$(TEMP_DIR)/wrapper-jre

# Tell Defs.gmk that we want a static link of the MS runtime libraries
MS_RUNTIME_STATIC=true

INSANE=TRUE
include $(INSTALL_BUILDDIR)/common/Defs.gmk

EXEVALUE=$(JRE_NAME)-$(JDK_MINOR_VERSION)-windows-$(ARCH)
EXEDESVALUE=$(JAVA_DESC) $(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)$(MARKET_NAME)
IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_EXE_FILENAME=$(SED) -e 's%EXEFILENAME%$(EXEVALUE)%g'
SED_APP_VERSION=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'
SED_APP_DES=$(SED) -e 's%EXEDES%$(EXEDESVALUE)%g'
SED_ALL_MANIFEST=$(SED_APP_VERSION) | $(SED_EXE_FILENAME) | $(SED_APP_DES)

CURRENTDIR := $(call FullPath,.)

TARGET_NAME=wrapper
TARGET=$(TARGET_NAME)$(SUFFIX)$(EXE_SUFFIX)

ifneq ($(ARCH), amd64)
  ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
    JREISHIELDDIR=$(TMPISHIELD_DIR)/patch/jre
  else
    JREISHIELDDIR=$(TMPISHIELD_DIR)/regular/jre
  endif
  ifdef ALT_JRE_32BIT_FILEPATH
    JRE_32BIT_FILENAME = $(notdir $(basename $(ALT_JRE_32BIT_FILEPATH)))
  endif
else
  JREISHIELDDIR=$(TMPISHIELD_DIR)/patch/jre
endif
JRE_BUNDLE_NAME = $(JRE_NAME)-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)

# CPP flags
ifdef J4B
	CPPFLAGS += -DJ4B
endif

# Linker flags
LDFLAGS += version.lib shell32.lib wininet.lib wintrust.lib 
ifeq ($(ARCH), amd64)
  LDFLAGS += bufferoverflowu.lib 
endif

all: jre jre-sponsor

jre: clean setup_jre 
	$(CAT)  $(SRCDIR)/wrapper.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/wrapperer.exe.manifest
	$(MAKE) target
ifeq ($(SIGN_FLAG),1)
	$(call Try3Times, $(FULL_SIGNER) -n "$(JAVA_DESC) $(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)$(MARKET_NAME)" -i http://java.com/  $(OBJDIR)/wrapper.exe )
endif
	$(CP) $(OBJDIR)/wrapper.exe $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).exe
ifdef JRE_32BIT_FILENAME
	$(CP) $(OBJDIR)/wrapper.exe $(BIN_BUNDLEDIR)/$(JRE_32BIT_FILENAME).exe
endif
	$(RM)  $(OBJDIR)/*

setup_jre: $(OBJDIR)
	$(CP) -f $(JREISHIELDDIR)/../offline-final/jre$(JDK_VERSION)-c.msi $(OBJDIR)/jre-c.msi
ifeq ($(ARCH_DATA_MODEL), 32)
  ifdef ALT_AUMSI_DIR	
	$(CP) -f $(ALT_AUMSI_DIR)/au.msi $(OBJDIR)/au.msi
	$(CP) -f $(ALT_AUMSI_DIR)/au.cab $(OBJDIR)/au.cab
  else
	$(CP) -f $(INSTALL_BUILDDIR)/au/au.msi $(OBJDIR)/au.msi
	$(CP) -f $(INSTALL_BUILDDIR)/au/au.cab $(OBJDIR)/au.cab
  endif
endif
	$(CP) -f $(JREISHIELDDIR)/../offline-final/jre$(JDK_VERSION)-s.msi $(OBJDIR)/jre-s.msi
	$(CP) -f $(JREISHIELDDIR)/../offline-final/Data1.cab $(OBJDIR)/
	$(CP) $(PLUGIN_SRC)/win32/graphics/java-icon.ico $(OBJDIR)/java-icon.ico
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1028.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1031.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1034.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1036.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1040.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1041.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1042.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1046.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre1053.MST $(OBJDIR)/
	$(CP) $(JREISHIELDDIR)/jre/transform/javaone/jre2052.MST $(OBJDIR)/
BACKSLASH_OBJDIR = $(subst /,\\,$(OBJDIR))
MSIDBCMD:=$(shell cd $(OBJDIR) ; $(DEVTOOLS_PATH)/msidb.exe -e -d au.msi -f $(BACKSLASH_OBJDIR) Property)
AUVERSION:=$(shell $(CD) $(OBJDIR) ; $(CAT) Property.idt | $(GREP) ProductVersion | $(NAWK)  '{print $$2}')
CPPFLAGS += -DAUVERSION='"$(AUVERSION)"'

jre-sponsor:
ifneq ($(ARCH), amd64)
	$(MAKE) -f Makefile.sponsor
endif

plus:
	$(RM) $(OBJDIR)/au.msi $(OBJDIR)/au.cab $(OBJDIR)/*.obj $(OBJDIR)/*res $(OBJDIR)/*pdb $(OBJDIR)/*map
ifdef ALT_AUMSI_DIR	
	$(CP) -f $(ALT_AUMSI_DIR)/au.msi $(OBJDIR)/au.msi
	$(CP) -f $(ALT_AUMSI_DIR)/au.cab $(OBJDIR)/au.cab
else
	$(CP) -f $(INSTALL_BUILDDIR)/au/au.msi $(OBJDIR)/au.msi
	$(CP) -f $(INSTALL_BUILDDIR)/au/au.cab $(OBJDIR)/au.cab
endif
	$(MAKE) target
ifeq ($(SIGN_FLAG),1)
	$(call Try3Times, $(FULL_SIGNER) -n "$(JAVA_DESC) $(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)$(MARKET_NAME)" -i http://java.com/  $(OBJDIR)/wrapper.exe )
endif
	$(CP) $(OBJDIR)/wrapper.exe $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).exe
	$(MAKE) -f Makefile.sponsor plus

target: optimized $(BIN_BUNDLEDIR) debug 

$(OBJDIR):
	$(MKDIR) -p $(OBJDIR)

$(BIN_BUNDLEDIR):
	$(MKDIR) -p  $(BIN_BUNDLEDIR)

FILES_rc = $(SRCDIR)wrapper.rc

vpath %.cpp $(SRCDIR):$(PLUGIN_SRC)/win32/jupdate/checker:$(PLUGIN_SRC)/../common/windows/native:$(PLUGIN_SRC)/win32/regutils $(PLUGIN_SRC)/win32/common:$(SRCDIR)/../common

FILES_c= \
	wrapper.cpp \
	StdAfx.cpp \
	Locale.cpp \
	XMLParser.cpp \
	common.cpp \
	UserProfile.cpp \
	UpdateUtils.cpp \
	UpdateConf.cpp \
	WrapperUtils.cpp \
	WrapperOffline.cpp


FILES_o	= $(patsubst %.cpp, %.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))
FILES_res = $(patsubst %.rc, %.res, $(addprefix $(OBJDIR)/, $(notdir $(FILES_rc))))

$(OBJDIR)/%.res: $(SRCDIR)%.rc
ifeq ($(VARIANT), DBG)
  ifeq ($(ARCH_DATA_MODEL), 32)
	$(RC) $(RC_FLAGS) -l 0x409 -d _DEBUG -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
  else
	$(RC) $(RC_FLAGS) -l 0x409 -d _DEBUG -d _AMD64 -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
  endif
else
  ifeq ($(ARCH_DATA_MODEL), 32)
	$(RC) $(RC_FLAGS) -l 0x409 -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
  else
	$(RC) $(RC_FLAGS) -l 0x409 -d _AMD64 -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
  endif
endif

$(OBJDIR)/%.$(OBJECT_SUFFIX): %.cpp
	$(COMPILE.cc) -I$(PLUGIN_TMP) -I$(PLUGIN_SRC)/../common/windows/native  -I$(PLUGIN_SRC)/win32/jupdate/checker  -I $(PLUGIN_SRC)/win32/regutils -I$(PLUGIN_SRC)/win32/common -I$(SRCDIR)/../common -I$(subst $(EXECUTABLE),,$(TEMPDIR)) -DARCH=\"$(ARCH)\" -DDEFAULT_XML='"$(DEFAULT_XML)"' -DFULLVERSION='"$(FULL_VERSION)"' -DVERSION='"$(JDK_VERSION)"' -DBUNDLE='"jre"' $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.$(OBJECT_SUFFIX) $<

$(OBJDIR)/$(TARGET): $(FILES_res) $(FILES_o)
	$(LINK) $(LDFLAGS) $(FILES_res) $(FILES_o)


build: $(OBJDIR) $(BIN_BUNDLEDIR) $(OBJDIR)/$(TARGET)

debug:

optimized:
	$(MKDIR) -p $(OBJDIR)
	$(MAKE) build VARIANT=OPT

clean::
	$(RM) -r $(OBJDIR) $(OBJDIR)$(DBG_SUFFIX)
	$(RM) -r $(TEMPDIR)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber clean build
