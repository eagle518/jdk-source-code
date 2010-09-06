#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#  @(#)Makefile	1.74 04/03/02
#
# This is the makefile for building the J2RE installer bundles after compiling
# the j2sdk sources. Listed below are some general notes in using this makefile.
#
# 1) Requires InstallShield Developer defined as $(ALT_ISHIELDDIR).
# 2) Requires InstallShield Developer lang packs East and West
# 3) Requires MKS Toolkit (should be first thing in your PATH).
# 5) $(TMPDIR) is the temporary building directory.
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

#set path names
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
  TMPDIR = $(TMPISHIELD_DIR)/patch
else
  TMPDIR = $(TMPISHIELD_DIR)/regular
endif

#This is the disk space we reserve for the installed image in bytes
#It will be used in the ReserveCost table
ifdef JDK_UPDATE_VERSION
  CORESPACEVALUE=101000000
  OTHERSPACEVALUE=23000000
  EXTRASPACEVALUE=5000000
else
  CORESPACEVALUE=55000000
  OTHERSPACEVALUE=17000000
  EXTRASPACEVALUE=4000000
endif

#This is what will display in text for the disk space in MB that each
#feature requires when selected.  It includes both update repository 
#and installed image.  It should eventually be auto-computed during 
#the build by checking the size of the .zip files for each feature
ifdef JDK_UPDATE_VERSION
  CORESPACETEXT=131
  OTHERSPACETEXT=23
  EXTRASPACETEXT=7
else
  CORESPACETEXT=72
  OTHERSPACETEXT=20
  EXTRASPACETEXT=6
endif

CURRENTDIR = $(shell $(PWD))
TMPJREIMAGE = $(TMPDIR)/jre-image
TMPPACKED_JRE_JARDIR = $(TMPDIR)/packed-jre-jars

SED_CORE_RESERVE=$(SED) -e 's%CORESPACE%$(CORESPACEVALUE)%g'
SED_OTHER_RESERVE=$(SED) -e 's%OTHERSPACE%$(OTHERSPACEVALUE)%g'
SED_EXTRA_RESERVE=$(SED) -e 's%EXTRASPACE%$(EXTRASPACEVALUE)%g'
SED_ALL_RESERVE=$(SED_CORE_RESERVE) | $(SED_OTHER_RESERVE) | $(SED_EXTRA_RESERVE)

SED_CORE_TEXT=$(SED) -e 's%CORESPACETEXT%$(CORESPACETEXT)%g'
SED_OTHER_TEXT=$(SED) -e 's%OTHERSPACETEXT%$(OTHERSPACETEXT)%g'
SED_EXTRA_TEXT=$(SED) -e 's%EXTRASPACETEXT%$(EXTRASPACETEXT)%g'
SED_ALL_SPACE_TEXT=$(SED_CORE_TEXT) | $(SED_OTHER_TEXT) | $(SED_EXTRA_TEXT)

SED_BINARY_LOCATION = $(SED) -e 's%BINARYPROJECT%$(subst /,\\,$(TMPDIR)/jre.ism)%g'
SED_TEXT_LOCATION = $(SED) -e 's%TEXTPROJECT%$(subst /,\\,$(TMPDIR)/jre.isv)%g'
SED_CAB_LOCATION = $(SED) -e 's%CABLOCATION%$(JRE_CAB_URL)%g'
SED_JDK_VERSION = $(SED) -e 's%JDKVERSION%$(JDK_VERSION)%g'
SED_DOWNLOAD_TEXT = $(SED) -e 's%BUNDLETYPE%DL%g'
SED_COPY_TEXT = $(SED) -e 's%BUNDLETYPE%CP%g'

INSTALLERFILEBUNDLEWI=$(JREISHIELDDIR)/jre/bundlewi/DiskImages/Disk1/setup.exe
INSTALLERFILESIFTW=$(JREISHIELDDIR)/jre/iftw/DiskImages/Disk1/
INSTALLERFILESPRIVATEJRE=$(JREISHIELDDIR)/jre/privatejre/DiskImages/Disk1/
INSTALLERMSIFILE=$(JREISHIELDDIR)/jre/singlemsi/DiskImages/Disk1/*.msi

###################### all ################################
# 
# THIS IS THE ONLY VALID TARGET (with clean). Copy the jre-image
# to the temp directory, setup the ishield project, and build

all: $(TMPDIR) copy_licenses setup_image setup_installer_files create_bundle 

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
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_de.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_de.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_es.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_es.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_fr.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_fr.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_it.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_it.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_ja.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_ja.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_ko.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_ko.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_sv.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_sv.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_zh_CN.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_zh_CN.rtf
	$(CP) $(J2SE_TOPDIR)/src/share/doc/jre/LICENSE_zh_TW.rtf.$(LICENSEEND) $(TMPDIR)/LICENSE_zh_TW.rtf


ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
    JRE_BUNDLE_NAME = jre-$(BIN_BUNDLE_NAMEPART)-p-$(BUNDLE_DATE)
    JRE_IFTW_BUNDLE_NAME = $(JRE_SHORT_BUNDLE_NAME)-p-iftw
else
    JRE_BUNDLE_NAME = jre-$(BIN_BUNDLE_NAMEPART)-$(BUNDLE_DATE)
    JRE_IFTW_BUNDLE_NAME = $(JRE_SHORT_BUNDLE_NAME)-iftw
endif


JREISHIELDDIR = $(TMPDIR)/jre

$(JREISHIELDDIR):
	$(MKDIR) -p $@

ZIPPER = $(TMPDIR)/zipper$(EXE_SUFFIX)

$(ZIPPER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.zipper/$(OBJDIRNAME)/zipper$(EXE_SUFFIX) $@

LAUNCHER = $(TMPDIR)/launcher$(EXE_SUFFIX)

$(LAUNCHER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.launcher/$(OBJDIRNAME)/launcher$(EXE_SUFFIX) $@

COPIER = $(TMPDIR)/copier$(EXE_SUFFIX)

$(COPIER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.copier/$(OBJDIRNAME)/copier$(EXE_SUFFIX) $@

TOUCHFILE = $(TMPDIR)/touchfile$(EXE_SUFFIX)

$(TOUCHFILE):
	$(CP) $(ALT_OUTPUTDIR)/tmp/sun.tools.touchfile/$(OBJDIRNAME)/touchfile$(EXE_SUFFIX) $@

PATCHER = $(TMPDIR)/patchjre$(EXE_SUFFIX)

$(PATCHER):
# 
# Use stub patcher to reduce patcher size for download if no patch should be applied.
#
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	$(CP) $(ALT_OUTPUTDIR)/tmp/patcher/jre/$(OBJDIRNAME)/jre-patcher$(EXE_SUFFIX) $@
else
	$(CP) $(ALT_OUTPUTDIR)/tmp/patcher/stub/$(OBJDIRNAME)/stub-patcher$(EXE_SUFFIX) $@
endif

FONTCHECKER = $(TMPDIR)/FontChecker.jar

$(FONTCHECKER):
	$(CP) $(ALT_OUTPUTDIR)/tmp/fontchecker/com.sun.java2d.fontchecker/FontChecker.jar $@

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
create_bundle: $(TMPDIR)/setup.bmp 
ifeq ($(ARCH), amd64)
	-$(BUILDER) -p $(TMPDIR)/jre.ism -r "bundlewi" -c COMP
else
	$(BUILDER) -p $(TMPDIR)/jre.ism -r "bundlewi" -c COMP
endif
ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "Java Plug-in 1.5.0" -i http://java.com/ $(INSTALLERFILEBUNDLEWI)
endif
	$(CP) $(INSTALLERFILEBUNDLEWI) $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).exe
ifeq ($(ARCH_DATA_MODEL),32)
	$(CP) $(CURRENTDIR)/jre/publicjreISSetupFile.idt $(JREISHIELDDIR)/ISSetupFile.idt
	$(CAT) $(CURRENTDIR)/jre/Binary.idt | $(GREP) -v ISSetupFilesHelper > $(JREISHIELDDIR)/Binary.idt
	$(CAT) $(CURRENTDIR)/jre/InstallUISequence.idt | $(GREP) -v ISSetupFilesExtract | $(GREP) -v ISSetupFilesCleanup > $(JREISHIELDDIR)/InstallUISequence.idt
	$(CAT) $(CURRENTDIR)/jre/InstallExecuteSequence.idt | $(GREP) -v ISSetupFilesExtract | $(GREP) -v ISSetupFilesCleanup > $(JREISHIELDDIR)/InstallExecuteSequence.idt
	$(CAT) $(CURRENTDIR)/jre/CustomAction.idt | $(GREP) -v ISSetupFilesExtract | $(GREP) -v ISSetupFilesCleanup | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/CustomAction.idt
	$(RM) $(TMPDIR)/jre.ism
	$(WSCRIPT) $(TMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TMPDIR)/jre.ism -r "singlemsi" -c COMP
	$(CAT) $(CURRENTDIR)/jre/ActionText.idt | $(SED_DOWNLOAD_TEXT) > $(JREISHIELDDIR)/ActionText.idt
	$(RM) $(TMPDIR)/jre.ism
	$(WSCRIPT) $(TMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TMPDIR)/jre.ism -r "iftw" -c COMP
	for file in `ls -p $(INSTALLERFILESIFTW)/*.mst ` ; do \
		mstbase=`$(BASENAME) $$file` ; \
		MSTbase=`$(ECHO) $$mstbase | $(NAWK) '{ print toupper($1) }'` ; \
		$(CD) $(INSTALLERFILESIFTW) ; $(MV) $$mstbase `$(ECHO) "jre"`$$MSTbase || exit 1 ; $(RM) jre1033.MST ; \
	done 	
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/deploy/plugin/jinstall/obj/jinstall.exe $(INSTALLERFILESIFTW)/$(JRE_IFTW_BUNDLE_NAME).exe
  ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "Java Plug-in 1.5.0" -i http://java.com/ $(INSTALLERFILESIFTW)/$(JRE_IFTW_BUNDLE_NAME).exe
  endif
	$(RM) $(INSTALLERFILESIFTW)/setup.exe
	$(MV) $(INSTALLERFILESIFTW)/*msi $(INSTALLERFILESIFTW)/jre$(JDK_VERSION)-iftw.msi 
	$(CP) $(INSTALLERMSIFILE) $(INSTALLERFILESIFTW)/jre$(JDK_VERSION).msi
  ifeq ($(SIGN_FLAG),1)
	$(FULL_SIGNER) -n "JRE 1.5.0 Installer" -i http://java.com/ $(INSTALLERFILESIFTW)/jre$(JDK_VERSION).msi
  endif
	$(CD) $(INSTALLERFILESIFTW); $(ZIPEXE) -r $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).zip *
	$(CP) $(CURRENTDIR)/jre/privatejreFeature.idt $(JREISHIELDDIR)/Feature.idt
	$(CAT) $(CURRENTDIR)/jre/ActionText.idt | $(SED_COPY_TEXT) > $(JREISHIELDDIR)/ActionText.idt
	$(RM) $(TMPDIR)/jre.ism
	$(WSCRIPT) $(TMPDIR)/jrefile.vbs
	$(BUILDER) -p $(TMPDIR)/jre.ism -r "privatejre" -c COMP
	$(CP) -f $(INSTALLERFILESPRIVATEJRE)/*msi $(TMPDIR)/jre.msi
	$(CP) -f $(INSTALLERFILESPRIVATEJRE)/*.cab $(TMPDIR)/
	$(CP) -f $(INSTALLERFILESPRIVATEJRE)/1041.mst $(TMPDIR)/jre1041.MST
endif

ifeq ($(ARCH_DATA_MODEL),32)
setup_installer_files: $(COPIER) $(PATCHER) $(FONTCHECKER) $(SUNGRAPHIC1) $(SUNGRAPHIC2) $(LAUNCHER) $(JREISHIELDDIR)
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/bin/RegUtils.dll $(TMPDIR)
	$(CP) -f $(NEW_IMAGE_JRE_DIR)/bin/msvcrt.dll $(TMPDIR)
	$(CP) -f $(ALT_OUTPUTDIR)/tmp/deploy/plugin/RegUtilsMSI/$(OBJDIRNAME)/RegUtilsMSI.dll $(TMPDIR)
else
setup_installer_files: $(SUNGRAPHIC1) $(SUNGRAPHIC2) $(LAUNCHER) $(JREISHIELDDIR)
endif
	$(CP) $(CURRENTDIR)/jre.isv $(TMPDIR)
	$(CD) $(CURRENTDIR)/jre/; $(FIND) . -depth | $(GREP) -v "SCCS" | $(CPIO) -pdum $(JREISHIELDDIR)/
	$(CHMOD) 777 $(JREISHIELDDIR)/*
	$(SED_CAB_LOCATION) $(CURRENTDIR)/jre/ISReleaseExtended.idt > $(JREISHIELDDIR)/ISReleaseExtended.idt
	$(CAT) $(CURRENTDIR)/jre/Property.idt | $(SED_JDK_VERSION) | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_FULL_VERSION) | $(SED_ALL_SPACE_TEXT) > $(JREISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/jre/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) > $(JREISHIELDDIR)/_SummaryInformation.idt
	$(SED_FULL_VERSION) $(CURRENTDIR)/jre/Directory.idt | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(JREISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/jre/File.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/File.idt
	$(CAT) $(CURRENTDIR)/jre/DuplicateFile.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/DuplicateFile.idt
	$(CAT) $(CURRENTDIR)/jre/Component.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/jre/Registry.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/Registry.idt
	$(CAT) $(CURRENTDIR)/jre/FeatureComponents.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/FeatureComponents.idt
	$(CAT) $(CURRENTDIR)/jre/ISComponentExtended.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/ISComponentExtended.idt
	$(CAT) $(CURRENTDIR)/jre/CustomAction.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/CustomAction.idt
	$(CAT) $(CURRENTDIR)/jre/ISString.idt | $(SED_FULL_VERSION) > $(JREISHIELDDIR)/ISString.idt
	$(CAT) $(CURRENTDIR)/jre/ReserveCost.idt | $(PROCESS_FILES_COMP) | $(SED_ALL_RESERVE) > $(JREISHIELDDIR)/ReserveCost.idt
	$(CAT) $(CURRENTDIR)/jre/ActionText.idt | $(SED_COPY_TEXT) > $(JREISHIELDDIR)/ActionText.idt
ifeq ($(ARCH_DATA_MODEL),32)
	$(MAKE) create_removefile
	$(CP) $(TMPDIR)/removefile $(JREISHIELDDIR)/RemoveFile.idt
	$(MAKE) create_directory
	$(CP) $(TMPDIR)/directory $(JREISHIELDDIR)/Directory.idt
else
	$(CP) -fr $(CURRENTDIR)/64bit/* $(JREISHIELDDIR)/
	$(SED_FULL_VERSION) $(CURRENTDIR)/64bit/Directory.idt | $(SED_BASE_IMAGE_FULL_VERSION) | $(SED_NEW_IMAGE_FULL_VERSION) > $(JREISHIELDDIR)/Directory.idt
	$(CAT) $(CURRENTDIR)/64bit/Component.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/Component.idt
	$(CAT) $(CURRENTDIR)/64bit/Property.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) | $(SED_FULL_VERSION) | $(SED_ALL_SPACE_TEXT) > $(JREISHIELDDIR)/Property.idt
	$(CAT) $(CURRENTDIR)/64bit/ISComponentExtended.idt | $(PROCESS_FILES_COMP) > $(JREISHIELDDIR)/ISComponentExtended.idt
	$(CAT) $(CURRENTDIR)/64bit/_SummaryInformation.idt | $(PROCESS_FILES_COMP) | $(SED_UPDATE_VERSION) > $(JREISHIELDDIR)/_SummaryInformation.idt
	regsvr32.exe /s $(ISHIELDDIR)/System/ISWIAutomation.dll
endif
	$(CAT) $(CURRENTDIR)/../file.vbs | $(SED_BINARY_LOCATION) | $(SED_TEXT_LOCATION) | $(SED_AUTOMATION_LAYER) > $(TMPDIR)/jrefile.vbs
	$(WSCRIPT) $(TMPDIR)/jrefile.vbs

#
# Copy base image to be bundled with installer.
#
ifeq ($(ARCH_DATA_MODEL),32)
setup_image: $(ZIPPER) 
else
setup_image:
endif
	@$(ECHO) Cleaning old temporary install image
	$(CHMOD) -Rf 777 $(TMPJREIMAGE)
	$(RM) -r $(TMPJREIMAGE)
	$(MKDIR) -p $(TMPJREIMAGE)
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@#
	@# Patch build - copy cabs from old base image repository
	@#
	$(CP) -r $(ALT_BASE_IMAGE_DIR)/jre-repository-image/*.zip $(TMPJREIMAGE)
	$(CP) $(NEW_IMAGE_JRE_DIR)/COPYRIGHT $(TMPJREIMAGE)
else
	$(CP) -r $(NEW_IMAGE_JRE_DIR)/* $(TMPJREIMAGE)
	$(RM) -r $(TMPPACKED_JRE_JARDIR)
	$(MKDIR) -p $(TMPPACKED_JRE_JARDIR)
	$(MAKE) do_pack
ifeq ($(ARCH_DATA_MODEL),32)
	$(MAKE) do_zip
endif
endif

#
# Pack jars in base image.
#

do_pack:
ifneq ($(BASE_IMAGE_FULLVERSION), $(NEW_IMAGE_FULLVERSION))
	@#we pack/unpack the TMPJREIMAGE
	cd $(INSTALL_BUILDDIR)/pack; \
	$(MAKE) PACK_OUTPUTDIR=$(TMPPACKED_JRE_JARDIR) \
		VAR_IMAGE_DIR=$(TMPJREIMAGE) \
		JARS_TO_PACK="$(JRE_PACKED_JARS)" \
		pack-jars
	for i in $(JRE_PACKED_JARS) ; do \
	    if [ -f $(BASE_IMAGE_JRE_DIR)/$$i ]; then \
		$(RM) $(TMPJREIMAGE)/$$i || exit 1; \
		$(CP) $(TMPPACKED_JRE_JARDIR)/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPJREIMAGE)/`$(DIRNAME) $$i` || exit 1; \
	    fi ; \
	done
else
	for i in $(JRE_PACKED_JARS) ; do \
	    if [ -f $(JRE_IMAGE_DIR)/$$i ]; then \
		$(RM) $(TMPJREIMAGE)/$$i || exit 1; \
                $(CP) $(ALT_OUTPUTDIR)/pack/pack-jre-jars/`$(DIRNAME) $$i`/`$(BASENAME) $$i .jar`$(PACK_SUFFIX) $(TMPJREIMAGE)/`$(DIRNAME) $$i` || exit 1; \
	    fi ; \
	done
endif

##########################################################
#
# This is a workaround.  Install shield 7.04 makes creates
# a component for each sub-dir of our product, and therefore
# a .cab file for each component.  By zipping w/no compression
# we eliminate sub-dirs, and have less .cab files to host
do_zip: $(ZIPPER) $(TOUCHFILE) 
	$(MV) $(TMPJREIMAGE)/bin $(TMPDIR)/tmpbin
	$(MV) $(TMPJREIMAGE) $(TMPDIR)/tmpjreimage
	$(MKDIR) $(TMPJREIMAGE)
	$(MV) $(TMPDIR)/tmpbin $(TMPJREIMAGE)/bin
	$(RM) $(TMPJREIMAGE)/bin/RegUtils.dll
	$(RM) $(TMPJREIMAGE)/bin/msvcrt.dll
	$(MV) $(TMPJREIMAGE)/bin/eula.dll $(TMPDIR)
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/core1.zip *
	$(RM) -r $(TMPJREIMAGE)
	$(MV) $(TMPDIR)/tmpjreimage $(TMPJREIMAGE)
	$(MV) $(TMPJREIMAGE)/lib/rt.pack $(TMPDIR)/tmprt.pack
	$(MV) $(TMPJREIMAGE) $(TMPDIR)/tmpjreimage
	$(MKDIR) -p $(TMPJREIMAGE)/lib
	$(MV) $(TMPDIR)/tmprt.pack $(TMPJREIMAGE)/lib/rt.pack
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/core2.zip *
	$(RM) -r $(TMPJREIMAGE)
	$(MV) $(TMPDIR)/tmpjreimage $(TMPJREIMAGE)
	$(MV) $(TMPJREIMAGE)/lib/ext/localedata.pack $(TMPJREIMAGE)/lib/charsets.pack $(TMPDIR)
	$(MV) $(TMPJREIMAGE)/lib/fonts $(TMPDIR)
	$(MV) $(TMPJREIMAGE)/COPYRIGHT $(TMPDIR)
	$(MKDIR) -p $(TMPJREIMAGE)/lib/fonts
	$(MV) $(TMPDIR)/fonts/LucidaSansRegular.ttf $(TMPJREIMAGE)/lib/fonts
	$(MV) $(TMPJREIMAGE)/lib/cmm/PYCC.pf $(TMPDIR)
	$(RM) $(TMPJREIMAGE)/LICENSE*.rtf
	@if [ -x `$(ECHO) $(TMPJREIMAGE)/lib/audio` ]; then \
	    $(MV) $(TMPJREIMAGE)/lib/audio $(TMPDIR) ; \
	fi
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/core3.zip *
	$(RM) -r $(TMPJREIMAGE)/*
	$(MKDIR) -p $(TMPJREIMAGE)/lib/ext
	$(MV) $(TMPDIR)/localedata.pack $(TMPJREIMAGE)/lib/ext
	$(MV) $(TMPDIR)/charsets.pack $(TMPJREIMAGE)/lib
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/other.zip *
	$(RM) -r $(TMPJREIMAGE)/*
	$(MKDIR) -p $(TMPJREIMAGE)/lib/cmm
	$(MKDIR) -p $(TMPJREIMAGE)/bin
	$(MV) $(TMPDIR)/PYCC.pf $(TMPJREIMAGE)/lib/cmm
	$(MV) $(TMPDIR)/fonts $(TMPJREIMAGE)/lib
	$(MV) $(TMPDIR)/eula.dll $(TMPJREIMAGE)/bin
	$(CP) $(TMPDIR)/LICENSE*.rtf $(TMPJREIMAGE)/
	@if [ -x `$(ECHO) $(TMPDIR)/audio` ]; then \
	    $(MV) $(TMPDIR)/audio $(TMPJREIMAGE)/lib ;	\
	fi
	$(CD) $(TMPJREIMAGE) ; $(ZIPEXE) -0 -r $(TMPDIR)/extra.zip *
	$(RM) -r $(TMPJREIMAGE)/*
	$(MV) $(TMPDIR)/core1.zip $(TMPDIR)/core2.zip $(TMPDIR)/core3.zip $(TMPDIR)/other.zip $(TMPDIR)/extra.zip $(TMPDIR)/COPYRIGHT $(TMPJREIMAGE)
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
	@# $(TOUCHFILE) $(TMPJREIMAGE)

create_removefile:
	$(RM) $(TMPDIR)/removefile
	$(CAT) $(JREISHIELDDIR)/RemoveFile.idt | $(PROCESS_FILES_COMP) > $(TMPDIR)/removefile
	for dir in `$(FIND) $(JRE_IMAGE_DIR) -type d` ; do \
	  dirbase=`$(BASENAME) $$dir` ; \
	  if [ "$$dirbase" != "j2re-image" ] && [ "$$dirbase" != "ext" ]; then \
	    $(PRINTF) "$$dirbase\tjz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t\t$$dirbase\t3\n" >> $(TMPDIR)/removefile || exit 1; \
	    $(PRINTF) "file$$dirbase\tjz$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(JDK_UPDATE_ISHIELD_VERSION)\t*\t$$dirbase\t3\n" >> $(TMPDIR)/removefile || exit 1; \
	  fi ; \
	done;

create_directory:
	$(RM) $(TMPDIR)/directory
	$(CAT) $(JREISHIELDDIR)/Directory.idt | $(PROCESS_FILES_COMP) > $(TMPDIR)/directory
	for dir in `$(CD) $(JRE_IMAGE_DIR) ; $(FIND) . -name "*" -type d` ; do \
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
	    $(PRINTF) "$$dirbase\t$$dirparent\t$$dirdir\t\t0\n" >> $(TMPDIR)/directory || exit 1; \
	  fi ; \
	done;

.PHONY: clean all create_bundle setup_installer_files setup_image do_pack do_zip
