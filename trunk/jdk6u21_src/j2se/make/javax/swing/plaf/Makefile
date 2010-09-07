#
# @(#)Makefile	1.42 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR  = ../../..
PACKAGE   = javax.swing.plaf
PRODUCT   = com
SWING_SRC = $(SHARE_SRC)/classes/javax/swing
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES.gmk
AUTO_FILES_JAVA_DIRS = javax/swing/plaf sun/swing com/sun/java/swing/plaf

# Nimbus is handled in its own directory
AUTO_JAVA_PRUNE = nimbus

MISC_FILES = $(MISC_SWING_FILES)
FILES_java = $(FILES_SWING_java)


#
# Resources
#
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
	com/sun/swing/internal/plaf/basic/resources/basic.properties \
	com/sun/swing/internal/plaf/metal/resources/metal.properties \
	com/sun/swing/internal/plaf/synth/resources/synth.properties \
	com/sun/java/swing/plaf/motif/resources/motif.properties \
	com/sun/java/swing/plaf/windows/resources/windows.properties \
	com/sun/java/swing/plaf/gtk/resources/gtk.properties


LOCALE_SET_DEFINITION = j2re
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk


#
# Rules
#
build: other_files nimbus

clean clobber:: classes.clean nimbus.clean

nimbus:
	@$(ECHO) ">>>Recursively making nimbus all @ `$(DATE)` ..."; \
	$(CD) nimbus; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) all\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making nimbus all @ `$(DATE)`." ; \

nimbus.clean:
	@$(ECHO) ">>>Recursively making "$@" @ `$(DATE)` ..."; \
	$(CD) nimbus; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) clobber\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making "$@" @ `$(DATE)`." ; \

.PHONY: nimbus nimbus.clean

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

other_files: $(MISC_FILES)

$(CLASSBINDIR)/%.wav: $(SHARE_SRC)/classes/%.wav
	$(install-file)

$(CLASSBINDIR)/%.gif: $(SHARE_SRC)/classes/%.gif
	$(install-file)

$(CLASSBINDIR)/%.png: $(SHARE_SRC)/classes/%.png
	$(install-file)

$(CLASSBINDIR)/%.xml: $(SHARE_SRC)/classes/%.xml
	$(install-file)

$(CLASSBINDIR)/%.css: $(SHARE_SRC)/classes/%.css
	$(install-file)

$(CLASSBINDIR)/%.bdtd: $(SHARE_SRC)/classes/%.bdtd
	$(install-file)

$(CLASSBINDIR)/%.txt: $(SHARE_SRC)/classes/%.txt
	$(install-file)

