#
# Copyright (c) 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.225 10/06/07
#
# This is the makefile for building the J2RE installer bundles after compiling
# the j2sdk sources. Listed below are some general notes in using this makefile.
#
# 1) Requires InstallShield Developer defined as $(ALT_ISHIELDDIR).
# 2) Requires InstallShield Developer lang packs East and West
# 3) Requires MKS Toolkit (should be first thing in your PATH).
# 5) $(TEMPDIR) is the temporary building directory.
# 6) $(BASE_IMAGE_JRE_DIR) is the base J2RE image.
# 7) $(NEW_IMAGE_JRE_DIR) is the new J2RE image.
# 8) $(INSTALL_BUILDDIR) is two directories above this directory.
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

CURRENTDIR := $(call FullPath,.)
TMPJREIMAGE = $(TEMPDIR)/jre-image
TMPPACKED_JRE_JARDIR = $(TEMPDIR)/packed-jre-jars

ifeq ($(ARCH_DATA_MODEL),64)
  OPT_PLATFORM=.64bit
endif

SED_BINARY_LOCATION = $(SED) -e 's%BINARYPROJECT%$(subst /,\\,$(TEMPDIR)/jre.ism)%g'
SED_TEXT_LOCATION = $(SED) -e 's%TEXTPROJECT%$(subst /,\\,$(TEMPDIR)/jre.isv)%g'
SED_JDK_VERSION = $(SED) -e 's%JDKVERSION%$(JDK_VERSION)%g'

SED_MARKETING_VERSION = $(SED) -e 's%MARKETINGVERSION%$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)%g'

SED_UNDERSCORE_VERSION = $(SED) -e 's%UNDERSCOREVERSION%$(JDK_MKTG_UNDERSCORE_VERSION)%g'

ifdef JDK_UPDATE_VERSION
  MARKETING_NUMBER := $(shell \
	$(ECHO) $(JDK_UPDATE_VERSION) | $(NAWK) '{if (substr($$0,1,1)=="0") print substr($$0, 2); else print $$0;}')
  MARKET_NAME= $(shell $(ECHO) " Update $(MARKETING_NUMBER)")
else
  MARKET_NAME=
endif

MARKET_NAME_CONSUMER= $(shell $(ECHO) " Family")
  
SED_MARKETING_UPDATE = $(SED) -e 's%MARKETNAME%$(MARKET_NAME)%g'
SED_MARKETING_UPDATE_CONSUMER = $(SED) -e 's%MARKETNAME%$(MARKET_NAME_CONSUMER)%g'
SED_MSI_UPDATE = $(SED) -e 's%MSIVERSION%$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER)%g'
REMOVE_JQS_IE      = $(NAWK) 'BEGIN { FS="\t"; OFS="\t" } {if (($$1!="jqs_ie")          && ($$2!="jqs_ie")         ) print $$0}'
REMOVE_SSV_PATCHES = $(NAWK) 'BEGIN { FS="\t"; OFS="\t" } {if (($$1!="jre_bin_ssvDlls") && ($$2!="jre_bin_ssvDlls")) print $$0}'

INSTALLERFILESIFTW=$(JREISHIELDDIR)/jre/iftw/DiskImages/Disk1/
INSTALLERFILESSINGLEMSI=$(JREISHIELDDIR)/jre/singlemsi/DiskImages/Disk1
INSTALLERMSIFILE=$(INSTALLERFILESSINGLEMSI)/*.msi

EXTRA_PACKOPTS=--strip-debug --code-attribute=StackMapTable=strip

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the jre-image
# to the temp directory, setup the ishield project, and build
ifeq ($(ENHANCED),true)
all: setup_installer_files create_bundle generate_iftw #see iftw.gmk
else
all: $(TEMPDIR) do_zip setup_installer_files create_bundle generate_iftw #see iftw.gmk
endif
inst_bund: setup_installer_files create_bundle generate_iftw #see iftw.gmk

include $(INSTALL_BUILDDIR)/common/iftw.gmk

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

JRE_BUNDLE_NAME = $(JRE_NAME)-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)
JRE_IFTW_BUNDLE_NAME = $(JRE_SHORT_BUNDLE_NAME)-iftw

JREISHIELDDIR = $(TEMPDIR)/jre

$(JREISHIELDDIR):
	$(MKDIR) -p $@

ZIPPER = $(TEMPDIR)/zipper$(EXE_SUFFIX)

$(ZIPPER):
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/sun.tools.zipper/$(OBJDIRNAME)/zipper$(EXE_SUFFIX) $@

LAUNCHER = $(ALT_OUTPUTDIR)/tmp/sun.tools.launcher/$(OBJDIRNAME)/launcher$(EXE_SUFFIX)

SUNGRAPHIC3 = $(TEMPDIR)/javaad.ibd

$(SUNGRAPHIC3):
	$(CP) -f $(CURRENTDIR)/../javaad.ibd $@


SUNGRAPHIC4 = $(TEMPDIR)/sunlogobanner.ibd

$(SUNGRAPHIC4):
	$(CP) -f $(CURRENTDIR)/../sunlogobanner.ibd $@


SUNGRAPHIC5 = $(TEMPDIR)/white_background.ibd

$(SUNGRAPHIC5):
	$(CP) -f $(CURRENTDIR)/../white_background.ibd $@

SUNGRAPHIC6 = $(TEMPDIR)/IsDialogBannerRed.ibd

$(SUNGRAPHIC6):
	$(CP) -f $(CURRENTDIR)/../IsDialogBannerRed.ibd $@


TRANSFORMDIR=$(JREISHIELDDIR)/jre/transform/javaone/

javaone_transforms:
	$(RM) -rf $(TMPJREIMAGE).mv
	$(MV) $(TMPJREIMAGE) $(TMPJREIMAGE).mv
	$(RM) -r $(TRANSFORMDIR)
	$(MKDIR) -p $(TRANSFORMDIR)
	$(CAT) $(JREISHIELDDIR)/Property.idt | $(GREP) -v MODE | $(GREP) -v FROMVERSION > $(JREISHIELDDIR)/Property.idt.tmp
	$(MV) -f $(JREISHIELDDIR)/Property.idt.tmp $(JREISHIELDDIR)/Property.idt
	$(RM) $(TEMPDIR)/jre.ism
	$(WSCRIPT) $(TEMPDIR)/jrefile.vbs
ifneq ($(MAKE_INSTALL_LANGS), english_only)
	$(CP) -f $(CURRENTDIR)/../javaad1028.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi  $(TRANSFORMDIR)/1028.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1028.mst $(TRANSFORMDIR)/1028.msi
	$(CP) -f $(CURRENTDIR)/../javaad1031.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1031.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1031.mst $(TRANSFORMDIR)/1031.msi
	$(CP) -f $(CURRENTDIR)/../javaad1034.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1034.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1034.mst $(TRANSFORMDIR)/1034.msi
	$(CP) -f $(CURRENTDIR)/../javaad1036.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1036.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1036.mst $(TRANSFORMDIR)/1036.msi
	$(CP) -f $(CURRENTDIR)/../javaad1040.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1040.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1040.mst $(TRANSFORMDIR)/1040.msi
	$(CP) -f $(CURRENTDIR)/../javaad1041.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1041.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1041.mst $(TRANSFORMDIR)/1041.msi
	$(CP) -f $(CURRENTDIR)/../javaad1042.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1042.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1042.mst $(TRANSFORMDIR)/1042.msi
	$(CP) -f $(CURRENTDIR)/../javaad1046.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1046.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1046.mst $(TRANSFORMDIR)/1046.msi
	$(CP) -f $(CURRENTDIR)/../javaad1053.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1053.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/1053.mst $(TRANSFORMDIR)/1053.msi
	$(CP) -f $(CURRENTDIR)/../javaad2052.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/2052.msi
	$(DEVTOOLS_PATH)/msitran.exe -a $(INSTALLERFILESIFTW)/2052.mst $(TRANSFORMDIR)/2052.msi
endif
	$(CP) -f $(CURRENTDIR)/../javaad.ibd $(TEMPDIR)/javaad.ibd
	-$(BUILDER) -s -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TRANSFORMDIR)/1033.msi
ifneq ($(MAKE_INSTALL_LANGS), english_only)
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1028.msi $(TRANSFORMDIR)/jre1028.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1031.msi $(TRANSFORMDIR)/jre1031.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1034.msi $(TRANSFORMDIR)/jre1034.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1036.msi $(TRANSFORMDIR)/jre1036.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1040.msi $(TRANSFORMDIR)/jre1040.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1041.msi $(TRANSFORMDIR)/jre1041.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1042.msi $(TRANSFORMDIR)/jre1042.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1046.msi $(TRANSFORMDIR)/jre1046.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/1053.msi $(TRANSFORMDIR)/jre1053.MST
	$(DEVTOOLS_PATH)/msitran.exe -g $(TRANSFORMDIR)/1033.msi $(TRANSFORMDIR)/2052.msi $(TRANSFORMDIR)/jre2052.MST
endif
	$(MV) $(TMPJREIMAGE).mv $(TMPJREIMAGE)
ifneq ($(MAKE_INSTALL_LANGS), english_only)
	$(CP) -f $(TRANSFORMDIR)/jre*.MST $(TEMPDIR)/offline-final
	$(CP) -f $(TRANSFORMDIR)/jre*.MST $(TEMPDIR)/online-final
endif

###################### create_bundle ###################### 
# 
# Create the install project and build the release
#

create_bundle:
	$(RM) -f $(TEMPDIR)/offline-final/* $(TEMPDIR)/online-final/*
	$(MKDIR) -p $(TEMPDIR)/offline-final $(TEMPDIR)/online-final
	$(MAKE) create_consumer_bundle
	$(MAKE) create_static_bundle
	$(MAKE) create_update_bundles
	$(MAKE) javaone_transforms
	$(MAKE) create_private_jre
#validate all of the msi files here
	$(MSIVAL2) $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c.msi $(LOGOCUB) -f
ifeq ($(ARCH_DATA_MODEL),32)
  ifneq ($(KERNEL), off)
	$(MSIVAL2) $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c-k.msi $(LOGOCUB) -f
  endif
endif
	$(MSIVAL2) $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c-l.msi $(LOGOCUB) -f
	$(MSIVAL2) $(TEMPDIR)/online-final/jre$(JDK_VERSION)-s.msi $(LOGOCUB) -f
	$(MSIVAL2) $(TEMPDIR)/online-final/jre$(JDK_VERSION)-s-l.msi $(LOGOCUB) -f
	$(MSIVAL2) $(TEMPDIR)/offline-final/jre$(JDK_VERSION)-c.msi $(LOGOCUB) -f
	$(MSIVAL2) $(TEMPDIR)/offline-final/jre$(JDK_VERSION)-s.msi $(LOGOCUB) -f
	$(MSIVAL2) $(TEMPDIR)/privateJREforSDK/jre.msi $(LOGOCUB) -f
#
#sign all of the msi files
ifeq ($(SIGN_FLAG),1)
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c.msi )
  ifeq ($(ARCH_DATA_MODEL),32)
    ifneq ($(KERNEL), off)
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c-k.msi )
    endif
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c-l.msi )
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/online-final/jre$(JDK_VERSION)-s.msi )
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/online-final/jre$(JDK_VERSION)-s-l.msi )
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/offline-final/Data1.cab )
	$(call Try3Times, $(CD) $(TEMPDIR)/offline-final ; $(MSICERT) -d jre$(JDK_VERSION)-c.msi -m 1 -c Data1.cab )
	$(CD) $(TEMPDIR)/offline-final ; $(MSICERT) -d jre$(JDK_VERSION)-s.msi -m 1 -c Data1.cab
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/offline-final/jre$(JDK_VERSION)-c.msi )
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/offline-final/jre$(JDK_VERSION)-s.msi )
	$(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/privateJREforSDK/jre.msi )
	# for each version like "10" of jre1.6.0_10 or "20-b03" of jre1.6.0_20-b03 in $PATCH_LIST, respectively
	for i in $(PATCH_LIST) ; do \
	  UPDATESHORT=`$(ECHO) $$i | $(NAWK) '{print substr($$0,10);}'` ; \
	  if [ "$$UPDATESHORT" == "" ]; then \
	    UPDATESHORT=00 ; \
	  fi ; \
	  $(call Try3Times, $(FULL_SIGNER) -n "Java(TM) SE Runtime Environment $(JDK_MINOR_VERSION)$(MARKET_NAME)" -i http://java.com/ $(TEMPDIR)/online-final/jre$(JDK_VERSION)-pfrom`$(ECHO) $$UPDATESHORT`.msi ) ; \
	done;
  endif
endif
# Do extra compression on all online msi/MST files if it's available, otherwise
# it will have been compressed with zip and cab LZW:21 earlier.
ifdef EXTRA_COMP_CMD_PATH
	$(RM) -r $(TEMPDIR)/online-final.beforecompression
	$(CP) -r $(TEMPDIR)/online-final $(TEMPDIR)/online-final.beforecompression
	for filetocompress in `$(CD) $(TEMPDIR)/online-final; $(LS) -rd *MST *msi` ; do \
	  $(EXTRA_COMP_CMD_PATH) e $(EXTRA_COMP_CMD_MSI_OPTS) "$(TEMPDIR)/online-final/$$filetocompress" "$(TEMPDIR)/online-final/$$filetocompress.compress" ; \
	  $(RM) "$(TEMPDIR)/online-final/$$filetocompress" ; \
	  $(MV) "$(TEMPDIR)/online-final/$$filetocompress.compress" "$(TEMPDIR)/online-final/$$filetocompress" ; \
	done;
endif
#copy files for inclusion as SDK private jre
	$(CP) -f $(TEMPDIR)/privateJREforSDK/jre.msi $(TEMPDIR)/
ifneq ($(MAKE_INSTALL_LANGS), english_only)
	$(CP) -f $(TEMPDIR)/privateJREforSDK/jre1041.MST $(TEMPDIR)/
	$(CP) -f $(TEMPDIR)/privateJREforSDK/jre2052.MST $(TEMPDIR)/
endif

#this target is for development/debugging only
recreate_consumer_bundle: $(JREISHIELDDIR)
	$(MKDIR) -p $(TEMPDIR)/offline-final $(TEMPDIR)/online-final
	$(RM) -f $(JREISHIELDDIR)/*.idt.save
	$(CP) -f $(CURRENTDIR)/jre.isv $(TEMPDIR)
	$(CHMOD) 777 $(JREISHIELDDIR)/*
	$(CD) $(CURRENTDIR)/jre/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(JREISHIELDDIR)/
	$(CHMOD) 777 $(JREISHIELDDIR)/*
	$(CAT) $(CURRENTDIR)/jre/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_MARKETING_VERSION) | $(SED_HARDWARE) > $(JREISHIELDDIR)/_SummaryInformation.idt
	$(CAT) $(CURRENTDIR)/jre/InstallExecuteSequence.idt | $(GREP_DEPLOY_ACTIONS) > $(JREISHIELDDIR)/InstallExecuteSequence.idt
ifeq ($(ARCH_DATA_MODEL),64)
	$(CAT) $(CURRENTDIR)/jre/RegLocator.idt | $(AWK_RegLocator_Type) > $(JREISHIELDDIR)/RegLocator.idt
endif
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) | $(SED_AUTOMATION_LAYER) > $(TEMPDIR)/jrefile.vbs
	$(MAKE) create_consumer_bundle

# If the $(BIN_BUNDLEDIR)/patch-image.zip file is missing, instead of a NULL ImageCkSum being written to the registry,
# the build will error with:
# C:\devtools\bin\gnumake.exe: *** No rule to make target `D:/OUTPUTDIRs/6u21/bundles/patch-image.zip', needed by `create_consumer_bundle'.  Stop.
create_consumer_bundle: $(BIN_BUNDLEDIR)/patch-image.zip
# modify .idt files and import for consumer msi
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) | $(SED_AUTOMATION_LAYER) > $(TEMPDIR)/jrefile.vbs
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(ChangePropertyTable) | $(SED_CONSUMER) > $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "FROMVERSION\tNA\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "IS32BITMSI\t$(IS32BITARCH)\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "OTHER\t1\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/jre/Upgrade.idt | $(SED_MSI_UPDATE) | $(SED_UPGRADE_CODE_VERSION_CONSUMER) | $(SED_UPGRADE_CODE_VERSIONS) | $(SED_UPGRADE_ATTRIBS_NON_STATIC) > $(JREISHIELDDIR)/Upgrade.idt
	$(CAT) $(CURRENTDIR)/jre/Component.idt | $(AWK_Component_Attributes) | $(SED_COMPONENT_ID_VERSION_CONSUMER) | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/jre/Directory.idt | $(AWK_Directory_ProgramFilesFolder) | $(SED_FULL_VERSION) | $(SED_INSTALLDIR_CONSUMER) > $(JREISHIELDDIR)/Directory.idt
	$(CAT) $(TEMPDIR)/directory >> $(JREISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/jre/RemoveFile.idt | $(SED_FOLDER) > $(JREISHIELDDIR)/RemoveFile.idt
	$(CAT) $(CURRENTDIR)/jre/RemoveFile.idt.nonupdate | $(SED_FOLDER) >> $(JREISHIELDDIR)/RemoveFile.idt
	$(CAT) $(TEMPDIR)/removefile >> $(JREISHIELDDIR)/RemoveFile.idt
	$(CAT) $(CURRENTDIR)/jre/FeatureComponents.idt   | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/FeatureComponents.idt
	$(CAT) $(CURRENTDIR)/jre/File.idt				 | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/File.idt
	$(CAT) $(CURRENTDIR)/jre/ISComponentExtended.idt | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/IsComponentExtended.idt
	# build "singlemsi" offline installer
	$(RM) -r $(TMPJREIMAGE)/*
	$(TOUCH) $(TMPJREIMAGE)/patchjre.exe
	$(CP) -f $(TEMPDIR)/core-offline.zip $(TMPJREIMAGE)/core.zip
	$(WSCRIPT) $(TEMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "singlemsi" -c COMP
ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERMSIFILE)
endif
	$(CP) -f $(INSTALLERMSIFILE) $(TEMPDIR)/offline-final/jre$(JDK_VERSION)-c.msi
	$(CP) -f $(INSTALLERFILESSINGLEMSI)/Data1.cab $(TEMPDIR)/offline-final/
	# iftw - with charsets.jar and localedata.jar (OTHER=1)
	$(CP) -f $(TEMPDIR)/core-online-l.zip $(TMPJREIMAGE)/core.zip
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERFILESIFTW)/*.msi
endif
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c-l.msi
	# iftw - without charsets.jar and localedata.jar (OTHER<>1)
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(ChangePropertyTable) | $(SED_CONSUMER) > $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "IS32BITMSI\t$(IS32BITARCH)\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(WSCRIPT) $(TEMPDIR)/jrefile.vbs
	$(CP) -f $(TEMPDIR)/core-online.zip $(TMPJREIMAGE)/core.zip
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERFILESIFTW)/*.msi
endif
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c.msi
	# iftw (for kernel)
ifeq ($(ARCH_DATA_MODEL),32)
  ifneq ($(KERNEL), off)
	$(CP) -f $(TEMPDIR)/core-kernel.zip $(TMPJREIMAGE)/core.zip
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(ChangePropertyTable) | $(SED_CONSUMER) > $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "KERNEL\t1\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "FROMVERSION\tNA\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "IS32BITMSI\t$(IS32BITARCH)\t\n" >> $(JREISHIELDDIR)/Property.idt
	# If extra compression available, unpack/repack Data1.cab so it is 
	# truly uncompressed and then put it into the .msi as a stream.
	# It will get extra compressed after signing. With extra compression
	# at install time the installer (i.e. what starts life as 
	# jinstall_jkernel) will have extracted the extra compression library
	# into the same folder as the .msi is in. A line is added to 
	# MoveFile.idt to get the library into its final place under 
	# lib/deploy for use by sun.jkernel.Bundle. It isn't under bin to
	# avoid any implication for users that it's intended for general use.
	# The MoveFile.idt is restored after the msi build to avoid confusion
    ifdef EXTRA_COMP_CMD_PATH
	$(RM) -f $(INSTALLERFILESSINGLEMSI)/*.msi
#	$(CP) -f $(CURRENTDIR)/jre/MoveFile.idt $(JREISHIELDDIR)/MoveFile.idt
#	$(ECHO) \
#	"CopyLZMA_dll    jz    $(EXTRA_COMP_LIB_NAME)        SOURCEDIR    deploy    0" \
#	  >>$(JREISHIELDDIR)/MoveFile.idt
	$(WSCRIPT) $(TEMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "singlemsi" -c NOCOMP
      ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERFILESSINGLEMSI)
      endif
#	$(CP) -f $(CURRENTDIR)/jre/MoveFile.idt $(JREISHIELDDIR)/MoveFile.idt
	$(CD) $(INSTALLERFILESSINGLEMSI); $(RM) -rf tmp ; \
	$(RM) filetable.txt ; \
	$(WSCRIPT) $(CURRENTDIR)/ExtractFileList.vbs *.msi ; \
	$(MKDIR) tmp ; $(RM) -f OldData1.cab ; $(CD) tmp ;  \
	$(DEVTOOLS_PATH)cabarc x ../Data1.cab ; $(MV) ../Data1.cab \
	    ../OldData1.cab ; \
	$(DEVTOOLS_PATH)cabarc -m NONE n ../Data1.cab `cat ../filetable.txt` ; \
	$(CD) .. ; \
	$(CSCRIPT) $(CURRENTDIR)/WiStreamData1.vbs *.msi Data1.cab ; \
	$(CP) -f $(INSTALLERFILESSINGLEMSI)/*.msi \
	    $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c-k.msi
    else
	# Not available: cab LZW:21 will be used
	$(WSCRIPT) $(TEMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
      ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERFILESIFTW)/*.msi
      endif
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi \
	  $(TEMPDIR)/online-final/jre$(JDK_VERSION)-c-k.msi
    endif
  endif
endif

create_static_bundle: $(BIN_BUNDLEDIR)/patch-image.zip
# modify .idt files and import for static msi
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(ChangePropertyTable) | $(SED_STATIC) > $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "FROMVERSION\tNA\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "IS32BITMSI\t$(IS32BITARCH)\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "OTHER\t1\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/jre/Upgrade.idt | $(SED_MSI_UPDATE) | $(SED_UPGRADE_CODE_VERSION_STATIC) | $(SED_UPGRADE_CODE_VERSIONS) | $(SED_UPGRADE_ATTRIBS_STATIC) > $(JREISHIELDDIR)/Upgrade.idt
	$(CAT) $(CURRENTDIR)/jre/Component.idt | $(AWK_Component_Attributes) | $(SED_COMPONENT_ID_VERSION_STATIC) | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/jre/Directory.idt | $(AWK_Directory_ProgramFilesFolder) | $(SED_FULL_VERSION) | $(SED_INSTALLDIR_STATIC) > $(JREISHIELDDIR)/Directory.idt
	$(CAT) $(TEMPDIR)/directory >> $(JREISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/jre/RemoveFile.idt | $(SED_FOLDER) > $(JREISHIELDDIR)/RemoveFile.idt
	$(CAT) $(CURRENTDIR)/jre/RemoveFile.idt.nonupdate | $(SED_FOLDER) >> $(JREISHIELDDIR)/RemoveFile.idt
	$(CAT) $(TEMPDIR)/removefile >> $(JREISHIELDDIR)/RemoveFile.idt
	$(CAT) $(CURRENTDIR)/jre/FeatureComponents.idt   | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/FeatureComponents.idt
	$(CAT) $(CURRENTDIR)/jre/File.idt				 | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/File.idt
	$(CAT) $(CURRENTDIR)/jre/ISComponentExtended.idt | $(REMOVE_JQS_IE) | $(REMOVE_SSV_PATCHES) > $(JREISHIELDDIR)/IsComponentExtended.idt
	# build "singlemsi" offline installer
	$(RM) -r $(TMPJREIMAGE)/*
	$(TOUCH) $(TMPJREIMAGE)/patchjre.exe
	$(CP) -f $(TEMPDIR)/core-offline.zip $(TMPJREIMAGE)/core.zip
	$(WSCRIPT) $(TEMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "singlemsi" -c COMP
ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERMSIFILE)
endif
	$(CP) -f $(INSTALLERMSIFILE) $(TEMPDIR)/offline-final/jre$(JDK_VERSION)-s.msi
	# iftw - with charsets.jar and localedata.jar (OTHER=1)
	$(CP) -f $(TEMPDIR)/core-online-l.zip $(TMPJREIMAGE)/core.zip
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERFILESIFTW)/*.msi
endif
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TEMPDIR)/online-final/jre$(JDK_VERSION)-s-l.msi
	# iftw - without charsets.jar and localedata.jar (OTHER<>1)
	$(CP) -f $(TEMPDIR)/core-online.zip $(TMPJREIMAGE)/core.zip
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(ChangePropertyTable) | $(SED_STATIC) > $(JREISHIELDDIR)/Property.idt
	$(PRINTF) "IS32BITMSI\t$(IS32BITARCH)\t\n" >> $(JREISHIELDDIR)/Property.idt
	$(WSCRIPT) $(TEMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP
ifeq ($(ARCH_DATA_MODEL), 64)
	$(CSCRIPT) $(CURRENTDIR)/Wi64bitValidation.vbs $(INSTALLERFILESIFTW)/*.msi
endif
	$(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TEMPDIR)/online-final/jre$(JDK_VERSION)-s.msi

create_update_bundles: $(BIN_BUNDLEDIR)/patch-image.zip
ifdef PATCH_LIST
# modify .idt files and import for update msi's
# if create_update_bundles is to be supported on Windows x64, will need to call $(REMOVE_JQS_IE) instead of CP of jqs_plugin.dl
	$(CAT) $(CURRENTDIR)/jre/Component.idt | $(AWK_Component_Attributes) | $(SED_COMPONENT_ID_VERSION_UPDATE) > $(JREISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(ChangePropertyTable) | $(SED_UPDATE) > $(JREISHIELDDIR)/Property.idt.update
	$(CAT) $(CURRENTDIR)/jre/Upgrade.idt | $(SED_MSI_UPDATE) | $(SED_UPGRADE_CODE_VERSION_UPDATE) | $(SED_UPGRADE_CODE_VERSIONS) | $(SED_UPGRADE_ATTRIBS_NON_STATIC) > $(JREISHIELDDIR)/Upgrade.idt
	$(CP) $(CURRENTDIR)/jre/FeatureComponents.idt   $(JREISHIELDDIR)/FeatureComponents.idt
	$(CP) $(CURRENTDIR)/jre/File.idt                $(JREISHIELDDIR)/File.idt
	$(CP) $(CURRENTDIR)/jre/ISComponentExtended.idt $(JREISHIELDDIR)/IsComponentExtended.idt
	$(CAT) $(CURRENTDIR)/jre/RemoveFile.idt | $(SED_FOLDER) > $(JREISHIELDDIR)/RemoveFile.idt
	$(RM) -r $(TMPJREIMAGE)/*
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/lib/deploy/jqs/ie/jqs_plugin.dll $(TMPJREIMAGE)
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/bin/jp2ssv.dll $(TMPJREIMAGE)
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/bin/ssv.dll $(TMPJREIMAGE)
	$(TOUCH) $(TMPJREIMAGE)/core.zip
	# $fromversion will look like "1.6.0_10" or "1.6.0_20" if jre1.6.0_10 and jre1.6.0_20-b03 are in $PATCH_LIST, respectively
	# $patcherversion will like "1.6.0_10" or "1.6.0_20-b03" if jre1.6.0_10 and jre1.6.0_20-b03 are in $PATCH_LIST, respectively
	# $fromversion is used to access the registry and must not contain the build number
	# $patcherversion contains the build number because patches from "1.6.0_20-b01" and "1.6.0_20-b02" are different
	for i in $(PATCH_LIST) ; do \
	  fromversion=`$(ECHO) $$i | $(NAWK) '{print substr($$0,4, 8);}'` ; \
	  patcherversion=`$(ECHO) $$i | $(NAWK) '{print substr($$0,4);}'` ; \
	  $(CP) -f $(JREISHIELDDIR)/Property.idt.update $(JREISHIELDDIR)/Property.idt ; \
	  $(PRINTF) "IS32BITMSI\t$(IS32BITARCH)\t\n" >> $(JREISHIELDDIR)/Property.idt  ; \
	  $(PRINTF) "FROMVERSION\t$$fromversion\t\n" >> $(JREISHIELDDIR)/Property.idt || exit 1; \
	  $(WSCRIPT) $(TEMPDIR)/jrefile.vbs ; \
	  $(CP) -f $(ALT_OUTPUTDIR)/tmp/patcher/jre/$(OBJDIRNAME)/patcher-jre`$(ECHO) $$patcherversion`.exe $(TMPJREIMAGE)/patchjre.exe ; \
	  $(BUILDER) -p $(TEMPDIR)/jre.ism -r "iftw" -c COMP ; \
	  UPDATESHORT=`$(ECHO) $$i | $(NAWK) '{print substr($$0,10);}'` ; \
	  if [ "$$UPDATESHORT" == "" ]; then \
	    UPDATESHORT=00 ; \
	  fi ; \
	  $(CP) -f $(INSTALLERFILESIFTW)/*.msi $(TEMPDIR)/online-final/jre$(JDK_VERSION)-pfrom`$(ECHO) $$UPDATESHORT`.msi ; \
	done;
endif

create_private_jre:
#create files for SDK private jre with embedded cab file
	$(RM) -f $(TEMPDIR)/privateJREforSDK/*
	$(MKDIR) -p $(TEMPDIR)/privateJREforSDK
	$(CP) -f $(TEMPDIR)/offline-final/jre$(JDK_VERSION)-c.msi $(TEMPDIR)/privateJREforSDK/jre.msi
	$(CP) -f $(TEMPDIR)/offline-final/Data1.cab $(TEMPDIR)/privateJREforSDK/
ifneq ($(MAKE_INSTALL_LANGS), english_only)
	$(CP) -f $(TEMPDIR)/offline-final/jre1041.MST $(TEMPDIR)/privateJREforSDK/
	$(CP) -f $(TEMPDIR)/offline-final/jre2052.MST $(TEMPDIR)/privateJREforSDK/
endif
	$(CD) $(TEMPDIR)/privateJREforSDK ; $(CSCRIPT) $(CURRENTDIR)/WiStreamData1.vbs jre.msi Data1.cab
	$(RM) -f $(TEMPDIR)/privateJREforSDK/Data1.cab

setup_installer_files: $(SUNGRAPHIC3) $(SUNGRAPHIC4) $(SUNGRAPHIC5) $(SUNGRAPHIC6) $(JREISHIELDDIR) $(ZIPPER)
	$(RM) -f $(JREISHIELDDIR)/*.idt.save
	$(CP) -f $(CURRENTDIR)/jre.isv $(TEMPDIR)
	$(CD) $(CURRENTDIR)/jre/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(JREISHIELDDIR)/
	$(CHMOD) 777 $(JREISHIELDDIR)/*
	$(CAT) $(CURRENTDIR)/jre/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_MARKETING_VERSION) | $(SED_HARDWARE) > $(JREISHIELDDIR)/_SummaryInformation.idt
	$(CAT) $(CURRENTDIR)/jre/InstallExecuteSequence.idt | $(GREP_DEPLOY_ACTIONS) > $(JREISHIELDDIR)/InstallExecuteSequence.idt
ifeq ($(ARCH_DATA_MODEL),64)
	$(CAT) $(CURRENTDIR)/jre/RegLocator.idt | $(AWK_RegLocator_Type) > $(JREISHIELDDIR)/RegLocator.idt
endif
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/bin/regutils.dll $(TEMPDIR)
ifeq ($(ARCH_DATA_MODEL),32)
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/bin/msvcr71.dll $(TEMPDIR)
endif
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/deploy/plugin/RegUtilsMSI/$(OBJDIRNAME)/RegUtilsMSI.dll $(TEMPDIR)
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/deploy/plugin/FilesInUse/$(OBJDIRNAME)/FilesInUse.exe $(TEMPDIR)
	$(MAKE) create_removefile
	$(MAKE) create_directory
ifeq ($(ARCH_DATA_MODEL),64)
	regsvr32.exe /s $(ISHIELDDIR)/System/ISWIAutomation.dll
endif

PRUNE_LIST_OTHER = lib/charsets.pack \
                   lib/ext/localedata.pack

PRUNE_LIST_EXTRA = bin/eula.dll \
	           lib/audio/soundbank.gm \
	           lib/fonts/LucidaBrightDemiBold.ttf \
	           lib/fonts/LucidaBrightDemiItalic.ttf \
	           lib/fonts/LucidaBrightItalic.ttf \
	           lib/fonts/LucidaBrightRegular.ttf \
	           lib/fonts/LucidaSansDemiBold.ttf \
	           lib/fonts/LucidaTypewriterBold.ttf \
	           lib/fonts/LucidaTypewriterRegular.ttf \
	           lib/cmm/PYCC.pf

#
# This target will create the core-offline.zip and core-online.zip files
#
do_zip:
	@$(ECHO) Cleaning old temporary install image
	$(MKDIR) -p $(TMPJREIMAGE)
	$(CHMOD) -Rf 777 $(TMPJREIMAGE)
	$(RM) -r $(TMPJREIMAGE)
	$(MKDIR) -p $(TMPJREIMAGE)
	$(CP) -fr $(NEW_IMAGE_JRE_DIR)/* $(TMPJREIMAGE)
	-$(RM) `$(FIND) $(TMPJREIMAGE) -name \*.map`
	-$(RM) `$(FIND) $(TMPJREIMAGE) -name \*.pdb`
ifeq ($(ARCH_DATA_MODEL),32)
  ifneq ($(KERNEL), off)
	$(CD) ../kernel ; $(MAKE) all
  endif	
endif
ifeq ($(ARCH_DATA_MODEL),64)
  ifneq ($(BUILD_DEPLOY_AMD64) , true)
	$(RM) $(TMPJREIMAGE)/lib/deploy.jar
  endif
endif
	$(RM) $(BIN_BUNDLEDIR)/patch-image.zip
ifneq ($(MAKE_INSTALL_PATCH_IMAGE), off)
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -r $(BIN_BUNDLEDIR)/patch-image.zip *
endif
	$(RM) -r $(TMPPACKED_JRE_JARDIR)
	$(MKDIR) -p $(TMPPACKED_JRE_JARDIR)
	$(MAKE) do_pack
	$(RM) -f $(TEMPDIR)/core-offline.zip $(TEMPDIR)/core-online.zip $(TEMPDIR)/core-online-l.zip
	$(RM) $(TMPJREIMAGE)/bin/RegUtils.dll
	# msvcr71.dll copied from bin to bin/new_plugin in MSIDuplicateUnzippedFiles() in RegInstall.cpp
	$(RM) $(TMPJREIMAGE)/bin/new_plugin/msvcr71.dll
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -r -0 $(TEMPDIR)/core-offline.zip *
	$(CD) $(TMPJREIMAGE) ; $(RM) $(PRUNE_LIST_EXTRA)
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TEMPDIR)/core-online-l.zip *
	$(CD) $(TMPJREIMAGE) ; $(RM) $(PRUNE_LIST_OTHER)
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TEMPDIR)/core-online.zip *
#now create the kernel.zip
ifeq ($(ARCH_DATA_MODEL),32)
  ifneq ($(KERNEL), off)
	$(RM) -f $(TEMPDIR)/core-kernel.zip
	$(RM) -r $(TMPJREIMAGE)
	$(MKDIR) -p $(TMPJREIMAGE)
	$(CP) -fr $(NEW_IMAGE_JRE_DIR)-kernel/* $(TMPJREIMAGE)
	$(RM) $(TMPJREIMAGE)/bin/regutils.dll
	# msvcr71.dll copied from bin to bin/new_plugin in MSIDuplicateInstalledFiles() in RegInstall.cpp
	$(RM) $(TMPJREIMAGE)/bin/new_plugin/msvcr71.dll
	$(CD) $(INSTALL_BUILDDIR)/pack ; ${PACK_CMD} ${EXTRA_PACKOPTS} $(TMPJREIMAGE)/lib/rt.pack $(TMPJREIMAGE)/lib/rt.jar
	$(RM) $(TMPJREIMAGE)/lib/rt.jar
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TEMPDIR)/core-kernel.zip *
  endif
endif

#
# Pack jars in base image.
#
 
do_pack:
	for i in $(JRE_PACKED_JARS) ; do \
	    if [ -f $(JRE_IMAGE_DIR)/$$i ]; then \
	        $(RM) $(TMPJREIMAGE)/$$i || exit 1; \
	        $(CP) -f $(ALT_OUTPUTDIR)/pack/pack-jre-jars/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPJREIMAGE)/`$(DIRNAME) $$i` || exit 1; \
	    fi ; \
	done
	
create_removefile:
	$(RM) $(TEMPDIR)/removefile
	$(TOUCH) $(TEMPDIR)/removefile
	for dir in `($(CD) $(JRE_IMAGE_DIR) ; $(FIND) . -name "*" -type d ; $(CD) $(BASE_IMAGE_JRE_DIR) ; $(FIND) . -name "*" -type d ; ) | $(SORT) -u` ; do \
	  dirbase=`$(BASENAME) $$dir` ; \
	  if [ "$$dirbase" != "." ] && [ "$$dirbase" != "ext" ]; then \
	    $(PRINTF) "$$dirbase\tjz\t\t$$dirbase\t3\n" >> $(TEMPDIR)/removefile || exit 1; \
	    if [ "$$dirbase" != "bin" ]; then \
	      $(PRINTF) "file$$dirbase\tjz\t*\t$$dirbase\t3\n" >> $(TEMPDIR)/removefile || exit 1; \
	    fi ; \
	  fi ; \
	done;
	for libextlist in `($(CD) $(JRE_IMAGE_DIR)/lib/ext ; $(FIND) . -name "*"; $(CD) $(BASE_IMAGE_JRE_DIR)/lib/ext ; $(FIND) . -name "*"; ) | $(SORT) -u` ; do \
	  libextbase=`$(BASENAME) $$libextlist` ; \
	  filekey=`$(ECHO) $$libextbase | $(TR) -d -` ; \
	  LENGTH=`$(ECHO) $$libextbase | $(NAWK) '{print length($$0);}'` ; \
	  if [ "$$LENGTH" -gt "7" ]; then \
	    filename=`$(ECHO) $$libextbase | $(NAWK) '{print substr($$0,0,6);}'``$(ECHO) '*'` ; \
	  else \
	    filename=`$(ECHO) $$libextbase` ; \
	  fi ; \
	  if [ "$$libextbase" != "ext" ] && [ "$$libextbase" != "." ] ; then \
	    $(PRINTF) "$$filekey\tjz\t$$filename\text\t3\n" >> $(TEMPDIR)/removefile || exit 1; \
	  fi ; \
	done;
	# special handling to exclude bin\msvcr71.dll from the RemoveFile table and InstallValidate processing (CR 6597916, CR 6624173) 
	for binlist in `($(CD) $(JRE_IMAGE_DIR)/bin ; $(FIND) . ! -name msvcr71.dll  ! -name "*.map"  ! -name "*.pdb" -type f | $(SORT) )` ; do \
	  NUM_FIELDS=`$(ECHO) $$binlist | $(NAWK) 'BEGIN { FS="\/" } { print NF;}'` ; \
	  if [ "$$NUM_FIELDS" -eq "2" ]; then \
		binbase=`$(BASENAME) $$binlist` ; \
		filekey=`$(ECHO) $$binbase | $(TR) -d -` ; \
		filename=`$(ECHO) $$binbase | $(NAWK) 'BEGIN { OFS="" } {if (length($$0) > 7) printf("%s*", substr($$0,0,6)); else print $$0}'` ; \
		$(PRINTF) "$$filekey\tjz\t$$filename\tbin\t3\n" >> $(TEMPDIR)/removefile || exit 1; \
	  fi ; \
	done;


create_directory:
	$(RM) $(TEMPDIR)/directory
	$(TOUCH) $(TEMPDIR)/directory
	for dir in `($(CD) $(JRE_IMAGE_DIR) ; $(FIND) . -name "*" -type d ; $(CD) $(BASE_IMAGE_JRE_DIR) ; $(FIND) . -name "*" -type d ; ) | $(SORT) -u` ; do \
	  dirbase=`$(BASENAME) $$dir` ; \
	  dirfull=`$(DIRNAME) $$dir` ; \
	  dirparent=`$(BASENAME) $$dirfull` ; \
	  LENGTH=`$(ECHO) $$dirbase | $(NAWK) '{print length($$0);}'` ; \
	  if [ "$$LENGTH" -gt "7" ]; then \
	    dirdir=`$(ECHO) $$dirbase | $(NAWK) '{print substr($$0,0,6);}'``$(ECHO) "~1|"``$(ECHO) $$dirbase` ; \
	  else \
	    dirdir=`$(ECHO) $$dirbase` ; \
	  fi ; \
	  if [ "$$dirparent" = "." ]; then \
	    dirparent=INSTALLDIR ; \
	  fi ; \
	  if [ "$$dirbase" != "." ]; then \
	    $(PRINTF) "$$dirbase\t$$dirparent\t$$dirdir\t\t0\n" >> $(TEMPDIR)/directory || exit 1; \
	  fi ; \
	done;


.PHONY: clean all create_bundle setup_installer_files setup_image do_pack
