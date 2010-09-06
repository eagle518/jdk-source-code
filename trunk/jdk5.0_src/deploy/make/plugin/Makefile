#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.35 03/12/19
#
# Makefile to build the Java Plug-in components
#
# Build each subdirectory
DEPLOY_BUILDDIR = ..
PLUGIN_TOP = ../..
DEPLOY_TOPDIR = ../..
include ./common/config.gmk

# for zh_HK
include ../common/Resources.gmk

SUBDIRS = java adapter nscore
ifeq ($(PLATFORM), windows)
  SUBDIRS += jupdate com jpishare regutils regutilsmsi controlpanel jinstall nsstub eula
else
  SUBDIRS += controlpanel l10n jvmexec jvmnative
  ADAPTERDIRS = jcomponent ns4-adapter
endif

SUBDIRS += converter

all optimized debug :: deploy-jar $(VERSION_DEST) $(SUBDIRS)

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
	-$(RM) -r $(DEPLOYCLASSDESTDIR)  $(DEPLOYDBGCLASSDESTDIR) $(DEPLOY_COMMON_JAR) $(DEPLOY_COMMON_DBG_JAR) $(DEPLOY_COMMON_LIB) $(DEPLOYOBJDIR)
#Since deploy.jar/libdeploy.so is not built against J2SE rules, we need to remove it manually
	-$(RM) -r $(DEPLOY_OUTPUT_JAR) $(DEPLOY_OUTPUT_DBG_JAR) $(DEPLOY_OUTPUT_LIB)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

ifeq ($(PLATFORM), windows)
# versioning for windows
CONVERT_MAJOR = $(SED) -e "s/_PLUGIN_MAJOR_VER/$(JDK_MAJOR_VERSION)/g"
CONVERT_MINOR = $(SED) -e "s/_PLUGIN_MINOR_VER/$(JDK_MINOR_VERSION)/g"
CONVERT_MICRO = $(SED) -e "s/_PLUGIN_MICRO_VER/$(JDK_MICRO_VERSION)/g"
CONVERT_FULL  = $(SED) -e "s/_PLUGIN_FULL_VER/$(FULL_VERSION)/g"
CONVERT_CLSID = $(SED) -e "s/_CLSID_CHAR/$(CONVERT_CLSID_CHAR)/g"

ifndef JDK_UPDATE_VERSION
  ifeq (0, $(JDK_MICRO_VERSION))
    CONVERT_MIMETYPE = $(SED) -e "s/_PLUGIN_MAJOR_MIMETYPE/$(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION)/g"
  endif
  CONVERT_UPDAT = $(SED) -e "s/_PLUGIN_UPDAT_VER/00/g"
  CONVERT_UNDERSCORE_UPDAT = $(SED) -e "s/_PLUGIN_UNDERSCORE_UPDAT_VER//g"
  CONVERT_CAB_VER = $(SED) -e "s/_CAB_VER/0/g"
else
  CONVERT_MIMETYPE = $(SED) -e "s/_PLUGIN_MAJOR_MIMETYPE/$(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)/g"
  CONVERT_UPDAT = $(SED) -e "s/_PLUGIN_UPDAT_VER/$(CAB_CHAR1)$(CAB_CHAR2)/g"
  CONVERT_UNDERSCORE_UPDAT = $(SED) -e "s/_PLUGIN_UNDERSCORE_UPDAT_VER/"_$(JDK_UPDATE_VERSION)"/g"
  CONVERT_CAB_VER = $(SED) -e "s/_CAB_VER/$(PLUGIN_CAB_VERSION)/g"
endif

SUBSTITUTE_VERSIONS = $(CONVERT_MAJOR) | $(CONVERT_MINOR) | $(CONVERT_MICRO) | $(CONVERT_UPDAT) | $(CONVERT_UNDERSCORE_UPDAT) | $(CONVERT_FULL) | $(CONVERT_CAB_VER) | $(CONVERT_CLSID) | $(CONVERT_MIMETYPE)

$(VERSION_DEST):$(VERSION_SRC)
	$(CAT) $< | $(SUBSTITUTE_VERSIONS) > $@
endif

include ../common/Release.gmk

# for deploy.jar
include ../common/FILES_c.gmk
include ../common/FILES_java.gmk
include ../common/DeployRules.gmk
include ../common/Sanity.gmk

ifeq ($(PLATFORM), windows)
   include ../common/DeployRules-windows.gmk
   DEPLOY_COMMON_LIB = $(DEPLOY_COMMON_DLL)
else
   include ../common/DeployRules-solaris.gmk
   DEPLOY_COMMON_LIB = $(DEPLOY_COMMON_SO)
endif

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clean $(SUBDIRS)
