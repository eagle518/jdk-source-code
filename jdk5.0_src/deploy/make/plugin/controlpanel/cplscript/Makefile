#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.12 03/12/19
#
#
# Makefile to build control panel subdirectories
#

PLUGIN_TOP = ../../../..
PACKAGE = ControlPanel

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

DESKTOPDIR = $(OUTPUTDIR)/plugin/desktop
CPANELSRCDIR = $(PLUGIN_TOP)/src/plugin/solaris/controlpanel
SCRIPTFILE = $(OUTPUTDIR)/bin/ControlPanel
PNGFILE = $(DESKTOPDIR)/sun_java.png
DESKTOPFILE = $(DESKTOPDIR)/sun_java.desktop

PVER="PLUGIN_VERSION=$(JDK_VERSION)"
PNDVER="PLUGIN_NODOTVERSION=$(JDK_MAJOR_VERSION)$(JDK_MINOR_VERSION)$(JDK_MICRO_VERSION)$(UVSUFFIX)"
REPLACEMENT=$(PVER)

ifndef JDK_UPDATE_VERSION
	ifeq (0, $(JDK_MICRO_VERSION))
		REPLACEMENT = $(JDK_MAJOR_VERSION).$(JDK_MINOR_VERSION)
	endif
endif

all: $(OBJDIR) $(DESKTOPDIR) $(SCRIPTFILE) $(HTMLFILE) $(PNGFILE) $(DESKTOPFILE)

$(OBJDIR) $(DESKTOPDIR):
	$(MKDIR) -p $@

$(SCRIPTFILE): $(CPANELSRCDIR)/ControlPanel
	printf "$(PVER) \n$(PNDVER)\n" > $(OBJDIR)/vertmp
	$(SED) -e '/REPLACEME/r $(OBJDIR)/vertmp' -e '/REPLACEME/d' < $< > $@
	$(CHMOD) a+x $@

$(PNGFILE): $(CPANELSRCDIR)/sun_java.png
	$(CP) $< $@

$(DESKTOPFILE): $(CPANELSRCDIR)/sun_java.desktop
	$(CP) $< $@

clobber:: 
	$(RM) $(SCRIPTFILE) $(HTMLFILE) $(PNGFILE) $(DESKTOPFILE)
	$(RM) -rf $(OBJDIR)
	$(RM) -rf $(DESKTOPDIR)
#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber  
