#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.10 04/06/10
#
#
# Most frequently used targets:
#
#    all            -- builds
#    clobber        -- clobber the builds
#
INSTALL_BUILDDIR=../../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

ifeq ($(ARCH_DATA_MODEL), 32)
     SOLARIS_PACKAGE_LIST_SHIP = SUNWj5rt SUNWj5cfg SUNWj5dev SUNWj5man SUNWj5jmp SUNWj5dmo
     SOLARIS_PACKAGE_LIST_BUILD = $(SOLARIS_PACKAGE_LIST_SHIP) SUNWj5dbg
else
     SOLARIS_PACKAGE_LIST_SHIP = SUNWj5rtx SUNWj5dvx SUNWj5dmx
     SOLARIS_PACKAGE_LIST_BUILD = $(SOLARIS_PACKAGE_LIST_SHIP) SUNWj5dbx
endif

all: sanity-bundles post-sanity-bundles bundle-jdk pkgdefs-bundle bundle-jdk-debug bundle-jre

clean:: bundle-jdk-debug-clobber bundle-jre-clobber bundle-jdk-clobber pkgdefs-clobber

pkgdefs-bundle:
	@# 
	@# Generate the Solaris SVR4 pkgs...
	@# JDK_VERSION used instead of RELEASE because inclusion of MILESTONES 
	@# isn't desireable.
	@#
	$(RM) -rf $(OUTPUTDIR)/pkgarchive
	$(MKDIR) -p $(OUTPUTDIR)/pkgarchive
	(unset MAKEFLAGS; $(CD) pkgdefs ; \
	     $(SOLARIS_MAKE) \
		SOLARIS_PACKAGE_LIST_BUILD="$(SOLARIS_PACKAGE_LIST_BUILD)" \
		ARCH="$(ARCH)" \
	        MAN_PAGE_FILES="$(JDK_MAN_PAGES)" \
	        MAN_DEST_PATH="$(DEST_MAN_PATH)" \
		RELEASE=$(JDK_VERSION) \
		RELEASE_NP=$(shell $(ECHO) $(JDK_VERSION) | $(SED) -e "s/_.*//") \
	        FULL_VERSION=$(FULL_VERSION) \
	        OUTPUTDIR=$(ABS_OUTPUTDIR) \
		JDK_IMAGE_DIR="$(JDK_IMAGE_DIR)" \
		JDK_DEBUG_IMAGE_DIR="$(JDK_DEBUG_IMAGE_DIR)" \
	        TOPDIR=$(shell $(CD) $(TOPDIR); $(PWD)) \
		UNIXCOMMAND_PATH=$(UNIXCOMMAND_PATH) \
		JDK_DOCS="$(JDK_NAMECHANGE_DOCLIST) README.html" world	)
  ifeq ($(ARCH_DATA_MODEL), 32)
	@# 
	@# Create tar.Z of packages
	@# FIX ME: missing Release-Notes and Developer-Guide, may not be in 1.3.1 and beyond
	@#
	for d in $(JDK_SOURCE_DOCLIST) $(JDK_NAMECHANGE_DOCLIST) $(LICENSE_DOCLIST); do \
		$(CP) $(JDK_IMAGE_DIR)/$$d $(OUTPUTDIR)/pkgarchive; \
	done ;
  endif
	$(CD) $(OUTPUTDIR)/pkgarchive ; \
	    $(TAR) -cfh - $(JDK_SOURCE_DOCLIST) $(JDK_NAMECHANGE_DOCLIST) \
		$(LICENSE_DOCLIST) $(SOLARIS_PACKAGE_LIST_SHIP) | \
		$(COMPRESS) - > $(JDK_BUNDLE_NAME).tar.Z
	$(MV) $(OUTPUTDIR)/pkgarchive/$(JDK_BUNDLE_NAME).tar.Z $(BIN_BUNDLEDIR)
	$(RM) $(OUTPUTDIR)/pkgarchive/$(JDK_SOURCE_DOCLIST) \
	      $(OUTPUTDIR)/pkgarchive/$(LICENSE_DOCLIST)
	@#
	@# Finished generating Solaris SVR4 pkgs...
	@#

bundle-jdk-clobber: pkgdefs-clobber

	$(RM) -r $(OUTPUTDIR)/$(JDK_TEMPORARY_DIR) $(OUTPUTDIR)/$(JDK_BUNDLE_NAME).zip
	$(RM) $(OUTPUTDIR)/$(JDK_BUNDLE_NAME).bin
	$(RM) $(BIN_BUNDLEDIR)/$(JDK_BUNDLE_NAME).sh

pkgdefs-clobber::
	$(RM) pkgdefs/.make.state pkgdefs/common/awk_pkginfo \
		pkgdefs/common/.make.state
	@for dir in $(PKGDEFS_SUBDIRS); do \
		$(CD) pkgdefs/$$dir;  \
			$(RM) $(PKGDEFS_REMNANTFILES) ; \
			$(CD) ../.. ; \
	done

$(BIN_BUNDLEDIR):
	$(MKDIR) -p $@

include $(INSTALL_BUILDDIR)/common/Sanity.gmk
include $(INSTALL_BUILDDIR)/common/Pack.gmk
include $(INSTALL_BUILDDIR)/common/Bundles.gmk

.PHONY: all clean bundle-jre bundle-jdk bundle-jdk-debug \
	bundle-jdk-debug-clobber bundle-jre-clobber \
	bundle-jdk-clobber pkgdefs-clobber
