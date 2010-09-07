#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.10 05/11/17
#
#
# Makefile to build JQS (Java Quick Starter) and associated
# binaries in each subdir.
#

DEPLOY_TOPDIR = ../..

PRODUCT = deploy
PROGRAM = jqs

include $(DEPLOY_TOPDIR)/make/common/Defs.gmk

OBJDIR = $(OUTPUTDIR)/tmp/deploy/$(PROGRAM)

SUBDIRS = jqs jqsnotify jqs_plugin


#
# JQS files for the IE/Firefox plugins, the config file, and the messages file.
# These will be copied into the JRE image.
#

JQS_HOME = $(DEPLOY_TOPDIR)/src/jqs

JQS_DESTDIR = $(OUTPUTDIR)/lib/deploy/jqs

JQS_IE_FILES = \
	$(JQS_DESTDIR)/ie/jqs_plugin.dll \
	$(JQS_DESTDIR)/ie/jqs_plugin.map \
	$(JQS_DESTDIR)/ie/jqs_plugin.pdb

JQS_FF_FILES = \
	$(JQS_DESTDIR)/ff/install.rdf \
	$(JQS_DESTDIR)/ff/chrome.manifest \
	$(JQS_DESTDIR)/ff/chrome/content/overlay.js \
	$(JQS_DESTDIR)/ff/chrome/content/overlay.xul

JQS_CONFIG_FILE = \
	$(JQS_DESTDIR)/jqs.conf

JQS_MESSAGES_FILE = \
	$(JQS_DESTDIR)/jqsmessages.properties


#
# Make Targets
#

all:: $(SUBDIRS) $(JQS_IE_FILES) $(JQS_FF_FILES) $(JQS_CONFIG_FILE) $(JQS_MESSAGES_FILE)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

$(JQS_IE_FILES):
	$(MKDIR) -p $(@D)
	$(MV) -f $(subst $(JQS_DESTDIR)/ie, $(OUTPUTDIR)/bin, $@) $@

$(JQS_FF_FILES) $(JQS_CONFIG_FILE) $(JQS_MESSAGES_FILE):
	$(MKDIR) -p $(@D)
	$(CP) -f $(subst $(JQS_DESTDIR), $(JQS_HOME)/windows, $@) $@

clobber::
	for i in $(SUBDIRS); do \
	  $(MAKE) -C $$i $@; \
	done
	$(RM) -r $(JQS_DESTDIR)
	$(RM) -r $(OBJDIR)
	$(RM) -r $(OUTPUTDIR)/bin/jqs*


#
# Phonies to avoid accidents.
#
.PHONY: all build clobber $(SUBDIRS)
