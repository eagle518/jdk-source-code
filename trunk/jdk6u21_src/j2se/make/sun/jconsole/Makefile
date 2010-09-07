#
# @(#)Makefile	1.17 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jconsole
#

BUILDDIR = ../..
PACKAGE = sun.tools.jconsole
PRODUCT = sun
PROGRAM = jconsole
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

JARFILE = $(LIBDIR)/jconsole.jar

OTHER_JAVACFLAGS += -encoding iso8859-1

#Override the default APP_CLASSPATH to pick up jconsole.jar also.
#Default is defined in src/[solaris,windows]/bin/java_md.h
#PROGRAM, JAVA_ARGS, and APP_CLASSPATH are used in src/share/bin/java.c
JCONSOLE_CLASSPATH = "{ \"/lib/jconsole.jar\", \"/lib/tools.jar\", \"/classes\" }"
OTHER_CPPFLAGS += -DAPP_CLASSPATH=$(JCONSOLE_CLASSPATH)

JAVA_ARGS = "{ \"sun.tools.jconsole.JConsole\" }"

ifeq ($(PLATFORM), windows)
  OTHER_CPPFLAGS += -DJAVAW
  LDLIBS_COMMON  += user32.lib
  JAVA_ARGS = "{ \"-J-Djconsole.showOutputViewer\", \"sun.tools.jconsole.JConsole\" }"
endif

#
# Files to compile.
#
include FILES.gmk

AUTO_FILES_JAVA_DIRS =  sun/tools/jconsole \
			com/sun/tools/jconsole

FILES_png = $(PNG_FILES:%.png=$(CLASSBINDIR)/%.png)
FILES_gif = $(GIF_FILES:%.gif=$(CLASSBINDIR)/%.gif)

build: $(GENSRCDIR)/sun/tools/jconsole/Version.java

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_JAVA = $(TARGDIR)$(PKGDIR)/resources/JConsoleResources.java
include $(BUILDDIR)/common/Resources.gmk

build: $(FILES_png) $(FILES_gif) $(TEMPDIR)/manifest $(JARFILE)

$(GENSRCDIR)/sun/tools/jconsole/Version.java: \
                $(SHARE_SRC)/classes/sun/tools/jconsole/Version-template.java
	$(MKDIR) -p $(@D)
	$(SED) -e 's/@@jconsole_version@@/$(FULL_VERSION)/g' $< > $@

$(CLASSBINDIR)/$(PKGDIR)/resources/%: $(SHARE_SRC)/classes/$(PKGDIR)/resources/%
	@$(MKDIR) -p $(@D)
	$(install-file)

$(TEMPDIR)/manifest: $(SHARE_SRC)/classes/$(PKGDIR)/manifest
	$(install-file)

#
# Extra rule to build jconsole.jar
#

$(JARFILE): $(LIBDIR) $(FILES_class) $(FILES_png) $(FILES_gif) $(TEMPDIR)/manifest
	$(JAR) -cfm $(JARFILE) $(TEMPDIR)/manifest \
               -C $(CLASSBINDIR) sun/tools/jconsole \
               -C $(CLASSBINDIR) com/sun/tools/jconsole \
		    $(JAR_JFLAGS)
	@$(java-vm-cleanup)

clean clobber::
	$(RM) $(TEMPDIR)/manifest $(JARFILE)

