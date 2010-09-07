#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.39 10/03/24
#
# Makefile to build the Java Plug-in components
#

DEPLOY_TOPDIR = ../../..

ifeq ($(VARIANT), DBG)
DEBUG_CLASSFILES = true
endif

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

IMVERSIONVALUE=$(JDK_MINOR_VERSION).$(JDK_MICRO_VERSION).$(J2SE_UPDATE_VER).$(COOKED_BUILD_NUMBER)
SED_ALL_MANIFEST=$(SED) -e 's%IMVERSION%$(IMVERSIONVALUE)%g'

ifeq ($(PLATFORM), windows)
PLATFORM_DIR = win32
else
PLATFORM_DIR = solaris
endif

PROGRAM_NAME = HtmlConverter

CLASSDESTDIR = 		$(TEMPDIR)/$(PROGRAM_NAME)/classes
OBJDIR = 		$(TEMPDIR)/obj

CONVERTER_SRC =		$(DEPLOY_TOPDIR)/src/plugin
LAUNCHER_SRC =		$(DEPLOY_TOPDIR)/src/plugin/win32/converter
LAUNCHER_EXE =		$(OBJDIR)/$(PROGRAM_NAME)$(EXE_SUFFIX)

CONVERTER_JAR = 	$(LIBDIR)/htmlconverter.jar
CONVERTER_PLATFORM_SRC= $(CONVERTER_SRC)/$(PLATFORM_DIR)/converter


ifeq ($(VARIANT), DBG)
CFLAGS = /c /nologo /Z7
else
CFLAGS = /c /nologo /W3 $(GX_OPTION) /O2     
endif

LFLAGS = /PDB:NONE

ifeq ($(ARCH), amd64)
  OTHER_LDLIBS = user32.lib bufferoverflowu.lib
else
  OTHER_LDLIBS = user32.lib
endif

#
# Rules
#

JAVAC = $(BINDIR)/javac
JAR   = $(BINDIR)/jar
include converter.jmk
include converter.gmk
include $(J2SE_TOPDIR)/make/common/Classes.gmk

# deal zh_HK resource
include $(DEPLOY_TOPDIR)/make/common/Resources.gmk

#
# Extra rules
#
build: .library.install 

ifeq ($(PLATFORM), windows)
build: .launcher.install
else
build: .scripts.install
endif

clobber::
	$(RM) -r $(CLASSDESTDIR)
	$(RM) -r $(OBJDIR)
	$(RM) $(CONVERTER_JAR)
	$(RM) $(LAUNCHER_EXE)
	$(RM) $(BINDIR)/$(PROGRAM_NAME)$(EXE_SUFFIX)

#
# Rules to install extension library. Clean tmp gened java files after this install.
#

.library.install: $(CONVERTER_JAR)
	$(RM) $(DEPLOY_L10N_TMP_OUTPUTDIR)/sun/plugin/converter/resources/Converter*.java

$(CONVERTER_JAR): $(CLASSDESTDIR) $(FILES_class)
	@$(RM) $(@)
	$(JAR) -cfm $(@) $(CONVERTER_SRC)/share/converter/$(MANIFEST)  \
	               -C $(CLASSDESTDIR) HTMLConverter.class \
	               -C $(CLASSDESTDIR) sun
	for i in $(FILES_other); do \
	    $(JAR) -uf  $(@) -C $(CONVERTER_SRC)/share/converter $${i}; \
	done

#
# Rules to build and install the win32 launcher.
#

.launcher.install: $(LAUNCHER_EXE)

$(OBJDIR)/$(PROGRAM_NAME).res: $(LAUNCHER_SRC)/$(PROGRAM_NAME).rc
	@$(prep-target)
	$(RC) $(RC_FLAGS) -i $(OBJDIR) $(CC_OBJECT_OUTPUT_FLAG)$(@) $(LAUNCHER_SRC)/$(PROGRAM_NAME).rc

updatefiles:
	$(MKDIR) -p $(OBJDIR)
	$(CAT) $(LAUNCHER_SRC)/htmlconverter_proto.exe.manifest | $(SED_ALL_MANIFEST) > $(OBJDIR)/htmlconverter.exe.manifest

$(OBJDIR)/$(PROGRAM_NAME).$(OBJECT_SUFFIX): updatefiles $(LAUNCHER_SRC)/$(PROGRAM_NAME).cpp
	@$(prep-target)
	$(CC) $(CFLAGS) $(LAUNCHER_SRC)/$(PROGRAM_NAME).cpp $(CC_OBJECT_OUTPUT_FLAG)$(@)

$(LAUNCHER_EXE): $(OBJDIR)/$(PROGRAM_NAME).$(OBJECT_SUFFIX) \
		 $(OBJDIR)/$(PROGRAM_NAME).res
	@$(prep-target)
	$(LINK) $(LFLAGS) $? $(OTHER_LDLIBS) /OUT:$(LAUNCHER_EXE:$(OBJDIR)/%=$(BINDIR)/%)
	@$(CHMOD) 755 $(LAUNCHER_EXE:$(OBJDIR)/%=$(BINDIR)/%)

#
# Rules to install scripts into the SDK bin directory.
#

.scripts.install: $(SCRIPTS)

$(SCRIPTS): $(SCRIPTS:$(BINDIR)/%=$(CONVERTER_PLATFORM_SRC)/%)
	$(install-file)
	@$(CHMOD) 755 $(@)

# call target to replace resources java files' copyright year and redirect java srcs to classes.list
define deal_copyrightyear_classeslist
	targetname=$(@:$(CLASSDESTDIR)%.class=$(DEPLOY_L10N_TMP_OUTPUTDIR)%.java);\
	$(MAKE) $${targetname};\
	$(ECHO) $${targetname} >> $(TEMPDIR)/.classes.list
endef

#
# Rules to compile
#
$(CLASSDESTDIR)/sun/plugin/converter/resources/Converter_%.class:
	$(deal_copyrightyear_classeslist)

$(CLASSDESTDIR)/sun/plugin/converter/resources/Converter.class:
	$(deal_copyrightyear_classeslist)

$(CLASSDESTDIR)/%.class: $(CONVERTER_SRC)/share/converter/%.java
	@$(ECHO) $? >> $(TEMPDIR)/.classes.list

# Redirect zh_HK java files to tmp directory which created from zh_TW
$(CLASSDESTDIR)/%_zh_HK.class: $(DEPLOY_L10N_TMP_OUTPUTDIR)/%_zh_HK.java
	@$(ECHO) $? >> $(TEMPDIR)/.classes.list

#
# Rules to create a source bundle.
#
source-bundle::
