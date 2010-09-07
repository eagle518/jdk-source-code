#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.19 06/04/25
#
#
# Makefile to build control panel subdirectories
#

DEPLOY_TOPDIR = ../../../..
PACKAGE = ControlPanel

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

DESKTOPDIR = $(OUTPUTDIR)/plugin/desktop
CPANELSRCDIR = $(DEPLOY_TOPDIR)/src/plugin/solaris/controlpanel
SCRIPTFILE = $(OUTPUTDIR)/bin/jcontrol
PNGFILE = $(DESKTOPDIR)/sun_java.png
DESKTOPFILE = $(DESKTOPDIR)/sun_java.desktop

#
# Might consider removing the "non-NEW_" entries above and replacing them
# with these.  However, due to the remote possibility of a stray dependency
# on the old location, they are retained.
#
NEW_DESKTOPDIR = $(OUTPUTDIR)/lib/desktop/applications
NEW_DESKTOPFILE = $(NEW_DESKTOPDIR)/sun_java.desktop

PVER="PLUGIN_VERSION=$(JDK_VERSION)"
PNDVER="PLUGIN_NODOTVERSION=$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(UVSUFFIX)"
REPLACEMENT=$(PVER)

ifndef JDK_UPDATE_VERSION
	ifeq (0, $(JDK_MICRO_VERSION))
		REPLACEMENT = $(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION)
	endif
endif

all: $(OBJDIR) $(DESKTOPDIR) $(NEW_DESKTOPDIR) $(SCRIPTFILE) $(HTMLFILE) $(PNGFILE) $(DESKTOPFILE) $(NEW_DESKTOPFILE)

$(OBJDIR) $(DESKTOPDIR) $(NEW_DESKTOPDIR):
	$(MKDIR) -p $@

$(SCRIPTFILE): $(CPANELSRCDIR)/jcontrol
	printf "$(PVER) \n$(PNDVER)\n" > $(OBJDIR)/vertmp
	$(SED) -e '/REPLACEME/r $(OBJDIR)/vertmp' -e '/REPLACEME/d' < $< > $@
	$(CHMOD) a+x $@

$(PNGFILE): $(CPANELSRCDIR)/sun_java.png
	$(CP) -f $< $@

$(DESKTOPFILE) $(NEW_DESKTOPFILE): $(CPANELSRCDIR)/sun_java.desktop
ifeq ($(PLATFORM), linux)
	$(SED) -e 's/^Categories=.*/&X-Red-Hat-Base;X-Ximian-Settings;/' < $< > $@ 
else
	$(CP) -f $< $@
endif

clobber:: 
	$(RM) -f $(SCRIPTFILE) $(HTMLFILE) $(PNGFILE) $(DESKTOPFILE) \
	    $(NEW_DESKTOPFILE)
	$(RM) -rf $(OBJDIR)
	$(RM) -rf $(DESKTOPDIR)

#
# Phonies to avoid accidents.
#
.PHONY: all build clobber  
