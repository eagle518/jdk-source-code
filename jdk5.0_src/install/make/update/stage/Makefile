#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.10 03/12/19
#
# Most frequently used targets:
#
#    all            -- build optimized and debug versions
#    clean          -- cleans the files and dirs created here.
#    base-stage-jre -- create a image of the BASE JRE
#    new-stage-jre  -- stages an image of the NEW JRE. Notice that
#		       pack/unpack is called, this must be done in
# 		       order to stage the JRE.
#    base-stage-jre -- create a image of the BASE SDK
#    new-stage-jre  -- stages an image of the NEW SDK. Notice that
#		       pack/unpack is called, this must be done in
# 		       order to stage the SDK.
all:: 
	@$(ECHO) $(ARCH) "Staging started:  " $(FULL_VERSION)


INSTALL_BUILDDIR=../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

STAGE_LOG_DIR=$(STAGE_DIR)/log

$(STAGE_LOG_DIR):
	$(MKDIR) -p $(STAGE_LOG_DIR)

$(BASE_STAGE_JRE_DIR):
	$(MKDIR) -p $(BASE_STAGE_JRE_DIR)

$(NEW_STAGE_JRE_DIR):
	$(MKDIR) -p $(NEW_STAGE_JRE_DIR)

$(BASE_STAGE_SDK_DIR):
	$(MKDIR) -p $(BASE_STAGE_SDK_DIR)

$(NEW_STAGE_SDK_DIR):
	$(MKDIR) -p $(NEW_STAGE_SDK_DIR)


#In this case we have been given a pointer to a zipped file.
base-stage-jre: $(BASE_STAGE_JRE_DIR)
	@#We need to clean up the dir first for remnants etc.
	$(RM) -r $(BASE_STAGE_JRE_DIR)/*
	$(UNZIP) $(BASE_IMAGE_JRE_ZIP) -d $(BASE_STAGE_JRE_DIR) > $(STAGE_LOG_DIR)/unzip-jre.log

base-stage-sdk: $(BASE_STAGE_SDK_DIR)
	@#We need to clean up the dir first for remnants etc.
	$(RM) -r $(BASE_STAGE_SDK_DIR)/*
	$(UNZIP) $(BASE_IMAGE_SDK_ZIP) -d $(BASE_STAGE_SDK_DIR) > $(STAGE_LOG_DIR)/unzip-sdk.log

#In this case we have the current build, we get the binaries from the image.
new-stage-jre: $(NEW_STAGE_JRE_DIR)
	@#We need to clean up the dir first for remnants etc.
	$(RM) -r $(NEW_STAGE_JRE_DIR)/*
	@# Find out jre copy it over
	$(CD) $(ABS_OUTPUTDIR)/j2re-image ; $(FIND) . -depth -print | $(CPIO) -pdmuv $(NEW_STAGE_JRE_DIR) > $(STAGE_LOG_DIR)/cpio-jre.log 2>&1

new-stage-sdk: $(NEW_STAGE_SDK_DIR)
	@#We need to clean up the dir first for remnants etc.
	$(RM) -r $(NEW_STAGE_SDK_DIR)/*
	@# Find out sdk copy it over
	$(CD) $(ABS_OUTPUTDIR)/j2sdk-image ; $(FIND) . -depth -print | $(CPIO) -pdmuv $(NEW_STAGE_SDK_DIR) > $(STAGE_LOG_DIR)/cpio-sdk.log 2>&1

#
# Core.
#
all:: $(STAGE_LOG_DIR) base-stage-jre new-stage-jre base-stage-sdk new-stage-sdk

clean: 
	$(RM) -r  $(STAGE_DIR)

# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Staging finished:  " $(FULL_VERSION)

#
# Phonies to avoid accidents.
#
.PHONY: all base-stage-jre new-stage-jre base-stage-sdk new-stage-sdk clean
