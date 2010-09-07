#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.13 10/03/24
#
#
# Makefile to build control panel subdirectories
#

DEPLOY_TOPDIR = ../../..
PACKAGE = l10n
RESOURCESRCDIR= $(DEPLOY_TOPDIR)/src/plugin/solaris/resources

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

include ./FileList.gmk

all: $(L10N_MESSAGES)

$(OUTPUTDIR)/lib/locale/%/LC_MESSAGES/sunw_java_plugin.mo : $(RESOURCESRCDIR)/%/sunw_java_plugin.po
	@$(MKDIR) -p $(@D)
	msgfmt -o $(@) $<

#deal HK locale special
$(OUTPUTDIR)/lib/locale/zh_HK.BIG5HK/LC_MESSAGES/sunw_java_plugin.mo : $(RESOURCESRCDIR)/zh_TW.BIG5/sunw_java_plugin.po
	@$(MKDIR) -p $(@D)
	msgfmt -o $(@) $<

clobber:: 
	$(RM) -f $(L10N_MESSAGES)
#
# Phonies to avoid accidents.
#
.PHONY: all build clobber  
