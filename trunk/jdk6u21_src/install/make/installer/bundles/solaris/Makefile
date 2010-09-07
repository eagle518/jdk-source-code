#
# Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.25 10/05/12
#
#
# Most frequently used targets:
#
#    all            -- builds
#    clobber        -- clobber the builds
#
INSTALL_BUILDDIR=../../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

#
# NOTE:  SUNWdefault-java is a Solaris 10 deliverable ONLY, it is intentionally 
#        not part of the shipping list.
#
ifeq ($(ARCH_DATA_MODEL), 32)
     JAVADB_PACKAGE_LIST = $(shell $(CD) $(COPKG_JAVADB_DIR)/solaris ; $(LS) -1 SUNWjavadb-*-$(SOLARIS_ARCH)-pkgbld.zip |$(CUT) -d'-' -f1-2 ) 
     SOLARIS_PACKAGE_LIST_SHIP = SUNWj6rt SUNWj6cfg SUNWj6dev SUNWj6man SUNWj6jmp SUNWj6dmo
     SOLARIS_PACKAGE_LIST_BUILD = $(SOLARIS_PACKAGE_LIST_SHIP) SUNWdefault-java
else
     SOLARIS_PACKAGE_LIST_SHIP = SUNWj6rtx SUNWj6dvx SUNWj6dmx
     SOLARIS_PACKAGE_LIST_BUILD = $(SOLARIS_PACKAGE_LIST_SHIP)
endif

#
# There is inconsitancy in what the ARCH identifier for Intel processors
# should be.  Sometimes its i386, other times its i586, and on rare occasions
# its actually i486.  Solaris is consistant in using i386, so undue the
# mapping from i386 to i586 done in the upper level Makefiles.
#
ifeq ($(ARCH), i586)
    SOLARIS_ARCH = i386
else
    SOLARIS_ARCH = $(ARCH)
endif

# The following can be overridden on the command line with
# "gnumake DEBUG_IMAGES=0 all" at the top level
DEBUG_IMAGES ?= 1
ifeq ($(DEBUG_IMAGES), 1)
    ALL_TARGETS = sanity-bundles post-sanity-bundles bundle-jdk pkgdefs-bundle bundle-jdk-debug bundle-jre
    CLEAN_TARGETS = bundle-jdk-debug-clobber bundle-jre-clobber bundle-jdk-clobber pkgdefs-clobber
else
    ALL_TARGETS = sanity-bundles post-sanity-bundles bundle-jdk pkgdefs-bundle bundle-jre
    CLEAN_TARGETS = bundle-jre-clobber bundle-jdk-clobber pkgdefs-clobber
endif

all: $(ALL_TARGETS)

clean:: $(CLEAN_TARGETS)

pkgdefs-bundle:
	@# 
	@# Generate the Solaris SVR4 pkgs...
	@# JDK_VERSION used instead of RELEASE because inclusion of MILESTONES 
	@# isn't desireable.
	@#
	$(RM) -rf $(OUTPUTDIR)/pkgarchive
	$(MKDIR) -p $(OUTPUTDIR)/pkgarchive
	(unset MAKEFLAGS; $(CD) pkgdefs ; \
	     $(MAKE) \
		SOLARIS_PACKAGE_LIST_BUILD="$(SOLARIS_PACKAGE_LIST_BUILD)" \
		ARCH="$(SOLARIS_ARCH)" \
		RELEASE=$(JDK_VERSION) \
		RELEASE_NP=$(shell $(ECHO) $(JDK_VERSION) | $(SED) -e "s/_.*//") \
	        FULL_VERSION=$(FULL_VERSION) \
	        OUTPUTDIR=$(ABS_OUTPUTDIR) \
		JDK_IMAGE_DIR="$(JDK_IMAGE_DIR)" \
		JDK_DEBUG_IMAGE_DIR="$(JDK_DEBUG_IMAGE_DIR)" \
	        TOPDIR=$(shell $(CD) $(TOPDIR); $(PWD)) \
		UNIXCOMMAND_PATH=$(UNIXCOMMAND_PATH) \
		JDK_DOCS="$(JDK_SOURCE_DOCLIST) $(LICENSE_DOCLIST)" world	)
  ifeq ($(ARCH_DATA_MODEL), 32)
	@# 
	@# Create tar.Z of packages
	@#
	for d in COPYRIGHT $(JDK_SOURCE_DOCLIST) $(LICENSE_DOCLIST); do \
		$(CP) $(JDK_IMAGE_DIR)/$$d $(OUTPUTDIR)/pkgarchive; \
	done ;
	@#
	@# Extract and copy in the JavaDB packages, just for 32bit.
	@#
	$(CP) $(COPKG_JAVADB_DIR)/solaris/SUNWjavadb-*-$(SOLARIS_ARCH)-pkgbld.zip $(OUTPUTDIR)/pkgarchive
	$(CD) $(OUTPUTDIR)/pkgarchive ; \
		for db in SUNWjavadb-*-$(SOLARIS_ARCH)-pkgbld.zip ; do \
			$(UNZIP) -o $$db ; \
			$(RM) $$db ; \
		done  
  endif
	$(CD) $(OUTPUTDIR)/pkgarchive ; \
	    $(TAR) -cfh - COPYRIGHT $(JDK_SOURCE_DOCLIST) $(LICENSE_DOCLIST) \
		$(SOLARIS_PACKAGE_LIST_SHIP) $(JAVADB_PACKAGE_LIST) | \
		$(COMPRESS) - > $(JDK_BUNDLE_NAME).tar.Z
	$(MV) $(OUTPUTDIR)/pkgarchive/$(JDK_BUNDLE_NAME).tar.Z $(BIN_BUNDLEDIR)
	$(RM) COPYRIGHT $(JDK_SOURCE_DOCLIST) $(LICENSE_DOCLIST)
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
	@for dir in $(SOLARIS_PACKAGE_LIST_BUILD); do \
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
