#
# @(#)Makefile	1.60 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR  = ../..
PACKAGE   = javax.swing
PRODUCT   = com
SWING_SRC = $(SHARE_SRC)/classes/javax/swing
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES.gmk
AUTO_FILES_JAVA_DIRS = javax/swing sun/swing com/sun/java/swing
AUTO_JAVA_PRUNE = plaf

#
# Rules
#

default_rule: all

html32dtd html32dtd.clean:
	@$(ECHO) ">>>Recursively making html32dtd "$@" @ `$(DATE)` ..."; \
	$(CD) html32dtd; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) $@\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making html32dtd "$@" @ `$(DATE)`." ; \

plaf:
	@$(ECHO) ">>>Recursively making plaf all @ `$(DATE)` ..."; \
	$(CD) plaf; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) all\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making plaf all @ `$(DATE)`." ; \

plaf.clean:
	@$(ECHO) ">>>Recursively making plaf "$@" @ `$(DATE)` ..."; \
	$(CD) plaf; $(MAKE) RELEASE=$(RELEASE) FULL_VERSION=$(FULL_VERSION) clobber\
        || exit 1; $(CD) ..; \
	$(ECHO) "<<<Finished Recursively making plaf "$@" @ `$(DATE)`." ; \

.PHONY: html32dtd html32dtd.clean plaf plaf.clean

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

build: other_files plaf html32dtd

clean clobber:: classes.clean plaf.clean html32dtd.clean

other_files: $(MISC_FILES)

$(CLASSBINDIR)/%.wav: $(SHARE_SRC)/classes/%.wav
	$(install-file)

$(CLASSBINDIR)/%.gif: $(SHARE_SRC)/classes/%.gif
	$(install-file)

$(CLASSBINDIR)/%.css: $(SHARE_SRC)/classes/%.css
	$(install-file)

$(CLASSBINDIR)/%.bdtd: $(SHARE_SRC)/classes/%.bdtd
	$(install-file)

$(CLASSBINDIR)/%.txt: $(SHARE_SRC)/classes/%.txt
	$(install-file)

