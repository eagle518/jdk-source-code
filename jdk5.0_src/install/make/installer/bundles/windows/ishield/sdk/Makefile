#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.74 04/06/16
#
# This is the makefile for building the J2SDK installer bundle after compiling
# the j2sdk sources. Listed below are some general notes in using this makefile.
#
# 1) Requires InstallShield Developer defined as $(ALT_ISHIELDDIR).
# 2) Requires InstallShield Developer lang packs East and West
# 3) Requires MKS Toolkit (should be first thing in your PATH).
# 5) $(TMPDIR) is the temporary building directory.
# 6) $(BASE_IMAGE_SDK_DIR) is the base J2SDK image.
# 7) $(NEW_IMAGE_SDK_DIR) is the new J2SDK image.
# 8) $(INSTALL_BUILDDIR) is 5 directories above this directory.
# 9) $(CURRENTDIR) is this directory.
#
# Unique target : all
#

# get a standard build environment set...
INSTALL_BUILDDIR=../../../../..
PRODUCT = ishield

include $(INSTALL_BUILDDIR)/common/Defs.gmk
include $(INSTALL_BUILDDIR)/common/Pack.gmk

#set path names
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
  TMPDIR = $(TMPISHIELD_DIR)/patch
else
  TMPDIR = $(TMPISHIELD_DIR)/regular
endif

#This is the disk space we reserve for the installed image in bytes
#It will be used in the ReserveCost table
ifdef JDK_UPDATE_VERSION
  TOOLSSPACEVALUE=133000000
  DEMOSSPACEVALUE=29000000
  SRCSPACEVALUE=33000000
else
  TOOLSSPACEVALUE=87000000
  DEMOSSPACEVALUE=13000000
  SRCSPACEVALUE=11000000
endif

#This is what will display in text for the disk space in MB that each
#feature requires when selected.  It includes both update repository 
#and installed image.  It should eventually be auto-computed during 
#the build by checking the size of the .zip files for each feature
ifdef JDK_UPDATE_VERSION
  TOOLSSPACETEXT=264
  DEMOSSPACETEXT=38
  SRCSPACETEXT=49
  PUBJRESPACETEXT=161
else
  TOOLSSPACETEXT=132
  DEMOSSPACETEXT=24
  SRCSPACETEXT=27
  PUBJRESPACETEXT=98
endif


CURRENTDIR = $(shell $(PWD))
TMPSDKIMAGE = $(TMPDIR)/sdk-image
TMPPACKED_JDK_JARDIR = $(TMPDIR)/packed-sdk-jars

SED_TOOLS_RESERVE=$(SED) -e 's%TOOLSSPACE%$(TOOLSSPACEVALUE)%g'
SED_DEMO_RESERVE=$(SED) -e 's%DEMOSPACE%$(DEMOSSPACEVALUE)%g'
SED_SRC_RESERVE=$(SED) -e 's%SRCSPACE%$(SRCSPACEVALUE)%g'
SED_ALL_RESERVE=$(SED_TOOLS_RESERVE) | $(SED_DEMO_RESERVE) |$(SED_SRC_RESERVE)

SED_TOOLS_TEXT=$(SED) -e 's%TOOLSSPACETEXT%$(TOOLSSPACETEXT)%g'
SED_DEMO_TEXT=$(SED) -e 's%DEMOSSPACETEXT%$(DEMOSSPACETEXT)%g'
SED_SRC_TEXT=$(SED) -e 's%SRCSPACETEXT%$(SRCSPACETEXT)%g'
SED_PUBJRE_TEXT=$(SED) -e 's%PUBJRESPACETEXT%$(PUBJRESPACETEXT)%g'
SED_ALL_TEXT=$(SED_TOOLS_TEXT) | $(SED_DEMO_TEXT) | $(SED_SRC_TEXT) | $(SED_PUBJRE_TEXT)

SED_BINARY_LOCATION = $(SED) -e 's%BINARYPROJECT%$(subst /,\\,$(TMPDIR)/sdk.ism)%g'
SED_TEXT_LOCATION = $(SED) -e 's%TEXTPROJECT%$(subst /,\\,$(TMPDIR)/sdk.isv)%g'
SED_CAB_LOCATION = $(SED) -e 's%CABLOCATION%$(SDK_CAB_URL)%g'
SED_DOWNLOAD_TEXT = $(SED) -e 's%BUNDLETYPE%DL%g'
SED_COPY_TEXT = $(SED) -e 's%BUNDLETYPE%CP%g'

INSTALLERFILE=$(SDKISHIELDDIR)/sdk/singleexe/DiskImages/Disk1/setup.exe
INSTALLERFILES=$(SDKISHIELDDIR)/sdk/iftw/DiskImages/Disk1/
INSTALLERMSIFILE=$(SDKISHIELDDIR)/sdk/singlemsi/DiskImages/Disk1/*.msi

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the sdk-image
# to the temp directory, setup the ishield 7 project, and build
# 
all: $(TMPDIR) setup_image setup_installer_files create_bundle 

###################### clean #############################
#
# Remove all produced files.
#
clean: 
	$(RM) -r $(TMPDIR)

$(TMPDIR):
	$(MKDIR) -p $(TMPDIR)

$(TMPDIR)/setup.bmp:
	$(CP) $(INSTALL_BUILDDIR)/installer/bundles/$(PLATFORM)/ishield/setup.bmp $@

ifeq ($(MILESTONE), fcs)
  LICENSEEND=fcs
else
  LICENSEEND=pre
endif

copy_licenses:
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jdk/LICENSE.rtf.$(LICENSEEND) $(TMPDIR)/sdk-license.rtf

SDKISHIELDDIR = $(TMPDIR)/sdk

$(SDKISHIELDDIR):
	$(MKDIR) -p $@

ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
    SDK_BUNDLE_NAME = jdk-$(BIN_BUNDLE_NAMEPART)-p-$(BUNDLE_DATE)
    SDK_IFTW_BUNDLE_NAME = $(JDK_SHORT_BUNDLE_NAME)-p-iftw
else
    SDK_BUNDLE_NAME = jdk-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)
    SDK_IFTW_BUNDLE_NAME = $(JDK_SHORT_BUNDLE_NAME)-iftw
endif

ZIPPER = $(TMPDIR)/zipper$(EXE_SUFFIX)

$(ZIPPER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.zipper/$(OBJDIRNAME)/zipper$(EXE_SUFFIX) $@

LAUNCHER = $(TMPDIR)/launcher$(EXE_SUFFIX)

$(LAUNCHER):
	$(CP) $(OUTPUTDIR)/tmp/sun.tools.launcher/$(OBJDIRNAME)/launcher$(EXE_SUFFIX) $@

COPIER = $(TMPDIR)/copier$(EXE_SUFFIX)

$(COPIER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.copier/$(OBJDIRNAME)/copier$(EXE_SUFFIX) $@

TOUCHFILE = $(TMPDIR)/touchfile$(EXE_SUFFIX)

$(TOUCHFILE):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.touchfile/$(OBJDIRNAME)/touchfile$(EXE_SUFFIX) $@

PATCHER = $(TMPDIR)/patchsdk$(EXE_SUFFIX)

$(PATCHER):
# 
# Use stub patcher to reduce patcher size for download if no patch should be applied.
#
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	$(CP) $(ALT_OUTPUTDIR)/tmp/patcher/jdk/$(OBJDIRNAME)/jdk-patcher$(EXE_SUFFIX) $@
else
	$(CP) $(ALT_OUTPUTDIR)/tmp/patcher/stub/$(OBJDIRNAME)/stub-patcher$(EXE_SUFFIX) $@
endif


SUNGRAPHIC1 = $(TMPDIR)/IsDialogBanner.ibd

$(SUNGRAPHIC1):
	$(CP) $(CURRENTDIR)/../IsDialogBanner.ibd $@

SUNGRAPHIC2 = $(TMPDIR)/IsDialogBitmap.ibd

$(SUNGRAPHIC2):
	$(CP) $(CURRENTDIR)/../IsDialogBitmap.ibd $@


###################### create_bundle ###################### 
# 
# Create the install project and build the release
#
create_bundle: copy_licenses $(TMPDIR)/setup.bmp $(SUNGRAPHIC1) $(SUNGRAPHIC2) $(LAUNCHER)
	$(MAKE) singleexe
ifeq ($(ARCH_DATA_MODEL),32)
	$(MAKE) iftw
endif

singleexe:
ifeq ($(ARCH), amd64)
	-$(BUILDER) -p $(TMPDIR)/sdk.ism -r "singleexe" -c COMP
else
	$(BUILDER) -p $(TMPDIR)/sdk.ism -r "singleexe" -c COMP
endif
ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "J2SE Development Kit 1.5.0" -i http://java.sun.com/ $(INSTALLERFILE)
endif
	$(CP) $(INSTALLERFILE) $(BIN_BUNDLEDIR)/$(SDK_BUNDLE_NAME).exe

iftw:
	$(CAT) $(CURRENTDIR)/sdk/ISSetupFile.idt | $(GREP) -v setup.bmp > $(SDKISHIELDDIR)/ISSetupFile.idt
	$(CAT) $(CURRENTDIR)/sdk/Binary.idt | $(GREP) -v ISSetupFilesHelper > $(SDKISHIELDDIR)/Binary.idt
	$(CAT) $(CURRENTDIR)/sdk/InstallUISequence.idt | $(SED_FULL_VERSION) | $(GREP) -v ISSetupFilesExtract | $(GREP) -v ISSetupFilesCleanup > $(SDKISHIELDDIR)/InstallUISequence.idt
	$(CAT) $(CURRENTDIR)/sdk/InstallExecuteSequence.idt | $(GREP) -v ISSetupFilesExtract | $(GREP) -v ISSetupFilesCleanup > $(SDKISHIELDDIR)/InstallExecuteSequence.idt
	$(CAT) $(CURRENTDIR)/sdk/CustomAction.idt | $(GREP) -v ISSetupFilesExtract | $(GREP) -v ISSetupFilesCleanup | $(SED_FULL_VERSION) | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/CustomAction.idt
	$(RM) $(TMPDIR)/sdk.ism
	$(WSCRIPT) $(TMPDIR)/sdkfile.vbs
	$(BUILDER) -p $(TMPDIR)/sdk.ism -r "singlemsi" -c COMP
	$(CAT) $(CURRENTDIR)/sdk/ActionText.idt | $(SED_DOWNLOAD_TEXT) > $(SDKISHIELDDIR)/ActionText.idt
	$(RM) $(TMPDIR)/sdk.ism
	$(WSCRIPT) $(TMPDIR)/sdkfile.vbs
	$(BUILDER) -p $(TMPDIR)/sdk.ism -r "iftw" -c COMP
	$(RM) $(TMPDIR)/setup.exe
	for file in `$(LS) -p $(INSTALLERFILES)/*.mst ` ; do \
		mstbase=`$(BASENAME) $$file` ; \
		MSTbase=`$(ECHO) $$mstbase | $(NAWK) '{ print toupper($1) }'` ; \
		$(CD) $(INSTALLERFILES) ; $(MV) $$mstbase `$(ECHO) "jdk"`$$MSTbase || exit 1; $(RM) jdk1033.MST ; \
	done 	
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/deploy/plugin/jinstall_sdk/obj/jinstall_sdk.exe $(INSTALLERFILES)/$(SDK_IFTW_BUNDLE_NAME).exe
ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "J2SE Development Kit 1.5.0" -i http://java.sun.com/ $(INSTALLERFILES)/$(SDK_IFTW_BUNDLE_NAME).exe
endif
	$(RM) $(INSTALLERFILES)/setup.exe
	$(MV) $(INSTALLERFILES)/*.msi $(INSTALLERFILES)/jdk$(JDK_VERSION)-iftw.msi
	$(CP) $(INSTALLERMSIFILE) $(INSTALLERFILES)/jdk$(JDK_VERSION).msi
ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "JDK 1.5.0 Installer" -i http://java.sun.com/ $(INSTALLERFILES)/jdk$(JDK_VERSION).msi
endif
	$(CD) $(INSTALLERFILES) ; $(ZIPEXE) -r $(BIN_BUNDLEDIR)/$(SDK_BUNDLE_NAME).zip *

ifeq ($(ARCH_DATA_MODEL),32)
setup_installer_files: $(COPIER) $(PATCHER) $(SDKISHIELDDIR)
else
setup_installer_files: $(SDKISHIELDDIR)
endif
	$(CP) $(CURRENTDIR)/sdk.isv $(TMPDIR)
	$(CD) $(CURRENTDIR)/sdk/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(SDKISHIELDDIR)/
	$(CHMOD) 777 $(SDKISHIELDDIR)/*
	$(CAT) $(CURRENTDIR)/sdk/InstallUISequence.idt | $(SED_FULL_VERSION) > $(SDKISHIELDDIR)/InstallUISequence.idt
	$(SED_CAB_LOCATION) $(CURRENTDIR)/sdk/ISReleaseExtended.idt > $(SDKISHIELDDIR)/ISReleaseExtended.idt
	$(CAT) $(CURRENTDIR)/sdk/Registry.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/Registry.idt
	$(CAT) $(CURRENTDIR)/sdk/Property.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_FULL_VERSION) | $(SED_ALL_TEXT) > $(SDKISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/sdk/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) > $(SDKISHIELDDIR)/_SummaryInformation.idt
	$(SED_FULL_VERSION) $(CURRENTDIR)/sdk/Directory.idt | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(SDKISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/sdk/File.idt | $(SED_FULL_VERSION) | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/File.idt
	$(CAT) $(CURRENTDIR)/sdk/DuplicateFile.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/DuplicateFile.idt
	$(CAT) $(CURRENTDIR)/sdk/Component.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/sdk/ReserveCost.idt | $(PROCESS_FILES_COMP) | $(SED_ALL_RESERVE) > $(SDKISHIELDDIR)/ReserveCost.idt
	$(CAT) $(CURRENTDIR)/sdk/FeatureComponents.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/FeatureComponents.idt
	$(CAT) $(CURRENTDIR)/sdk/ISComponentExtended.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/ISComponentExtended.idt
	$(CAT) $(CURRENTDIR)/sdk/CustomAction.idt | $(SED_FULL_VERSION) |  $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/CustomAction.idt
	$(CAT) $(CURRENTDIR)/sdk/ISString.idt | $(SED_FULL_VERSION) > $(SDKISHIELDDIR)/ISString.idt
	$(CAT) $(CURRENTDIR)/sdk/ActionText.idt | $(SED_COPY_TEXT) > $(SDKISHIELDDIR)/ActionText.idt
ifneq ($(ARCH_DATA_MODEL),64)
	$(MAKE) create_removefile
	$(CP) $(TMPDIR)/removefile $(SDKISHIELDDIR)/RemoveFile.idt
	$(MAKE) create_directory
	$(CAT) $(TMPDIR)/directory >> $(SDKISHIELDDIR)/Directory.idt
endif
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) | $(SED_AUTOMATION_LAYER) > $(TMPDIR)/sdkfile.vbs
ifeq ($(ARCH_DATA_MODEL),64)
	$(CP) -fr $(CURRENTDIR)/64bit/* $(SDKISHIELDDIR)/
	$(SED_FULL_VERSION) $(CURRENTDIR)/64bit/Directory.idt | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(SDKISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/64bit/Component.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/64bit/Property.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_FULL_VERSION) | $(SED_ALL_TEXT) > $(SDKISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/64bit/ISComponentExtended.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/ISComponentExtended.idt
	$(CAT) $(CURRENTDIR)/64bit/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) > $(SDKISHIELDDIR)/_SummaryInformation.idt
  ifeq ($(ARCH), amd64)
	$(CAT) $(CURRENTDIR)/64bit/_SummaryInformation.idt.amd64 | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) > $(SDKISHIELDDIR)/_SummaryInformation.idt
	regsvr32.exe /s $(ISHIELDDIR)/System/ISWIAutomation.dll
  endif
	$(RM) -rf $(SDKISHIELDDIR)/SCCS
endif
	$(WSCRIPT) $(TMPDIR)/sdkfile.vbs

#
# Copy base image to be bundled with installer.
#
ifeq ($(ARCH_DATA_MODEL),32)
setup_image: $(ZIPPER) 
else
setup_image:
endif
	@$(ECHO) Cleaning old temporary install image
	$(CHMOD) -Rf 777 $(TMPSDKIMAGE)
	$(RM) -r $(TMPSDKIMAGE)
	$(MKDIR) -p $(TMPSDKIMAGE)
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@#
	@# Patch build - copy cabs from old base image repository
	@#
	$(CP) -r $(ALT_BASE_IMAGE_DIR)/jdk-repository-image/*.zip $(TMPSDKIMAGE)	
	$(CP) $(NEW_IMAGE_SDK_DIR)/COPYRIGHT $(TMPSDKIMAGE)
else
	$(CP) -r $(NEW_IMAGE_SDK_DIR)/* $(TMPSDKIMAGE)
	$(RM) -r $(TMPPACKED_JDK_JARDIR)
	$(MKDIR) -p $(TMPPACKED_JDK_JARDIR)
	$(MAKE) do_pack
ifeq ($(ARCH_DATA_MODEL),32)
	$(MAKE) do_zip
endif
endif

do_pack:
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@#we pack/unpack the BASE_IMAGE_SDK_DIR, extra flag to keep debug info
	cd $(INSTALL_BUILDDIR)/pack; \
	$(MAKE) PACK_OUTPUTDIR=$(TMPPACKED_JDK_JARDIR) \
		VAR_IMAGE_DIR=$(TMPSDKIMAGE) \
		JARS_TO_PACK="$(JDK_PACKED_JARS)" \
		PACK_FLAGS+="-C -D" \
		pack-jars
	for i in $(JDK_PACKED_JARS) ; do \
	    if [ -f $(BASE_IMAGE_SDK_DIR)/$$i ]; then \
		$(RM) $(TMPSDKIMAGE)/$$i || exit 1; \
		$(CP) $(TMPPACKED_JDK_JARDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPSDKIMAGE)/`$(DIRNAME) $$i` || exit 1; \
	    fi ; \
	done
else
	for i in $(JDK_PACKED_JARS) ; do \
	    $(RM) $(TMPSDKIMAGE)/$$i || exit 1; \
	    $(CP) $(ALT_OUTPUTDIR)/pack/pack-jdk-jars/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPSDKIMAGE)/`$(DIRNAME) $$i` || exit 1; \
	done
endif

##########################################################
#
# This is a workaround.  Install shield 7.04 makes creates
# a component for each sub-dir of our product, and therefore
# a .cab file for each component.  By zipping w/no compression
# we eliminate sub-dirs, and have less .cab files to host
do_zip: $(ZIPPER) $(TOUCHFILE) 
	$(MV) $(TMPSDKIMAGE)/src.zip $(TMPSDKIMAGE)/demo $(TMPSDKIMAGE)/COPYRIGHT $(TMPDIR)
	$(MV) $(TMPSDKIMAGE)/jre/bin/server  $(TMPDIR)
	$(RM) -rf $(TMPDIR)/privatejrejars
	$(MKDIR) $(TMPDIR)/privatejrejars
	for i in $(subst jar,pack,$(JRE_PACKED_JARS)); do \
	    $(MV) $(TMPSDKIMAGE)/jre/$$i $(TMPDIR)/privatejrejars || exit 1; \
	done
	$(RM) -rf $(TMPSDKIMAGE)/jre
	$(MKDIR) -p $(TMPSDKIMAGE)/jre/bin/ $(TMPSDKIMAGE)/jre/lib/ext
	mv $(TMPDIR)/server $(TMPSDKIMAGE)/jre/bin/
	for i in $(subst jar,pack,$(JRE_PACKED_JARS)) ; do \
	    $(MV) $(TMPDIR)/privatejrejars/`$(BASENAME) $$i` $(TMPSDKIMAGE)/jre/`$(DIRNAME) $$i`/sdk.`$(BASENAME) $$i` || exit 1; \
	done
	$(CD) $(TMPSDKIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/tools.zip *
	$(RM) -rf $(TMPSDKIMAGE)/*
	$(MV) $(TMPDIR)/demo $(TMPSDKIMAGE)
	$(CD) $(TMPSDKIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/demos.zip *
	$(RM) -rf $(TMPSDKIMAGE)/*
	$(MV) $(TMPDIR)/tools.zip $(TMPDIR)/demos.zip $(TMPDIR)/src.zip $(TMPDIR)/COPYRIGHT $(TMPSDKIMAGE)
	@#
	@# Touch all the zip files in the directory and set the creation date
	@# to arbitary. This is important because InstallShield will only generate the 
	@# same base image CAB if and only if the file content are the same and the file
	@# creation date are the same.
	@#
	@# Notice that the arbitary date must be the same in the base image across
	@# releases, or the Install-On-Demand may trigger base image to be downloaded
	@# even if base image exists on the system. Currently, the arbitary file creation
	@# date is 01/01/2003 8:00am. See #4796128.
	@#
	@#$(TOUCHFILE) $(TMPSDKIMAGE)

create_removefile:
	$(RM) $(TMPDIR)/removefile
	$(CAT) $(SDKISHIELDDIR)/RemoveFile.idt | $(PROCESS_FILES_COMP) > $(TMPDIR)/removefile
	for dir in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) * -type d` ; do \
	  dirkey=`$(ECHO) $$dir | $(TR) -d "/" | $(TR) -d "-"` ; \
	  if [ "$$dirkey" != "jrelibext" ]; then \
	    dirsafe=`$(ECHO) $$dirkey | $(TR) - j | $(TR) + k | $(TR) $$ l ` ; \
	    $(PRINTF) "$$dirsafe\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t\t$$dirkey\t3\n" >> $(TMPDIR)/removefile || exit 1; \
	    $(PRINTF) "f$$dirsafe\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t*\t$$dirkey\t3\n" >> $(TMPDIR)/removefile || exit 1; \
	  fi ; \
	done;

create_directory:
	$(RM) $(TMPDIR)/directory
	$(CAT) $(JREISHIELDDIR)/Directory.idt | $(PROCESS_FILES_COMP) > $(TMPDIR)/directory
	for dir in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) . -name "*" -type d` ; do \
	  dirbase=`$(ECHO) $$dir | $(TR) -d "/" | $(TR) -d "." | $(TR) -d "-"` ; \
	  dirparent=`$(DIRNAME) $$dir | $(TR) -d "/" | $(TR) -d "."` ; \
	  LENGTH=`$(BASENAME) $$dir | $(NAWK) '{print length($$0);}'` ; \
	  if [ "$$dirparent" = "" ]; then \
	    dirparent=INSTALLDIR ; \
	  fi ; \
	  if [ "$$dirbase" != "" ]; then \
	    if [ "$$LENGTH" -gt "7" ]; then \
	      dirdir=`$(BASENAME) $$dir | $(NAWK) '{print substr($$0,0,6);}'``$(ECHO) "~1|"``$(BASENAME) $$dir` ; \
	    else \
	      dirdir=`$(BASENAME) $$dir` ; \
	    fi ; \
	    $(PRINTF) "$$dirbase\t$$dirparent\t$$dirdir\t\t0\n" >> $(TMPDIR)/directory || exit 1; \
	  fi ; \
	done;

.PHONY: clean all create_bundle setup_installer_files setup_image do_zip do_pack

