#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.8 03/12/19
#
#
# Makefile to build control panel subdirectories
#

PLUGIN_TOP = ../../..
PACKAGE = l10n
RESOURCESRCDIR= $(PLUGIN_TOP)/src/plugin/solaris/resources

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

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
.PHONY: all optimized debug clobber  
