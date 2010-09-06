#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.13 03/12/19 
#
# This is the makefile for building the J2RE installer bundles after compiling
# the j2sdk sources. Listed below are some general notes in using this makefile.
#
# 1) Requires InstallShield Package for the Web 3 which is defined as $(IPFW).
# 2) Requires MKS Toolkit (should be first thing in your PATH).
# 3) $(TMPDIR) is the temporary building directory.
# 4) $(INSTALL_BUILDDIR) is 4 directories above this directory.
#
# Unique target : all
#


# get a standard build environment set...
INSTALL_BUILDDIR = ../../../..
PRODUCT = xpi

TMPDIR = $(TEMP_DIR)/$(PRODUCT)
PLUGIN_TMPDIR = $(TEMP_DIR)/deploy/plugin

include $(INSTALL_BUILDDIR)/common/Defs.gmk

ifndef JDK_UPDATE_VERSION
  CONVERT_UPDATE = $(SED) -e "s/JDKUPDATE//g"
  UPDATENUMBER=0
else
  CONVERT_UPDATE = $(SED) -e "s/JDKUPDATE/_$(JDK_UPDATE_VERSION)/g"
  UPDATENUMBER=$(shell $(ECHO) $(JDK_UPDATE_VERSION) | $(NAWK) '{print substr($$0, 2, 1);}')
endif

CONVERT_MAJOR = $(SED) -e "s/JDKMAJOR/$(JDK_MAJOR_VERSION)/g"
CONVERT_MINOR = $(SED) -e "s/JDKMINOR/$(JDK_MINOR_VERSION)/g"
CONVERT_MICRO = $(SED) -e "s/JDKMICRO/$(JDK_MICRO_VERSION)/g"

PROCESS_FILES = $(CONVERT_MAJOR) | $(CONVERT_MINOR) | $(CONVERT_MICRO) | $(CONVERT_UPDATE)

ifndef VERSION
  VERSION=$(RELEASE).0
endif

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the installation scripts
# and jre-image directory to the TEMP directory so we can delete/add
# files between the English and International installations. Do the two
# installations in succession.
# 
all: $(TMPDIR) $(TMPDIR)/regular $(TMPDIR)/patch xpi

###################### clean #############################
#
# Remove all produced files.
#
clean: 
	$(RM) $(BIN_BUNDLEDIR)/$(JRE_NAME)*.xpi
	$(RM) -r $(TMPDIR)

$(BIN_BUNDLEDIR):
	$(MKDIR) -p $@

$(TMPDIR):
	$(MKDIR) -p $@

$(TMPDIR)/regular:
	$(MKDIR) -p $@

$(TMPDIR)/patch:
	$(MKDIR) -p $@

###################### xpi ############################
#
# Create english bundle through its dependency. Copy the bundle to the 
# output directory.
#
ifneq ($(MILESTONE),fcs)
XPI_MILESTONE = -$(MILESTONE)
endif


CONVERT_MILESTONE = $(SED) -e "s/MILESTONE/$(XPI_MILESTONE)/g"
CONVERT_PLATFORM = $(SED) -e "s/PLATFORM/$(PLATFORM)/g"
CONVERT_ARCH = $(SED) -e "s/ARCH/$(ARCH)/g"
CONVERT_HTTPSERVER = $(SED) -e "s@HTTP_SERVER@$(HTTPSERVER)@g"
CONVERT_FULLVERSION = $(SED) -e "s@FULL_VERSION@$(FULL_VERSION)@g"

INSTALL_JS_PROCESS = $(PROCESS_FILES) | $(CONVERT_MILESTONE) | $(CONVERT_PLATFORM) | $(CONVERT_ARCH) | $(CONVERT_FULLVERSION) | $(CONVERT_HTTPSERVER)

xpi: $(BIN_BUNDLEDIR) install.js
	$(CP) $(PLUGIN_TMPDIR)/jinstall/obj/jinstall.exe $(TMPDIR)/regular/jinstall.exe
	@#
	$(CAT) install.js | $(INSTALL_JS_PROCESS) > $(TMPDIR)/regular/install.js
	@#
	$(CD) $(TMPDIR)/regular; $(ZIPEXE) $(BIN_BUNDLEDIR)/$(JRE_NAME)-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE).xpi \
	jinstall.exe install.js
	$(RM) -f $(TMPDIR)/regular/install.js $(TMPDIR)/jinstall.exe
.PHONY: all clean xpi
