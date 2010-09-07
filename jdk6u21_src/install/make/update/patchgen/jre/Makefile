#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.31 10/03/23
#
# all		   -- build everything
# clean		   -- cleans everything created here only.
# settings-patch   -- outputs the settings

all:: 
	@$(ECHO) $(ARCH) "Patch Generation Started:  " $(FULL_VERSION)


INSTALL_BUILDDIR=../../..
PRODUCT=patchgen
SRCDIR=../../../../src/windows/patchgen

include $(INSTALL_BUILDDIR)/common/Defs.gmk


SED_JRE_SCRIPT=$(PATCH_OUTPUTDIR)/sed-script-jre-$(PATCH_VERSION)
PATCHGEN_JRE_CMD=$(PATCH_OUTPUTDIR)/patchgen-jre-$(PATCH_VERSION).cmd

$(PATCH_OUTPUTDIR):
	$(MKDIR) -p $(PATCH_OUTPUTDIR)

BASE_IMAGE_JRE_DIR=$(ALT_BASE_IMAGE_DIR)/$(SMALLER_PATCH)

#
# Core.
#
all build :: $(PATCH_OUTPUTDIR) settings-patch
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@$(ECHO) 's!NEW_IMAGE_DIR!$(NEW_IMAGE_JRE_DIR)!g' | $(SED) 's!\\!/\\\\!g' > $(SED_JRE_SCRIPT)
	@$(ECHO) 's!BASE_IMAGE_DIR!$(ALT_BASE_IMAGE_DIR)/$(SMALLER_PATCH)!g' | $(SED) 's!\\!\\\\!g' >> $(SED_JRE_SCRIPT)
	@$(ECHO) 's!PATCH_OUTPUTDIR!$(PATCH_OUTPUTDIR)/$(PATCH_JRE_FILENAME)!g' | $(SED) 's!\/!\\\\!g' >> $(SED_JRE_SCRIPT)
	$(SED) -f $(SED_JRE_SCRIPT) $(SRCDIR)/patch.cmd > $(PATCHGEN_JRE_CMD)
	@# In order to avoid creating the PATCHBLD.ERR file in the src
	@# directory we cd to the PATCH_OUTPDIR and return
	@$(CD) $(PATCH_OUTPUTDIR) ; $(RTPATCH_DIR)/patchbld @$(PATCHGEN_JRE_CMD)
	$(MV) $(PATCH_OUTPUTDIR)/$(PATCH_JRE_FILENAME).RTP $(PATCH_OUTPUTDIR)/$(PATCH_JRE_FILENAME)-$(SMALLER_PATCH).RTP
endif


settings-patch:
	@$(ECHO) "Patch Build Versions:"
	@$(ECHO) "  NEW_IMAGE_JRE_DIR = $(NEW_IMAGE_JRE_DIR) "
	@$(ECHO) "  NEW_IMAGE_FULLVERSION = $(NEW_IMAGE_FULLVERSION) "
	@$(ECHO) "  NEW_IMAGE_MAJOR_VERSION (Parsed) = $(NEW_IMAGE_MAJOR_VERSION) "
	@$(ECHO) "  NEW_IMAGE_MINOR_VERSION (Parsed) = $(NEW_IMAGE_MINOR_VERSION) "
	@$(ECHO) "  NEW_IMAGE_MICRO_VERSION (Parsed) = $(NEW_IMAGE_MICRO_VERSION) "
	@$(ECHO) "  NEW_IMAGE_UPDATE_VERSION (Parsed) = $(NEW_IMAGE_UPDATE_VERSION) "
	@$(ECHO) "  NEW_IMAGE_BUILD_NUMBER (Parsed) = $(NEW_IMAGE_BUILD_NUMBER) "
	@$(ECHO) "  NEW_IMAGE_MILESTONE (Parsed) = $(NEW_IMAGE_MILESTONE) "
	@$(ECHO)  
	@$(ECHO) "  BASE_IMAGE_JRE_DIR = $(BASE_IMAGE_JRE_DIR) "
	@$(ECHO) "  BASE_IMAGE_FULLVERSION = $(BASE_IMAGE_FULLVERSION) "	
	@$(ECHO) "  BASE_IMAGE_MAJOR_VERSION (Parsed) = $(BASE_IMAGE_MAJOR_VERSION) "
	@$(ECHO) "  BASE_IMAGE_MINOR_VERSION (Parsed) = $(BASE_IMAGE_MINOR_VERSION) "
	@$(ECHO) "  BASE_IMAGE_MICRO_VERSION (Parsed) = $(BASE_IMAGE_MICRO_VERSION) "
	@$(ECHO) "  BASE_IMAGE_UPDATE_VERSION (Parsed) = $(BASE_IMAGE_UPDATE_VERSION) "
	@$(ECHO) "  BASE_IMAGE_BUILD_NUMBER (Parsed) = $(BASE_IMAGE_BUILD_NUMBER) "
	@$(ECHO) "  BASE_IMAGE_MILESTONE (Parsed) = $(BASE_IMAGE_MILESTONE) "
	@$#	
clean: 
	$(RM) -r $(PATCH_OUTPUTDIR)

# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Patch Generation finished:  " $(FULL_VERSION)

#
# Phonies to avoid accidents.
#
.PHONY: all build settings-patch clean






