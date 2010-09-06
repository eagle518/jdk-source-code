#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.11 04/05/21 
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
PRODUCT = cabfile

TMPDIR = $(TEMP_DIR)/$(PRODUCT)
PLUGIN_TMPDIR = $(TEMP_DIR)/deploy/plugin

include $(INSTALL_BUILDDIR)/common/Defs.gmk

ifndef JDK_UPDATE_VERSION
  CONVERT_UPDATE = $(SED) -e "s/JDKUPDATE//g"
  UPDATE_NUMBER=0
  NODOTVERSION=$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)
else
  CONVERT_UPDATE = $(SED) -e "s/JDKUPDATE/_$(JDK_UPDATE_VERSION)/g"
  UPDATE_NUMBER=$(shell $(ECHO) $(JDK_UPDATE_VERSION) | $(NAWK) '{print substr($$0, 2, 1);}')
  NODOTVERSION=$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)_$(JDK_UPDATE_VERSION)
endif


CONVERT_MAJOR = $(SED) -e "s/JDKMAJOR/$(JDK_MAJOR_VERSION)/g"
CONVERT_MINOR = $(SED) -e "s/JDKMINOR/$(JDK_MINOR_VERSION)/g"
CONVERT_MICRO = $(SED) -e "s/JDKMICRO/$(JDK_MICRO_VERSION)/g"
CONVERT_UPDATE_NUMBER = $(SED) -e "s/UPDATENUMBER/$(UPDATE_NUMBER)/g"
CONVERT_NOTDOTVERSION = $(SED) -e "s/NOTDOTVERSION/$(NOTDOTVERSION)/g"


PROCESS_FILES = $(CONVERT_MAJOR) | $(CONVERT_MINOR) | $(CONVERT_MICRO) | $(CONVERT_UPDATE) | $(CONVERT_UPDATE_NUMBER) | $(CONVERT_NODOTVERSION) 

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
all: $(TMPDIR) cab_only

###################### clean #############################
#
# Remove all produced files.
#
clean: 
	$(RM) $(BIN_BUNDLEDIR)/jinstall*
	$(RM) -r $(TMPDIR)

$(BIN_BUNDLEDIR):
	$(MKDIR) -p $@

$(TMPDIR):
	$(MKDIR) -p $@


###################### cab_only #############################
#
# Build only the installation bootstrap
#

# generate filename and webpage equivalents of the VERSION string:
# It used to be done this way in a shell script:
#    Notice that the original methods truncate only those _0's at the end
#       of the string. The new versions removes all _0's in the string.
#
#    filever=`print $version| sed 's/\./_/g' | sed 's/\(\_0\)\+$//g'`
#    webver=`print $version| sed 's/\(\.0\)\+$//g'`
#

WEB_VERSION_STRING   = $(subst .0,, $(VERSION))
PLUGIN_FILE_VERSION  = $(JDK_MAJOR_VERSION),$(JDK_MINOR_VERSION),$(JDK_MICRO_VERSION),$(UPDATE_NUMBER)


TMPDIR_DRIVE = $(filter %:,$(subst /, ,$(TMPDIR)))
ifeq ($(TMPDIR_DRIVE),)
TMPDIR_DRIVE = ./
endif
TMPDIR_FILESPEC = $(patsubst $(TMPDIR_DRIVE)%,%,$(TMPDIR))

cab_only: 
  ifeq ($(ARCH_DATA_MODEL), 32)
	@#$(SH) construct.sh $(VERSION) $(HTTPSERVER) $(SIGN_FLAG)
	@#
	@# Construct the .xml and .inf file for the Java Plug-in installation
	@# bootstrap. Also modify the .pfw project file for package for the 
	@# web tool which creates the .cab file
	@# 
	@# format is construct VERSION_VERSION HTTP_SERVER SIGNING_REQUIRED
	@#
	@$(ECHO) Construction Plug-in Installation bundles for version $(VERSION)
	@$(ECHO) File version is $(JDK_UNDERSCORE_VERSION), Web pages version is $(WEB_VERSION_STRING)
	@$(ECHO) Using file: $(JRE_NAME)-$(JDK_UNDERSCORE_VERSION)-win.exe 
	@#
	@#construct the .inf and .xml file
	@#
	@$(ECHO) s@#PLUGIN_VERSION#@$(PLUGIN_FILE_VERSION)@g               > $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#HTTP_SERVER#@$(HTTPSERVER)@g                    >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#PLUGIN_VERSION_FOR_FILE#@$(JDK_UNDERSCORE_VERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#JDK_VERSION#@$(JDK_VERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#MINORMICRO_VERSION#@$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#JDK_MAJOR_VERSION#@$(JDK_MAJOR_VERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#JDK_MINOR_VERSION#@$(JDK_MINOR_VERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#JDK_MICRO_VERSION#@$(JDK_MICRO_VERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#UPDATE_NUMBER#@$(UPDATE_NUMBER)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#NODOTVERSION#@$(NODOTVERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#FULL_VERSION#@$(FULL_VERSION)@g      >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#TMPDIR_DRIVE#@$(TMPDIR_DRIVE)@g                        >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#TMPDIR_FILESPEC#@$(subst /,\\\\,$(TMPDIR_FILESPEC))@g  >> $(TMPDIR)/cab-only-tempfile.sed
	$(SED) -f $(TMPDIR)/cab-only-tempfile.sed  plugin_jinstall_original.inf > $(TMPDIR)/jinstall-$(JDK_UNDERSCORE_VERSION).inf
	$(SED) -f $(TMPDIR)/cab-only-tempfile.sed  plugin_jinstall_original.xml > $(TMPDIR)/jinstall-$(JDK_UNDERSCORE_VERSION)-windows-i586.xml
	@#
	@#construct the cab file
	@#
	@$(ECHO) s@#WEB_VERSION#@$(WEB_VERSION_STRING)@g     >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#SIGN_FLAG#@$(SIGN_FLAG)@g                >> $(TMPDIR)/cab-only-tempfile.sed
	@$(ECHO) s@#OUTPUTDIR#@$(subst /,\\\\,$(TMPDIR))@g   >> $(TMPDIR)/cab-only-tempfile.sed
	$(SED) 's/\\/\\\\/g' $(TMPDIR)/cab-only-tempfile.sed  > $(TMPDIR)/slash-adjusted-cab-only-tempfile.sed
	@$(ECHO) Main web version is $(WEB_VERSION_STRING)
	@#
	$(SED) -f $(TMPDIR)/slash-adjusted-cab-only-tempfile.sed   cab.pfw   > $(TMPDIR)/cab-install.pfw
	@#
	$(CP) $(PLUGIN_TMPDIR)/jinstall/obj/jinstall.exe $(TMPDIR)/jinstall.exe
	$(IPFW)/pftwwiz.exe $(TMPDIR)/cab-install.pfw -a -s
	$(MV) $(TMPDIR)/jinstall*.cab $(TMPDIR)/jinstall*.xml $(BIN_BUNDLEDIR)
endif

.PHONY: all clean cab_only
