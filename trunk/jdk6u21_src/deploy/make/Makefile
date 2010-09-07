#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.39 10/03/24
#
# Makefile for building java-web-start 
#

#  
# Note: The deploy workspace is 32-bit only 
#
# Most frequently used targets:
#
#    all            -- build javaws and plugin
#    javaws-all     -- build only the javaws
#    plugin-all     -- build only the plugin
#    jqs-all        -- build only the java quick starter
#    clobber        -- clobber javaws and plugin
#    images         -- build the jaws and plugin images 

TOPDIR = ../..
DEPLOY_TOPDIR=..
include $(DEPLOY_TOPDIR)/make/common/Defs.gmk

all::
	@$(ECHO) $(PLATFORM) $(ARCH) $(RELEASE) deploy build started: $(shell $(DATE) '+%y-%m-%d %H:%M')

plus:
	$(CD) plugin; $(MAKE) plus
	$(CD) lzma; $(MAKE) all

enhanced_plus:
	$(CD) plugin; $(MAKE) enhanced_plus
	$(CD) upx; $(MAKE) all
	$(CD) lzma; $(MAKE) all

#
# Core.
#
JAVAWS_SUBDIR = javaws
PLUGIN_SUBDIR = plugin
JQS_SUBDIR    = jqs

# define deploy amd64 build switch
BUILD_REGUTILS_ONLY=false
ifeq ($(PLATFORM), windows)
ifeq ($(ARCH_DATA_MODEL), 64)
  ifneq ($(BUILD_DEPLOY_AMD64), true)
    BUILD_REGUTILS_ONLY=true
  endif
endif
endif

ifeq ($(BUILD_REGUTILS_ONLY), true)
  SUBDIRS = $(PLUGIN_SUBDIR)
  DEPLOY_TARGETS = sanity plugin-all
  ALL_DEPLOY_TARGETS = $(DEPLOY_TARGETS) images
else
  SUBDIRS = $(JAVAWS_SUBDIR) $(PLUGIN_SUBDIR)
  ifeq ($(BUILD_JQS), true)
    SUBDIRS += $(JQS_SUBDIR)
  endif
  DEPLOY_TARGETS = sanity javaws-all plugin-all
  ifeq ($(PLATFORM), windows)
    ifeq ($(BUILD_JQS), true)
      DEPLOY_TARGETS += jqs-all
    endif
  endif

  ALL_DEPLOY_TARGETS = $(DEPLOY_TARGETS) images
  ifdef DEV_ONLY
    ifdef NO_IMAGES
      ALL_DEPLOY_TARGETS = $(DEPLOY_TARGETS)
    endif
  endif
endif

all:: build

build: $(ALL_DEPLOY_TARGETS)

javaws: sanity-javaws
  ifeq ($(BUILD_REGUTILS_ONLY), false)
	$(CD) $(DEPLOY_TOPDIR)/make/javaws ; $(MAKE) installer-int
  endif

javaws-all: sanity-javaws
  ifeq ($(BUILD_REGUTILS_ONLY), false)
	$(CD) $(DEPLOY_TOPDIR)/make/javaws ; $(MAKE) installer-int sample apidoc
  endif

kernel-all:
  ifeq ($(BUILD_JKERNEL), true)
	$(CD) $(DEPLOY_TOPDIR)/make/kernel ; $(MAKE) all
  endif

plugin-all: sanity-plugin
   ifeq ($(BUILD_PLUGIN), true)
	$(CD) $(DEPLOY_TOPDIR)/make/plugin ; $(MAKE) all 
   endif

jqs-all:
	$(CD) $(DEPLOY_TOPDIR)/make/jqs ; $(MAKE) all

extra-comp-all:
	$(CD) $(DEPLOY_TOPDIR)/make/lzma ; $(MAKE) all

cmd-comp-all:
	$(CD) $(DEPLOY_TOPDIR)/make/upx ; $(MAKE) all

#
# Build the images for both plugin and webstart
# Note: webstart temp image must already be built.
#
images:
	$(CD) $(DEPLOY_TOPDIR)/make/plugin ; $(MAKE) deploy-images BUILD_PLUGIN=$(BUILD_PLUGIN)

#
# Sanity checks.
#
include $(DEPLOY_TOPDIR)/make/common/Sanity.gmk

dummy := $(shell $(MKDIR) -p $(OUTPUTDIR) $(TEMPDIR))

clean: clobber

# cleanup everything. If the workspace is not being built by the control
# workspace, and if it is a Teamware workspace, then see if there are
# any files which are not under SCCS control.
clobber:
	@$(ECHO) ">>>Recursively making "$(JAVAWS_SUBDIR)" "clobber" @ `$(DATE)` ..."
	@$(CD) $(JAVAWS_SUBDIR); $(MAKE) clobber || exit 1; $(CD) ..
	@$(ECHO) "<<<Finished Recursively making "$(JAVAWS_SUBDIR)" clobber @ `$(DATE)`."
	@$(ECHO) ">>>Recursively making "$(PLUGIN_SUBDIR)" "clean" @ `$(DATE)` ..."
	@$(CD) $(PLUGIN_SUBDIR); $(MAKE) clean RELEASE="$(RELEASE)" \
		FULL_VERSION="$(FULL_VERSION)" || exit 1; $(CD) ..
	@$(ECHO) "<<<Finished Recursively making "$(PLUGIN_SUBDIR)" clean @ `$(DATE)`."
	$(CD) $(DEPLOY_TOPDIR)/make/plugin ; $(MAKE) deploy-images-clobber
ifeq ($(PLATFORM), windows)
	@$(ECHO) ">>>Recursively making "$(JQS_SUBDIR)" "clobber" @ `$(DATE)` ..."
	@$(CD) $(JQS_SUBDIR); $(MAKE) clobber || exit 1; $(CD) ..
	@$(ECHO) "<<<Finished Recursively making "$(JQS_SUBDIR)" clobber @ `$(DATE)`."
endif
	@$(ECHO) ">>>Clobber residual files in OUTPUT deploy folders $(OUTPUTDIR)/tmp/deploy $(OUTPUTDIR)/tmp/l10n/deploy @ `$(DATE)` ..."
	@$(RM) -rf $(OUTPUTDIR)/tmp/deploy
	@$(RM) -rf $(OUTPUTDIR)/tmp/l10n/deploy
	@$(ECHO) "<<<Finished Clobber residual files @ `$(DATE)`."


# this should be the last rule in this file:
all::
	@$(ECHO) $(ARCH) "Deploy Build finished:  " $(FULL_VERSION)

#
# Phonies to avoid accidents.
#
.PHONY: all build javaws-all plugin-all images clobber clean
