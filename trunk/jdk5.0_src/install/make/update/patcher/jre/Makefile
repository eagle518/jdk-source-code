#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.40 04/05/21
#
#
# Makefile for building and packaging all of the JDK and the JRE. See
# also included files.
#
#
# Most frequently used targets:
#
#    all            -- build optimized and debug versions of the core JDK and 
#                      ext (Defined in this makefile only)
#    clobber        -- clobber the builds of the core JDK and ext (defeined
#                      in this makefile only)
#

INSTALL_BUILDDIR= ../../..
PRODUCT = jre
SRCDIR=../../../../src/windows/patcher/

TARGET_NAME=jre-patcher
TARGET=$(TARGET_NAME)$(SUFFIX)$(EXE_SUFFIX)


TMPDIR=$(TEMP_DIR)/patcher/$(PRODUCT)

include $(INSTALL_BUILDDIR)/common/Defs.gmk

CPPFLAGS += -D FULL_VERSION="\"$(FULL_VERSION)\""
CPPFLAGS += -D RELEASE="\"$(RELEASE)\""

CPPFLAGS += -D BASE_IMAGE_MAJOR_VERSION="\"$(BASE_IMAGE_MAJOR_VERSION)\""
CPPFLAGS += -D BASE_IMAGE_MINOR_VERSION="\"$(BASE_IMAGE_MINOR_VERSION)\""
CPPFLAGS += -D BASE_IMAGE_MICRO_VERSION="\"$(BASE_IMAGE_MICRO_VERSION)\""
CPPFLAGS += -D BASE_IMAGE_UPDATE_VERSION="\"$(BASE_IMAGE_UPDATE_VERSION)\""
CPPFLAGS += -D BASE_IMAGE_MILESTONE="\"$(BASE_IMAGE_MILESTONE)\""
CPPFLAGS += -D BASE_IMAGE_BUILD_NUMBER="\"$(BASE_IMAGE_BUILD_NUMBER)\""
CPPFLAGS += -D BASE_IMAGE_VERSION="\"$(BASE_IMAGE_VERSION)\""
CPPFLAGS += -D BASE_IMAGE_FULLVERSION="\"$(BASE_IMAGE_FULLVERSION)\""

CPPFLAGS += -D NEW_IMAGE_MAJOR_VERSION="\"$(NEW_IMAGE_MAJOR_VERSION)\""
CPPFLAGS += -D NEW_IMAGE_MINOR_VERSION="\"$(NEW_IMAGE_MINOR_VERSION)\""
CPPFLAGS += -D NEW_IMAGE_MICRO_VERSION="\"$(NEW_IMAGE_MICRO_VERSION)\""
CPPFLAGS += -D NEW_IMAGE_UPDATE_VERSION="\"$(NEW_IMAGE_UPDATE_VERSION)\""
CPPFLAGS += -D NEW_IMAGE_MILESTONE="\"$(NEW_IMAGE_MILESTONE)\""
CPPFLAGS += -D NEW_IMAGE_BUILD_NUMBER="\"$(NEW_IMAGE_BUILD_NUMBER)\""
CPPFLAGS += -D NEW_IMAGE_VERSION="\"$(NEW_IMAGE_VERSION)\""
CPPFLAGS += -D NEW_IMAGE_FULLVERSION="\"$(NEW_IMAGE_FULLVERSION)\""

CPPFLAGS += -D WINDOWS -D _MBCS -D _ATL_STATIC_REGISTRY 
CPPFLAGS += $(CPPFLAGS_$(VARIANT))
CPPFLAGS +=  -c -I $(RTPATCH_DIR) 

# Linker flags

LDFLAGS += -libpath:$(RTPATCH_DIR) version.lib shell32.lib


PATCH_RTP_FILE=$(PATCH_JRE_FILENAME).RTP

all:	$(OBJDIR) debug post-build


$(OBJDIR):
	$(MKDIR) -p $(OBJDIR)

$(BIN_BUNDLEDIR):
	$(MKDIR) -p  $(BIN_BUNDLEDIR)

FILES_rc = $(SRCDIR)PatchInstaller.rc

FILES_c= \
	$(SRCDIR)PatchInstaller.cpp \
	$(SRCDIR)PatchUtils.cpp \
	$(SRCDIR)PatchDialog.cpp \
	$(SRCDIR)StdAfx.cpp


FILES_o	= $(patsubst %.cpp, %.$(OBJECT_SUFFIX), $(addprefix $(OBJDIR)/, $(notdir $(FILES_c))))
FILES_res = $(patsubst %.rc, %.res, $(addprefix $(OBJDIR)/, $(notdir $(FILES_rc))))

$(OBJDIR)/%.res: $(SRCDIR)%.rc
ifeq ($(VARIANT), DBG)
	$(RC) -l 0x409 -d _DEBUG -i $(RTPATCH_DIR) -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
else
	$(RC) -l 0x409 -i $(RTPATCH_DIR) -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.res $<
endif

$(OBJDIR)/%.$(OBJECT_SUFFIX): $(SRCDIR)%.cpp
	$(COMPILE.cc) $(CC_OBJECT_OUTPUT_FLAG)$(OBJDIR)/$*.$(OBJECT_SUFFIX) $<

$(OBJDIR)/$(TARGET): $(FILES_res) $(FILES_o)
	$(LINK) $(LDFLAGS) $(FILES_res) $(FILES_o)


build: $(OBJDIR) $(BIN_BUNDLEDIR) $(OBJDIR)/$(TARGET)

debug:
	$(MKDIR) -p $(OBJDIR)$(DBG_SUFFIX)
	$(TOUCH) $(OBJDIR)$(DBG_SUFFIX)/patch.rtp
	@if [ -x `$(ECHO) $(PATCH_OUTPUTDIR)/$(PATCH_RTP_FILE)` ]; then \
	    $(CP) $(PATCH_OUTPUTDIR)/$(PATCH_RTP_FILE) $(OBJDIR)$(DBG_SUFFIX)/patch.rtp ; \
	fi
	$(MAKE) build VARIANT=DBG

optimized:
	$(MKDIR) -p $(OBJDIR)
	$(TOUCH) $(OBJDIR)/patch.rtp
	@if [ -x `$(ECHO) $(PATCH_OUTPUTDIR)/$(PATCH_RTP_FILE)` ]; then \
	    $(CP) $(PATCH_OUTPUTDIR)/$(PATCH_RTP_FILE) $(OBJDIR)/patch.rtp ; \
	fi
	$(MAKE) build VARIANT=OPT


post-build: optimized $(BIN_BUNDLEDIR)
ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "Java Update for JRE 1.5.0" -i http://java.com/ $(OBJDIR)/$(TARGET)
endif
#	$(CP) $(OBJDIR)/$(TARGET) $(BIN_BUNDLEDIR)/$(FINAL_JRE_TARGET)


clean::
	$(RM) -r $(OBJDIR) $(OBJDIR)$(DBG_SUFFIX)
	$(RM) -r $(TMPDIR)
#	$(RM) -rf $(BIN_BUNDLEDIR)/$(FINAL_JRE_TARGET)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber clean build post-build








