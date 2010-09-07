#
# @(#)Makefile	1.39 10/04/27
#
# Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# Makefile to build the Java Plug-in classes, native headers, and JAR file.
#

DEPLOY_TOPDIR = ../../..

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk
include $(J2SE_TOPDIR)/make/common/shared/Defs-javadoc.gmk

include ./FileList.gmk

# for DEPLOYCLASSDESTDIR and JAVAWSCLASSDESTDIR
include ../../common/DeployRules.gmk

VPATH.java = $(PLUGIN_PLATFORM_SRC)/classes$(CLASSPATH_SEPARATOR)$(JAVASRCDIR)
JAVAC      = $(BINDIR)/javac
JAR        = $(BINDIR)/jar

ifeq ($(VARIANT),DBG)
  JAVAC += -g -deprecation
else
  JAVAC += -deprecation -source 1.4
endif
JAVAC_CP	= "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSDIR)$(CLASSPATH_SEPARATOR)$(DEPLOYCLASSDESTDIR)$(CLASSPATH_SEPARATOR)$(JAVAWSCLASSDESTDIR)"

CLASSDESTDIR = $(CLASSDIR)
$(OBJDIR)/.class.headers.$(ARCH): CLASSDESTDIR := "$(CLASSBINDIR)$(CLASSPATH_SEPARATOR)$(CLASSDIR)$(CLASSPATH_SEPARATOR)$(DEPLOYCLASSDESTDIR)"

all: build

include $(J2SE_TOPDIR)/make/common/Rules.gmk

build: $(PLUGIN_JARFILE)

$(CLASSDESTDIR)/%.class: $(PLUGIN_PLATFORM_SRC)/classes/%.java
	@$(RM) $@
	@$(ECHO) $? >> $(TEMPDIR)/.classes.list

$(CLASSDESTDIR)/%.class: $(JAVASRCDIR)/%.java
	@$(RM) $@
	@$(ECHO) $? >> $(TEMPDIR)/.classes.list

# Redirect zh_HK java files to tmp directory which created from zh_TW
$(CLASSDESTDIR)/%_zh_HK.class: $(DEPLOY_L10N_TMP_OUTPUTDIR)/%_zh_HK.java
	@$(RM) $@
	@$(ECHO) $< >> $(TEMPDIR)/.classes.list

$(PLUGIN_JARFILE): classes
	@if [ -s $(TEMPDIR)/.classes.list ] ; \
        then \
		$(ECHO) Rebuilding: $(PLUGIN_JARFILE) ;\
		$(CP) -f $(JAVASRCDIR)/sun/plugin/util/*.gif $(CLASSDIR)/sun/plugin/util; \
		$(CP) -f $(JAVASRCDIR)/sun/plugin/util/*.png $(CLASSDIR)/sun/plugin/util; \
		$(MKDIR) -p $(dir $(PLUGIN_JARFILE)); \
		cd $(CLASSDIR); $(JAR) cf0 $(PLUGIN_JARFILE) *; \
        fi

# This jar file contains only the exported interfaces of the Java Plug-In
PLUGIN_EXPORTS_JARFILE = $(OUTPUTDIR)/tmp/deploy/plugin/plugin_exports.jar
.PHONY: plugin.exports.jar
plugin.exports.jar: classes
	$(ECHO) Rebuilding: $(PLUGIN_EXPORTS_JARFILE)
	$(MKDIR) -p $(dir $(PLUGIN_EXPORTS_JARFILE))
	cd $(CLASSDIR); $(JAR) cf0 $(PLUGIN_EXPORTS_JARFILE) com netscape/javascript

# Copyright line
COPYRIGHT_LINE = \
'<font size="-1">\
$(call CopyrightLine,,$(PLUGIN2_FIRST_COPYRIGHT_YEAR),)\
</font>'

PLUGIN2_JAVADOC_DIR = $(OUTPUTDIR)/tmp/deploy/plugin/plugin2-javadoc
.PHONY: plugin2.javadoc
plugin2.javadoc:
	$(RM) -r $(PLUGIN2_JAVADOC_DIR)
	$(MKDIR) -p $(PLUGIN2_JAVADOC_DIR)
	$(JAVADOC) -d $(PLUGIN2_JAVADOC_DIR) \
	           -bottom $(COPYRIGHT_LINE) \
                   -sourcepath $(JAVASRCDIR) \
		   -linkoffline http://java.sun.com/javase/6/docs/jre/api/plugin/dom/ 6-common-dom-packages \
		   -linkoffline http://java.sun.com/javase/6/docs/api/ 6-packages \
                   $(PLUGIN2_PKGS)

DOM_JAVADOC_DIR = $(OUTPUTDIR)/tmp/deploy/plugin/dom-javadoc
.PHONY: dom.javadoc
dom.javadoc:
	$(RM) -r $(DOM_JAVADOC_DIR)
	$(MKDIR) -p $(DOM_JAVADOC_DIR)
	$(JAVADOC) -d $(DOM_JAVADOC_DIR) \
	           -bottom $(COPYRIGHT_LINE) \
                   -sourcepath $(JAVASRCDIR) \
		   -linkoffline http://java.sun.com/javase/6/docs/jre/api/plugin/dom/ 6-common-dom-packages \
		   -linkoffline http://java.sun.com/javase/6/docs/api/ 6-packages \
                   $(DOM_PKGS)

JSOBJECT_JAVADOC_DIR = $(OUTPUTDIR)/tmp/deploy/plugin/jsobject-javadoc
.PHONY: jsobject.javadoc
jsobject.javadoc:
	$(RM) -r $(JSOBJECT_JAVADOC_DIR)
	$(MKDIR) -p $(JSOBJECT_JAVADOC_DIR)
	$(JAVADOC) -d $(JSOBJECT_JAVADOC_DIR) \
	           -bottom $(COPYRIGHT_LINE) \
                   $(JAVASRCDIR)/netscape/javascript/JSException.java \
                   $(JAVASRCDIR)/netscape/javascript/JSObject.java

LCBRIDGE_JAVADOC_DIR = $(OUTPUTDIR)/tmp/deploy/plugin/lcbridge-javadoc
.PHONY: lcbridge.javadoc
lcbridge.javadoc:
	$(RM) -r $(LCBRIDGE_JAVADOC_DIR)
	$(MKDIR) -p $(LCBRIDGE_JAVADOC_DIR)
	$(JAVADOC) -d $(LCBRIDGE_JAVADOC_DIR) \
	           -bottom $(COPYRIGHT_LINE) \
                   -sourcepath $(JAVASRCDIR) \
		   -linkoffline http://java.sun.com/javase/6/docs/api/ 6-packages \
                   $(LCBRIDGE_PKGS)

clean:: clobber

clobber:: 
	$(RM) -r $(CLASSDIR)
	$(RM) $(PLUGIN_JARFILE)

#deal zh_HK resource
include $(DEPLOY_TOPDIR)/make/common/Resources.gmk
