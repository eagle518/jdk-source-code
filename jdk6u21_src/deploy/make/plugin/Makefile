#
# @(#)Makefile	1.69 10/03/24
#
# Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# Makefile to recursively build the Java Plug-in components.
#

DEPLOY_TOPDIR = ../..
include ./common/Defs.gmk

#define deploy amd64 build switch
BUILD_REGUTILS_ONLY=false
ifeq ($(PLATFORM), windows)
ifeq ($(ARCH_DATA_MODEL), 64)
  ifneq ($(BUILD_DEPLOY_AMD64) , true)
    BUILD_REGUTILS_ONLY=true
  endif
endif
endif

ifeq ($(BUILD_REGUTILS_ONLY), true)
  SUBDIRS = regutils regutilsmsi filesinuse 
else
# Build each subdirectory
SUBDIRS = java ffjavaconsole plugin2
ifeq ($(ARCH_DATA_MODEL), 32)
  SUBDIRS += adapter nscore 
endif

ifeq ($(PLATFORM), windows)
  SUBDIRS += toolkit jbroker ssv ssvagent regutils regutilsmsi filesinuse controlpanel eula
  ifeq ($(ARCH_DATA_MODEL), 32)
    SUBDIRS += com nsstub jpishare
  endif
else
  SUBDIRS += controlpanel l10n jvmexec jvmnative
  ADAPTERDIRS = jcomponent ns4-adapter
endif

ifndef FASTJRE
  SUBDIRS += converter
endif
endif

# Use wrapping build script will set environ vars, otherwise use this default
ifndef AU_VERSION_DEFINED
 AU_MAJOR_VERSION = 2
 AU_MINOR_VERSION = 0
 AU_MICRO_VERSION = 0
 AU_BUILD_VERSION = 0
endif

all build:: deploy-jar $(VERSION_DEST) $(AU_VERSION_DEST) $(SUBDIRS)

plus: $(VERSION_DEST)

enhanced_plus: $(VERSION_DEST)
	$(CD) filesinuse; $(MAKE) all
	$(CD) regutilsmsi; $(MAKE) all

adapter-all:: $(ADAPTERDIRS)

$(ADAPTERDIRS)::
	$(MAKE) -C $@ all

clean:
	@for i in $(SUBDIRS) ; do \
	    $(ECHO) ">>>Recursively making "$$i" "clean" @ `$(DATE)` ..."; \
	    $(CD) $$i; $(MAKE) clobber RELEASE="$(RELEASE)" FULL_VERSION="$(FULL_VERSION)" \
            || exit 1; $(CD) ..; \
	    $(ECHO) "<<<Finished Recursively making "$$i" all @ `$(DATE)`." ; \
	done
	# The next group of directories should really have their own target so
	# that the common Deployment files can be cleaded up separately.
	$(RM) -r $(DEPLOYCLASSDESTDIR) \
	         $(DEPLOY_COMMON_JAR)  \
	         $(DEPLOYOBJDIR)       \
	         $(DEPLOY_COMMON_LIB)
	#Since deploy.jar/libdeploy.so is not built against J2SE rules, we need
	# to remove it manually
	$(RM) -r $(DEPLOY_OUTPUT_JAR) \
	         $(DEPLOY_OUTPUT_LIB)







$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

# versioning for windows
CONVERT_MAJOR = $(SED) -e "s/_PLUGIN_MAJOR_VER/$(JDK_MAJOR_VERSION)/g"
CONVERT_MINOR = $(SED) -e "s/_PLUGIN_MINOR_VER/$(JDK_MINOR_VERSION)/g"
CONVERT_MICRO = $(SED) -e "s/_PLUGIN_MICRO_VER/$(JDK_MICRO_VERSION)/g"
CONVERT_FULL  = $(SED) -e "s/_PLUGIN_FULL_VER/$(FULL_VERSION)/g"
CONVERT_CLSID = $(SED) -e "s/_CLSID_CHAR/$(CONVERT_CLSID_CHAR)/g"
CONVERT_MIMETYPE = $(SED) -e "s/_PLUGIN_MAJOR_MIMETYPE/$(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)/g"
CONVERT_PRODUCT_VERSION = $(SED) -e "s/_RELEASE/$(RELEASE)/g"

ifndef JDK_UPDATE_VERSION
  ifeq (0, $(JDK_MICRO_VERSION))
    CONVERT_MIMETYPE = $(SED) -e "s/_PLUGIN_MAJOR_MIMETYPE/$(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION)/g"
  endif
  CONVERT_UPDAT = $(SED) -e "s/_PLUGIN_UPDAT_VER/00/g"
  CONVERT_UNDERSCORE_UPDAT = $(SED) -e "s/_PLUGIN_UNDERSCORE_UPDAT_VER//g"
  CONVERT_CAB_VER = $(SED) -e "s/_CAB_VER/0/g"
else
  CONVERT_UPDAT = $(SED) -e "s/_PLUGIN_UPDAT_VER/$(CAB_CHAR1)$(CAB_CHAR2)/g"
  CONVERT_UNDERSCORE_UPDAT = $(SED) -e "s/_PLUGIN_UNDERSCORE_UPDAT_VER/"_$(JDK_UPDATE_VERSION)"/g"
  CONVERT_CAB_VER = $(SED) -e "s/_CAB_VER/$(PLUGIN_CAB_VERSION)/g"
endif

CONVERT_BUILD_NUMBER = $(SED) -e "s/_PLUGIN_BUILD_NUMBER/$(PLUGIN_BUILD_NUMBER)/g"

CONVERT_SECURITY_BASELINE_131 = $(SED) -e "s/_SECURITY_BASELINE_131/$(SECURITY_BASELINE_131)/g"

CONVERT_SECURITY_BASELINE_142 = $(SED) -e "s/_SECURITY_BASELINE_142/$(SECURITY_BASELINE_142)/g"

CONVERT_SECURITY_BASELINE_150 = $(SED) -e "s/_SECURITY_BASELINE_150/$(SECURITY_BASELINE_150)/g"

CONVERT_SECURITY_BASELINE_160 = $(SED) -e "s/_SECURITY_BASELINE_160/$(SECURITY_BASELINE_160)/g"

SUBSTITUTE_VERSIONS = $(CONVERT_MAJOR) | $(CONVERT_MINOR) | $(CONVERT_MICRO) | $(CONVERT_UPDAT) | $(CONVERT_UNDERSCORE_UPDAT) | $(CONVERT_FULL) | $(CONVERT_CAB_VER) | $(CONVERT_CLSID) | $(CONVERT_MIMETYPE) | $(CONVERT_BUILD_NUMBER) | $(CONVERT_SECURITY_BASELINE_131) |$(CONVERT_SECURITY_BASELINE_142) | $(CONVERT_SECURITY_BASELINE_150) | $(CONVERT_SECURITY_BASELINE_160) | $(CONVERT_PRODUCT_VERSION)

# generate a native header file that defines version info
$(VERSION_DEST): $(VERSION_SRC)
	$(RM) $@
	$(MKDIR) -p $(@D)
	$(CAT) $< | $(SUBSTITUTE_VERSIONS) > $@

CONVERT_AU_MAJOR = $(SED) -e "s/_AU_MAJOR_VER/$(AU_MAJOR_VERSION)/g"
CONVERT_AU_MINOR = $(SED) -e "s/_AU_MINOR_VER/$(AU_MINOR_VERSION)/g"
CONVERT_AU_MICRO = $(SED) -e "s/_AU_MICRO_VER/$(AU_MICRO_VERSION)/g"
CONVERT_AU_BUILD = $(SED) -e "s/_AU_BUILD_VER/$(AU_BUILD_VERSION)/g"

AU_SUBSTITUTE_VERSIONS = $(CONVERT_AU_MAJOR) | $(CONVERT_AU_MINOR) | $(CONVERT_AU_MICRO) | $(CONVERT_AU_BUILD) 
# generate a native header file that defines version info for AU 2.0
$(AU_VERSION_DEST): $(AU_VERSION_SRC)
	$(RM) $@
	$(MKDIR) -p $(@D)
	$(CAT) $< | $(AU_SUBSTITUTE_VERSIONS) > $@

include ../common/Release.gmk

# for deploy.jar
include ../common/FILES_c.gmk
include ../common/FILES_java.gmk
include ../common/DeployRules.gmk
include ../common/Sanity.gmk

ifeq ($(PLATFORM), windows)
   include ../common/DeployRules-windows.gmk
else
   include ../common/DeployRules-solaris.gmk
endif

# for zh_HK
include ../common/Resources.gmk

#
# Phonies to avoid accidents.
#
.PHONY: all build clean $(SUBDIRS)
