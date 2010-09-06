#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.8 03/12/19
#
#
# Most frequently used targets:
#
#    all            -- builds
#    clobber        -- clobber the builds
#
INSTALL_BUILDDIR=../../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

all: sanity-binaries post-sanity-binaries docs-tar binaries-jdk binaries-jdk-debug binaries-jre

binaries-clobber: binaries-jre-clobber binaries-jdk-clobber binaries-jdk-debug-clobber

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
