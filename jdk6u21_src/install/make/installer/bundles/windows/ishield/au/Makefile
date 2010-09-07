#
# Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.10 10/03/29
#
# This is the makefile for building the standalong AU 2.0 MSI installer
#

# get a standard build environment set...
INSTALL_BUILDDIR=../../../../..
PRODUCT = ishield

include $(INSTALL_BUILDDIR)/common/Defs.gmk

#set path names
TEMPDIR = $(TMPISHIELD_DIR)/au

CURRENTDIR = $(shell $(PWD))
TMPIMAGE = $(TEMPDIR)/au

SED_BINARY_LOCATION = $(SED) -e 's%BINARYPROJECT%$(subst /,\\,$(TEMPDIR)/au.ism)%g'
SED_TEXT_LOCATION = $(SED) -e 's%TEXTPROJECT%$(subst /,\\,$(TEMPDIR)/au.isv)%g'

INSTALLERFILESAU=$(AUISHIELDDIR)/au/au/DiskImages/Disk1/

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the jre-image
# to the temp directory, setup the ishield project, and build

all: $(TEMPDIR) setup_installer_files create_bundle

###################### clean #############################
#
# Remove all produced files.
#
clean: 
	$(RM) -r $(TEMPDIR)

$(TEMPDIR):
	$(MKDIR) -p $(TEMPDIR)


AUISHIELDDIR = $(TEMPDIR)/au

$(AUISHIELDDIR):
	$(MKDIR) -p $@

###################### create_bundle ###################### 
# 
# Create the install project, sign the MSI, and build the release
#
create_bundle: 
	$(RM) $(TEMPDIR)/au.ism
	$(RM) $(TEMPDIR)/*.exe
	$(CP) $(INSTALL_BUILDDIR)/../../deploy/src/plugin/win32/jupdate/aucheck/Release/jaucheck.exe $(TEMPDIR)/
	$(CP) $(INSTALL_BUILDDIR)/../../deploy/src/plugin/win32/jupdate/jaureg/Release/jaureg.exe $(TEMPDIR)/
	$(CP) $(ALT_OUTPUTDIR)/bin/jucheck.exe $(TEMPDIR)/
	$(CP) $(ALT_OUTPUTDIR)/bin/jusched.exe $(TEMPDIR)/
ifeq ($(SIGN_FLAG),1)
	$(MSISIGNTOOLDIR)/signtool.exe sign /n "Sun Microsystems, Inc." /d "Java Auto Updater" /du http://java.com /t http://timestamp.verisign.com/scripts/timstamp.dll $(TEMPDIR)/jusched.exe
	$(MSISIGNTOOLDIR)/signtool.exe sign /n "Sun Microsystems, Inc." /d "Java Auto Updater" /du http://java.com /t http://timestamp.verisign.com/scripts/timstamp.dll $(TEMPDIR)/jucheck.exe
	$(MSISIGNTOOLDIR)/signtool.exe sign /n "Sun Microsystems, Inc." /d "Java Auto Updater" /du http://java.com /t http://timestamp.verisign.com/scripts/timstamp.dll $(TEMPDIR)/jaureg.exe
	$(MSISIGNTOOLDIR)/signtool.exe sign /n "Sun Microsystems, Inc." /d "Java Auto Updater" /du http://java.com /t http://timestamp.verisign.com/scripts/timstamp.dll $(TEMPDIR)/jaucheck.exe
endif
	$(CP) $(ALT_OUTPUTDIR)/j2re-image/lib/task*.xml $(TEMPDIR)/
	$(WSCRIPT) $(TEMPDIR)/au.vbs
	$(CP) $(TEMPDIR)/../../deploy/plugin/auutils/obj/auutils.dll $(TEMPDIR)/
	$(BUILDER) -p $(TEMPDIR)/au.ism -r "au" -c COMP
	$(MV) $(INSTALLERFILESAU)/*.msi $(INSTALLERFILESAU)/au.msi
	$(MV) $(INSTALLERFILESAU)/*.cab $(INSTALLERFILESAU)/au.cab
ifeq ($(SIGN_FLAG),1)
	$(MSISIGNTOOLDIR)/signtool.exe sign /n "Sun Microsystems, Inc." /d "Java Auto Updater" /du http://java.com /t http://timestamp.verisign.com/scripts/timstamp.dll $(INSTALLERFILESAU)/au.cab
	$(MSISIGNTOOLDIR)/msicert.exe -d $(INSTALLERFILESAU)/au.msi -m 1 -c $(INSTALLERFILESAU)/au.cab
	$(MSISIGNTOOLDIR)/signtool.exe sign /n "Sun Microsystems, Inc." /d "Java Auto Updater" /du http://java.com /t http://timestamp.verisign.com/scripts/timstamp.dll $(INSTALLERFILESAU)/au.msi
endif
	$(MKDIR) -p $(TMPISHIELD_DIR)/patch/ 
	$(CP) $(INSTALLERFILESAU)/au.msi $(TMPISHIELD_DIR)/patch/ 
	$(CP) $(INSTALLERFILESAU)/au.cab $(TMPISHIELD_DIR)/patch/ 
	$(RM) $(INSTALLERFILESAU)/*.exe $(INSTALLERFILESAU)/1033.mst
	$(CD) $(INSTALLERFILESAU); $(ZIPEXE) -r $(BIN_BUNDLEDIR)/au-$(JRE_BUNDLE_NAME).zip *

setup_installer_files: $(AUISHIELDDIR)
	$(CP) $(CURRENTDIR)/au.isv $(TEMPDIR)
	$(RM) -r $(AUISHIELDDIR)/*
	$(CD) $(CURRENTDIR)/au/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(AUISHIELDDIR)/
	$(CHMOD) 777 $(AUISHIELDDIR)/*
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) | $(SED_AUTOMATION_LAYER) > $(TEMPDIR)/au.vbs

.PHONY: clean all create_bundle setup_installer_files
