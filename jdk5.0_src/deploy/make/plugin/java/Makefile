#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.25 04/01/15
#
# Makefile to build the Java Plug-in components
#

PLUGIN_TOP = ../../..

include $(PLUGIN_TOP)/make/plugin/common/config.gmk

include ./FileList.gmk

# for DEPLOYCLASSDESTDIR
include ../../common/DeployRules.gmk

VPATH.java = $(PLUGIN_PLATFORM_SRC)/classes$(CLASSPATH_SEPARATOR)$(JAVASRCDIR)
JAVAC		= $(BINDIR)/javac
JAR		= $(BINDIR)/jar

ifeq ($(VARIANT),DBG)
  JAVAC += -g -deprecation
  JAVAC_CP = "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSDIR)$(CLASSPATH_SEPARATOR)$(DEPLOYDBGCLASSDESTDIR)"
else
  JAVAC += -deprecation -source 1.4
  JAVAC_CP	= "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSDIR)$(CLASSPATH_SEPARATOR)$(DEPLOYCLASSDESTDIR)"
endif

CLASSDESTDIR = $(CLASSDIR)
$(OBJDIR)/.class.headers.$(ARCH): CLASSDESTDIR := "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSDIR)$(CLASSPATH_SEPARATOR)$(DEPLOYCLASSDESTDIR)"

all: optimized debug

include $(J2SE_TOPDIR)/make/common/Rules.gmk

DEBUG = $(PLUGIN_JARFILE_g)
OPTIMIZED = $(PLUGIN_JARFILE)

$(CLASSDESTDIR)/%.class: $(PLUGIN_PLATFORM_SRC)/classes/%.java
	@$(RM) $@
	@$(ECHO) $? >> $(TMPDIR)/.classes.list

$(CLASSDESTDIR)/%.class: $(JAVASRCDIR)/%.java
	@$(RM) $@
	@$(ECHO) $? >> $(TMPDIR)/.classes.list

# Redirect zh_HK java files to tmp directory which created from zh_TW
$(CLASSDESTDIR)/%_zh_HK.class: $(DEPLOY_L10N_TMP_OUTPUTDIR)/%_zh_HK.java
	@$(RM) $@
	@$(ECHO) $< >> $(TMPDIR)/.classes.list

ifeq ($(VARIANT), DBG)
$(PLUGIN_JARFILE): classes
else
$(PLUGIN_JARFILE): classheaders
endif
	@if [ -s $(TMPDIR)/.classes.list ] ; \
        then \
		$(ECHO) Rebuilding: $(PLUGIN_JARFILE) ;\
		$(CP) -f $(JAVASRCDIR)/sun/plugin/util/*.gif $(CLASSDIR)/sun/plugin/util; \
		$(CP) -f $(JAVASRCDIR)/sun/plugin/util/*.png $(CLASSDIR)/sun/plugin/util; \
		$(MKDIR) -p $(dir $(PLUGIN_JARFILE)); \
		cd $(CLASSDIR); $(JAR) cf0 $(PLUGIN_JARFILE) *; \
        fi

clobber:: 
	$(RM) -r $(CLASSDIR) $(CLASSDIR_g)
	$(RM) $(PLUGIN_JARFILE) $(PLUGIN_JARFILE_g)

#deal zh_HK resource
include $(PLUGIN_TOP)/make/common/Resources.gmk

