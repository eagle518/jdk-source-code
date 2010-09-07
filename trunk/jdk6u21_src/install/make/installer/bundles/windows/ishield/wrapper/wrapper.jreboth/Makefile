#
# Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#  @(#)Makefile	1.10 09/11/04


INSTALL_BUILDDIR= ../../../../../..
PLUGIN_SRC= $(INSTALL_BUILDDIR)/../../deploy/src/plugin
PLUGIN_TMP= $(OUTPUTDIR)/tmp/deploy/plugin
PRODUCT = wrapper
SRCDIR=./
TEMPDIR=$(TEMP_DIR)/wrapper-jre-cobundle

# Tell Defs.gmk that we want a static link of the MS runtime libraries
MS_RUNTIME_STATIC=true

INSANE=TRUE
include $(INSTALL_BUILDDIR)/common/Defs.gmk

JRE_32BIT_INSTALLER := $(shell $(BASENAME) $(JRE_32BIT_FILEPATH) )
JRE_64BIT_INSTALLER := $(shell $(BASENAME) $(ALT_JRE_64BIT_FILEPATH) )
JRE_3264COMBO_NAME=jre-3264combo-$(JDK_MINOR_VERSION)u$(JDK_UPDATE_VERSION)-$(BUILD_NUMBER)-windows-$(BUNDLE_DATE)
EXEVALUE=$(JRE_3264COMBO_NAME)
EXEDESVALUE=$(JAVA_DESC) $(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)$(MARKET_NAME)
IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_EXE_FILENAME=$(SED) -e 's%EXEFILENAME%$(EXEVALUE)%g'
SED_APP_VERSION=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'
SED_APP_DES=$(SED) -e 's%EXEDES%$(EXEDESVALUE)%g'
SED_ALL_MANIFEST=$(SED_APP_VERSION) | $(SED_EXE_FILENAME) | $(SED_APP_DES)
SED_32BIT_JREFILE=$(SED) -e 's%JAVA32BITINSTALLER%$(JRE_32BIT_INSTALLER)%g'
SED_64BIT_JREFILE=$(SED) -e 's%JAVA64BITINSTALLER%$(JRE_64BIT_INSTALLER)%g'
SED_ALL_JREFILES=$(SED_32BIT_JREFILE) | $(SED_64BIT_JREFILE)

CURRENTDIR := $(call FullPath,.)

TARGET_NAME=wrapper
TARGET=$(TARGET_NAME)$(SUFFIX)$(EXE_SUFFIX)



# Linker flags
LDFLAGS += version.lib shell32.lib wininet.lib wintrust.lib 

all: jre 

jre: clean setup_jre 
	$(CAT)  $(SRCDIR)/wrapper.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/wrapperer.exe.manifest
	$(CAT) $(SRCDIR)/wrapper_file.rc | $(SED_ALL_JREFILES) >$(OBJDIR)/wrapper_file_sed.rc
	$(MAKE) target
ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "$(JAVA_DESC) $(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)$(MARKET_NAME)" -i http://java.com/  $(OBJDIR)/wrapper.exe
endif
	$(CP) $(OBJDIR)/wrapper.exe $(BIN_BUNDLEDIR)/$(JRE_3264COMBO_NAME).exe


setup_jre: $(OBJDIR)
	$(CP) $(PLUGIN_SRC)/win32/graphics/java-icon.ico $(OBJDIR)/java-icon.ico
	$(CP) $(JRE_32BIT_FILEPATH) $(OBJDIR)/
	$(CP) $(ALT_JRE_64BIT_FILEPATH) $(OBJDIR)/



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
	UserProfile.cpp \
	common.cpp \
	WrapperUtils.cpp \



FILES_o	= $(patsubst %.cpp, %.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))
FILES_res = $(patsubst %.rc, %.res, $(addprefix $(OBJDIR)/, $(notdir $(FILES_rc))))

$(OBJDIR)/%.res: $(SRCDIR)%.rc
ifeq ($(VARIANT), DBG)
	$(RC) $(RC_FLAGS) -l 0x409 -d _DEBUG -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
else
	$(RC) $(RC_FLAGS) -l 0x409 -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
endif

$(OBJDIR)/%.$(OBJECT_SUFFIX): %.cpp
	$(COMPILE.cc) -I$(PLUGIN_TMP) -I$(PLUGIN_SRC)/../common/windows/native  -I$(PLUGIN_SRC)/win32/jupdate/checker  -I $(PLUGIN_SRC)/win32/regutils -I$(PLUGIN_SRC)/win32/common -I$(SRCDIR)/../common -I$(subst $(EXECUTABLE),,$(TEMPDIR)) -DARCH=\"$(ARCH)\" -DDEFAULT_XML='"$(DEFAULT_XML)"' -DJRE_64BIT_INSTALLER='"$(JRE_64BIT_INSTALLER)"' -DJRE_32BIT_INSTALLER='"$(JRE_32BIT_INSTALLER)"' -DFULLVERSION='"$(FULL_VERSION)"' -DVERSION='"$(JDK_VERSION)"' -DBUNDLE='"jre"' $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.$(OBJECT_SUFFIX) $<

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
