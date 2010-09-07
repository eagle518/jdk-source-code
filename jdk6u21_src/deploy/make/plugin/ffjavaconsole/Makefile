#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.5 10/03/24
#
# Makefile to build Firefox extension for Java console 
#


DEPLOY_TOPDIR = ../../..
DEPLOY_BUILDDIR = $(DEPLOY_TOPDIR)/make

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk
include $(DEPLOY_BUILDDIR)/common/Defs.gmk

CLSID	:= $(shell printf "{CAFEEFAC-%04d-%04d-%04d-ABCDEFFEDCBA}" $(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION) $(JDK_MICRO_VERSION) $(JDK_UPDATE_VERSION))
XPIFILE	= $(LIBDIR)/deploy/ffjcext.zip
XPIDESTDIR	= $(TEMPDIR)/ffjavaconsole
XPISRCDIR	= $(DEPLOY_TOPDIR)/src/plugin/share/ffjavaconsole

CONVERT_JDK = $(SED) -e "s/_JDK_VERSION/$(JDK_VERSION)/g" | $(SED) -e "s/_JDK_UNDERSCORE_VERSION/$(JDK_UNDERSCORE_VERSION)/g"
CONVERT_MIMETYPE = $(SED) -e "s/_PLUGIN_MAJOR_MIMETYPE/$(JDK_VERSION)/g"
CONVERT_CLSID = $(SED) -e "s/_PLUGIN_CLSID/$(CLSID)/g"

ifdef JDK_UPDATE_VERSION
  CONVERT_EM  = $(SED) -e "s/_JC_EMVER/$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(JDK_UPDATE_VERSION)/g" 
else
  CONVERT_EM  = $(SED) -e "s/_JC_EMVER/$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION)/g"
  ifeq (0, $(JDK_MICRO_VERSION))
    CONVERT_MIMETYPE = $(SED) -e "s/_PLUGIN_MAJOR_MIMETYPE/$(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION)/g"
  endif
endif

CONVERT_XPI = $(CONVERT_JDK) | $(CONVERT_EM) | $(CONVERT_MIMETYPE) | $(CONVERT_CLSID)


dummy := $(shell $(MKDIR) -p $(XPIDESTDIR)/$(CLSID) $(LIBDIR)/deploy)

all: build

build: $(XPIFILE)

$(XPIFILE): FRC
	$(ECHO) Building java console xpi file 
	cd $(XPISRCDIR); $(FIND) . -name SCCS -prune -o -print | $(CPIO) -pdum $(XPIDESTDIR)/$(CLSID)
	for i in `$(FIND) $(XPIDESTDIR)/$(CLSID) -type f` ; do \
	  $(ECHO) converting $$i; \
	  $(CAT) $$i | $(CONVERT_XPI) > $(XPIDESTDIR)/.tmpxpi; \
	  $(MV) -f $(XPIDESTDIR)/.tmpxpi $$i; \
	done
	@#
	@# zip -0 only store files in zip bundles so RegJavaConsole.cpp need not do with 
	@# decompression, it simplifies the implementation of RegJavaConsole.cpp
	@# 
	cd $(XPIDESTDIR); $(ZIPEXE) -0 -r $(XPIFILE) *

clobber:: 
	$(RM) -r $(XPIDESTDIR)
	$(RM) $(XPIFILE)

FRC:
