#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.11 10/03/23
#
#
# Most frequently used targets:
#
#    all            -- builds
#    clobber        -- clobber the builds
#
INSTALL_BUILDDIR=../../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

# The following can be overridden on the command line with
# "gnumake DEBUG_IMAGES=0 all" at the top level
DEBUG_IMAGES ?= 1
ifeq ($(DEBUG_IMAGES), 1)
    ALL_TARGETS = sanity-binaries post-sanity-binaries docs-tar binaries-jdk binaries-jdk-debug binaries-jre
    CLEAN_TARGETS = binaries-jre-clobber binaries-jdk-clobber binaries-jdk-debug-clobber
else
    ALL_TARGETS = sanity-binaries post-sanity-binaries docs-tar binaries-jdk binaries-jre
    CLEAN_TARGETS = binaries-jre-clobber binaries-jdk-clobber
endif

all: $(ALL_TARGETS)

binaries-clobber: $(CLEAN_TARGETS)

clean:: binaries-clobber

binaries-xpi-clobber:
	$(RM) -r $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).xpi

######################################################
# JRE XPI (This is currently for Linux only)
######################################################
binaries-xpi:: $(BIN_BUNDLEDIR) install.js
	$(MV) $(JRE_IMAGE_DIR) $(BIN_BUNDLEDIR)/$(JRE_TEMPORARY_DIR)
	$(CAT) install.js | $(SED) -e "s/JDK_VERSION/$(JDK_VERSION)/g" > $(BIN_BUNDLEDIR)/install.js
	$(CP) symlink.sh $(BIN_BUNDLEDIR)/symlink.sh
	$(CD) $(BIN_BUNDLEDIR); \
	$(ZIPEXE) -r $(JRE_BUNDLE_NAME).xpi $(JRE_TEMPORARY_DIR) install.js symlink.sh
	$(RM) $(BIN_BUNDLEDIR)/install.js
	$(RM) $(BIN_BUNDLEDIR)/symlink.sh
	$(MV) $(BIN_BUNDLEDIR)/$(JRE_TEMPORARY_DIR) $(JRE_IMAGE_DIR)

include $(INSTALL_BUILDDIR)/common/Binaries.gmk
include $(INSTALL_BUILDDIR)/common/Sanity.gmk

#
# Phonies to avoid accidents.
#
.PHONY: all clean docs-tar binaries-jre binaries-jdk \
	binaries-jdk-debug binaries-jre-clobber \
	binaries-jdk-clobber binaries-jdk-debug-clobber
