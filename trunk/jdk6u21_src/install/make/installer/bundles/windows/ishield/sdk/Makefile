#
# Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.143 10/05/27 
#
# This is the makefile for building the J2SDK installer bundle after compiling
# the j2sdk sources. Listed below are some general notes in using this makefile.
#
# 1) Requires InstallShield Developer defined as $(ALT_ISHIELDDIR).
# 2) Requires InstallShield Developer lang packs East and West
# 3) Requires MKS Toolkit (should be first thing in your PATH).
# 5) $(TEMPDIR) is the temporary building directory.
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

TEMPDIR = $(TMPISHIELD_DIR)/patch
VALIDATIONSCRIPTDIR = $(CURRENTDIR)/../jre


#This is the disk space we reserve for the installed image in bytes
#It will be used in the ReserveCost table
ifdef JDK_UPDATE_VERSION
  TOOLSSPACEVALUE=151000000
  DEMOSSPACEVALUE=33000000
  SRCSPACEVALUE=33000000
else
  TOOLSSPACEVALUE=105000000
  DEMOSSPACEVALUE=17000000
  SRCSPACEVALUE=11000000
endif

#This is what will display in text for the disk space in MB that each
#feature requires when selected.  It includes both update repository 
#and installed image.  It should eventually be auto-computed during 
#the build by checking the size of the .zip files for each feature
ifdef JDK_UPDATE_VERSION
  TOOLSSPACETEXT=300
  DEMOSSPACETEXT=46
  SRCSPACETEXT=49
  PUBJRESPACETEXT=173
else
  TOOLSSPACETEXT=168
  DEMOSSPACETEXT=32
  SRCSPACETEXT=27
  PUBJRESPACETEXT=110
endif
JAVADBSPACETEXT=25


CURRENTDIR := $(call FullPath,.)
TMPSDKIMAGE = $(TEMPDIR)/sdk-image
TMPPACKED_JDK_JARDIR = $(TEMPDIR)/packed-sdk-jars

SED_TOOLS_RESERVE=$(SED) -e 's%TOOLSSPACE%$(TOOLSSPACEVALUE)%g'
SED_DEMO_RESERVE=$(SED) -e 's%DEMOSPACE%$(DEMOSSPACEVALUE)%g'
SED_SRC_RESERVE=$(SED) -e 's%SRCSPACE%$(SRCSPACEVALUE)%g'
SED_ALL_RESERVE=$(SED_TOOLS_RESERVE) | $(SED_DEMO_RESERVE) |$(SED_SRC_RESERVE)

SED_TOOLS_TEXT=$(SED) -e 's%TOOLSSPACETEXT%$(TOOLSSPACETEXT)%g'
SED_DEMO_TEXT=$(SED) -e 's%DEMOSSPACETEXT%$(DEMOSSPACETEXT)%g'
SED_SRC_TEXT=$(SED) -e 's%SRCSPACETEXT%$(SRCSPACETEXT)%g'
SED_PUBJRE_TEXT=$(SED) -e 's%PUBJRESPACETEXT%$(PUBJRESPACETEXT)%g'
SED_JAVADB_TEXT=$(SED) -e 's%JAVADBSPACETEXT%$(JAVADBSPACETEXT)%g'
SED_ALL_TEXT=$(SED_TOOLS_TEXT) | $(SED_DEMO_TEXT) | $(SED_SRC_TEXT) | $(SED_PUBJRE_TEXT) | $(SED_JAVADB_TEXT)

SED_BINARY_LOCATION = $(SED) -e 's%BINARYPROJECT%$(subst /,\\,$(TEMPDIR)/sdk.ism)%g'
SED_TEXT_LOCATION = $(SED) -e 's%TEXTPROJECT%$(subst /,\\,$(TEMPDIR)/sdk.isv)%g'
SED_COPY_TEXT = $(SED) -e 's%BUNDLETYPE%CP%g'

SED_MARKETING_VERSION = $(SED) -e 's%MARKETINGVERSION%$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)%g'

ifdef JDK_UPDATE_VERSION
  MARKETING_NUMBER := $(shell \
	$(ECHO) $(JDK_UPDATE_VERSION) | $(NAWK) '{if (substr($$0,1,1)=="0") print substr($$0, 2); else print $$0;}')
  MARKET_NAME= $(shell $(ECHO) " Update $(MARKETING_NUMBER)")
else
  MARKET_NAME=
endif

SED_MARKETING_UPDATE = $(SED) -e 's%MARKETNAME%$(MARKET_NAME)%g'
SED_MSI_UPDATE = $(SED) -e 's%MSIVERSION%$(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER)%g'


###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the sdk-image
# to the temp directory, setup the ishield 7 project, and build
# 
all: $(TEMPDIR) setup_image setup_installer_files create_bundle 

###################### clean #############################
#
# Remove all produced files.
#
clean: 
	$(RM) -r $(TEMPDIR)

$(TEMPDIR):
	$(MKDIR) -p $(TEMPDIR)

## Custom Revision milestone checking
## Point Custom rev will have 'revXXXXnn' as MILESTONE value
##    where XXXX is the stock/other symbol unique for customer
##    where nn is the 2 digit numeric number for point release
## See 6908562 for detail

ifeq ($(shell $(ECHO) $(MILESTONE) | $(CUT) -c1-3), rev)
    MILESTONE_CHECK=rev
else
    MILESTONE_CHECK=$(MILESTONE)
endif

$(SDKISHIELDDIR):
	$(MKDIR) -p $@

SDK_BUNDLE_NAME = $(JDK_NAME)-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)

ZIPPER = $(TEMPDIR)/zipper$(EXE_SUFFIX)

$(ZIPPER):
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/sun.tools.zipper/$(OBJDIRNAME)/zipper$(EXE_SUFFIX) $@

LAUNCHER = $(ALT_OUTPUTDIR)/tmp/sun.tools.launcher/$(OBJDIRNAME)/launcher$(EXE_SUFFIX)

TOUCHFILE = $(TEMPDIR)/touchfile$(EXE_SUFFIX)

$(TOUCHFILE):
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/sun.tools.touchfile/$(OBJDIRNAME)/touchfile$(EXE_SUFFIX) $@

SUNGRAPHIC3 = $(TEMPDIR)/white_background.ibd

$(SUNGRAPHIC3):
	$(CP) -f $(CURRENTDIR)/../white_background.ibd $@

SUNGRAPHIC4 = $(TEMPDIR)/IsDialogBannerRed.ibd

$(SUNGRAPHIC4):
	$(CP) -f $(CURRENTDIR)/../IsDialogBannerRed.ibd $@

JAVADB_MSI = $(TEMPDIR)/javadb.msi

$(JAVADB_MSI):
	$(CP) -f $(COPKG_JAVADB_DIR)/windows/javadb.msi $@


###################### create_bundle ###################### 
# 
# Create the install project and build the release
#
ifeq ($(ARCH_DATA_MODEL),32)
create_bundle: $(SUNGRAPHIC3) $(SUNGRAPHIC4) $(JAVADB_MSI)
else
create_bundle: $(SUNGRAPHIC1) $(SUNGRAPHIC2) $(SUNGRAPHIC3) $(SUNGRAPHIC4) 
endif
ifeq ($(ARCH), amd64)
	# to workaround CR 6670079 until CR 6657295 can be fixed, copy in launcher.exe
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/sun.tools.launcher/$(OBJDIRNAME)/launcher$(EXE_SUFFIX) $(TEMPDIR)
endif
	# install\make\common\Defs-windows.gmk puts the '-' before the BUILDER command on amd64
	$(BUILDER) -p $(TEMPDIR)/sdk.ism -r "offline" -c COMP
ifeq ($(ARCH), amd64)
	$(CSCRIPT) $(VALIDATIONSCRIPTDIR)/Wi64bitValidation.vbs $(JDKINSTALLERMSIFILE)
endif
	for file in `$(LS) -p $(JDKINSTALLERFILESDIR)/*.mst ` ; do \
		mstbase=`$(BASENAME) $$file` ; \
		MSTbase=`$(ECHO) $$mstbase | $(NAWK) '{ print toupper($$1) }'` ; \
		$(CD) $(JDKINSTALLERFILESDIR) ; $(MV) $$mstbase `$(ECHO) "jdk"`$$MSTbase || exit 1; $(RM) jdk1033.MST ; \
	done 	
	$(DEVTOOLS_PATH)/msitran.exe -a $(CURRENTDIR)/is.mst $(JDKINSTALLERMSIFILE)
	$(MV) -f $(JDKINSTALLERMSIFILE) $(JDKINSTALLERFILESDIR)/jdk.msi
	$(MSIVAL2) $(JDKINSTALLERFILESDIR)/jdk.msi $(LOGOCUB) -f

setup_installer_files: $(SDKISHIELDDIR)
	$(CP) -f $(CURRENTDIR)/sdk.isv $(TEMPDIR)
	$(CD) $(CURRENTDIR)/sdk/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(SDKISHIELDDIR)/
	$(CHMOD) 777 $(SDKISHIELDDIR)/*
	$(CAT) $(CURRENTDIR)/sdk/InstallUISequence.idt | $(SED_FULL_VERSION) > $(SDKISHIELDDIR)/InstallUISequence.idt
	$(CAT) $(CURRENTDIR)/sdk/InstallExecuteSequence.idt | $(GREP_DEPLOY_ACTIONS) > $(SDKISHIELDDIR)/InstallExecuteSequence.idt
	$(CAT) $(CURRENTDIR)/sdk/ISReleaseExtended.idt > $(SDKISHIELDDIR)/ISReleaseExtended.idt
	$(CAT) $(CURRENTDIR)/sdk/Registry.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/Registry.idt
ifeq ($(ARCH_DATA_MODEL),64)
	$(CAT) $(CURRENTDIR)/sdk/RegLocator.idt | $(AWK_RegLocator_Type) > $(SDKISHIELDDIR)/RegLocator.idt
endif
	$(CAT) $(CURRENTDIR)/sdk/Property.idt | $(AWK_ProductName) | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_FULL_VERSION) | $(SED_ALL_TEXT) | $(SED_MARKETING_UPDATE) | $(SED_MSI_UPDATE) > $(SDKISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/sdk/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_MARKETING_VERSION) | $(SED_HARDWARE) > $(SDKISHIELDDIR)/_SummaryInformation.idt
	$(SED_FULL_VERSION) $(CURRENTDIR)/sdk/Directory.idt | $(AWK_Directory_ProgramFilesFolder) | $(AWK_Directory_CommonFilesFolder) | $(PROCESS_FILES_COMP) | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(SDKISHIELDDIR)/Directory.idt
	$(SED_FULL_VERSION) $(CURRENTDIR)/sdk/RemoveFile.idt | $(PROCESS_FILES_COMP) | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(SDKISHIELDDIR)/RemoveFile.idt
	$(CAT) $(CURRENTDIR)/sdk/File.idt | $(SED_FULL_VERSION) | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/File.idt
ifeq ($(ARCH_DATA_MODEL),32)
	$(CAT) $(CURRENTDIR)/sdk/DuplicateFile.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/DuplicateFile.idt
endif
	$(CAT) $(CURRENTDIR)/sdk/Component.idt | $(AWK_Component_Attributes) | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/sdk/ReserveCost.idt | $(PROCESS_FILES_COMP) | $(SED_ALL_RESERVE) > $(SDKISHIELDDIR)/ReserveCost.idt
	$(CAT) $(CURRENTDIR)/sdk/FeatureComponents.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/FeatureComponents.idt
	$(CAT) $(CURRENTDIR)/sdk/ISComponentExtended.idt | $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/ISComponentExtended.idt
	$(CAT) $(CURRENTDIR)/sdk/CustomAction.idt | $(SED_FULL_VERSION) |  $(PROCESS_FILES_COMP) > $(SDKISHIELDDIR)/CustomAction.idt
	$(CAT) $(CURRENTDIR)/sdk/ISString.idt | $(SED_FULL_VERSION) > $(SDKISHIELDDIR)/ISString.idt
	$(CAT) $(CURRENTDIR)/sdk/ActionText.idt | $(SED_COPY_TEXT) > $(SDKISHIELDDIR)/ActionText.idt
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) | $(SED_AUTOMATION_LAYER) > $(TEMPDIR)/sdkfile.vbs
ifeq ($(ARCH_DATA_MODEL),64)
	regsvr32.exe /s $(ISHIELDDIR)/System/ISWIAutomation.dll
endif
	$(MAKE) create_removefile
	$(MAKE) create_directory
	$(WSCRIPT) $(TEMPDIR)/sdkfile.vbs

#
# Copy base image to be bundled with installer.
#
setup_image: $(ZIPPER) 
	@$(ECHO) Cleaning old temporary install image
	$(MKDIR) -p $(TMPSDKIMAGE)
	$(CHMOD) -Rf 777 $(TMPSDKIMAGE)
	$(RM) -r $(TMPSDKIMAGE)
	$(MKDIR) -p $(TMPSDKIMAGE)
	$(CP) -fr $(NEW_IMAGE_SDK_DIR)/* $(TMPSDKIMAGE)
	-$(RM) `$(FIND) $(TMPSDKIMAGE) -name \*.map`
	-$(RM) `$(FIND) $(TMPSDKIMAGE) -name \*.pdb`
	$(RM) -r $(TMPPACKED_JDK_JARDIR)
	$(MKDIR) -p $(TMPPACKED_JDK_JARDIR)
	$(MAKE) do_pack
	$(MAKE) do_zip

do_pack:
	$(CD) $(INSTALL_BUILDDIR)/pack ; \
	for i in $(JDK_PACKED_JARS) ; do \
	    $(PACK_CMD) $(TMPSDKIMAGE)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPSDKIMAGE)/$$i ; \
	    $(RM) $(TMPSDKIMAGE)/$$i || exit 1; \
	done

##########################################################
#
# This is a workaround.  Install shield 7.04 makes creates
# a component for each sub-dir of our product, and therefore
# a .cab file for each component.  By zipping w/no compression
# we eliminate sub-dirs, and have less .cab files to host
do_zip: $(ZIPPER) $(TOUCHFILE)
	$(MV) $(TMPSDKIMAGE)/src.zip $(TMPSDKIMAGE)/demo $(TMPSDKIMAGE)/sample $(TMPSDKIMAGE)/COPYRIGHT $(TEMPDIR) 
	$(CP) -f $(LAUNCHER) $(TMPSDKIMAGE)/jre/lib/
ifeq ($(ARCH_DATA_MODEL),64)
  ifneq ($(BUILD_DEPLOY_AMD64) , true)
	$(RM) $(TMPSDKIMAGE)/jre/lib/deploy.jar
  endif
endif
	$(CD) $(TMPSDKIMAGE) ; $(ZIPEXE) -0 -r $(TEMPDIR)/tools.zip *
	$(RM) -r $(TMPSDKIMAGE)/*
ifeq ($(ARCH_DATA_MODEL),32)
	$(CP) -fr  $(TEMPDIR)/demo/applets $(TMPSDKIMAGE)
else
	$(CP) -fr  $(TEMPDIR)/demo/plugin/applets $(TMPSDKIMAGE)
endif
	$(FIND) $(TMPSDKIMAGE) -name "*.html" -exec rm -f {} \;
	$(CD) $(TMPSDKIMAGE); $(ZIPEXE) -0 -r $(TEMPDIR)/applets.zip *
	$(RM) -r $(TMPSDKIMAGE)/*
	$(MV) $(TEMPDIR)/demo $(TEMPDIR)/sample $(TMPSDKIMAGE)
ifeq ($(ARCH_DATA_MODEL),32)
	$(FIND) $(TMPSDKIMAGE)/demo/applets $(TMPSDKIMAGE)/demo/plugin/applets -name "*.html" -prune -o -type f -print -exec rm -f {} \;
else
	$(FIND) $(TMPSDKIMAGE)/demo/plugin/applets $(TMPSDKIMAGE)/demo/plugin/applets -name "*.html" -prune -o -type f -print -exec rm -f {} \;
endif
	$(CD) $(TMPSDKIMAGE) ; $(ZIPEXE) -0 -r $(TEMPDIR)/demos.zip *
	$(RM) -r $(TMPSDKIMAGE)/*
	$(MV) $(TEMPDIR)/tools.zip $(TEMPDIR)/demos.zip $(TEMPDIR)/src.zip $(TEMPDIR)/applets.zip $(TEMPDIR)/COPYRIGHT $(TMPSDKIMAGE)

create_removefile:
	$(RM) $(TEMPDIR)/removefile
	$(TOUCH) $(TEMPDIR)/removefile
	for dir in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) * -type d` ; do \
	  dirkey=`$(ECHO) $$dir | $(TR) -d "/" | $(TR) -d "-"` ; \
	  if [ "$$dirkey" != "jrelibext" ]; then \
	    dirsafe=`$(ECHO) $$dirkey | $(TR) - j | $(TR) + k | $(TR) $$ l ` ; \
	    $(PRINTF) "$$dirsafe\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t\t$$dirkey\t3\n" >> $(TEMPDIR)/removefile || exit 1; \
	    $(PRINTF) "f$$dirsafe\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t*\t$$dirkey\t3\n" >> $(TEMPDIR)/removefile || exit 1; \
	  fi ; \
	done;
	for libextlist in `$(FIND) $(JDK_IMAGE_DIR)/jre/lib/ext` ; do \
	  libextbase=`$(BASENAME) $$libextlist` ; \
	  filekey=`$(ECHO) $$libextbase | $(TR) -d -` ; \
	  LENGTH=`$(ECHO) $$libextbase | $(NAWK) '{print length($$0);}'` ; \
	  if [ "$$LENGTH" -gt "7" ]; then \
	    filename=`$(ECHO) $$libextbase | $(NAWK) '{print substr($$0,0,6);}'``$(ECHO) '*'` ; \
	  else \
	    filename=`$(ECHO) $$libextbase` ; \
	  fi ; \
	  if [ "$$libextbase" != "ext" ]; then \
	    $(PRINTF) "$$filekey\tsz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t$$filename\tjrelibext\t3\n" >> $(TEMPDIR)/removefile || exit 1; \
	  fi ; \
	done;
	$(CAT) $(TEMPDIR)/removefile | $(PROCESS_FILES_COMP) >> $(SDKISHIELDDIR)/RemoveFile.idt

create_directory:
	$(RM) $(TEMPDIR)/directory
	$(TOUCH) $(TEMPDIR)/directory
	for dir in `$(CD) $(JDK_IMAGE_DIR) ; $(FIND) . -name "*" -type d` ; do \
	  dirbase=`$(ECHO) $$dir | $(TR) -d "/" | $(TR) -d "." | $(TR) -d "-"` ; \
	  dirparent=`$(DIRNAME) $$dir | $(TR) -d "/" | $(TR) -d "." | $(TR) -d "-"` ; \
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
	    $(PRINTF) "$$dirbase\t$$dirparent\t$$dirdir\t\t0\n" >> $(TEMPDIR)/directory || exit 1; \
	  fi ; \
	done;
	$(CAT) $(TEMPDIR)/directory | $(PROCESS_FILES_COMP) >> $(SDKISHIELDDIR)/Directory.idt

.PHONY: clean all create_bundle setup_installer_files setup_image do_zip do_pack

