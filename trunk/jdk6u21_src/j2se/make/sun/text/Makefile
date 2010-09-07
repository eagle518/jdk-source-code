#
# @(#)Makefile	1.19 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for locale data.
#

BUILDDIR = ../..

PACKAGE = sun.text
PRODUCT = sun

CLASSDESTDIR = $(TEMPDIR)/classes
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk
include FILES_properties.gmk

#
# Compile the properties files
#
COMPILED_PROPERTIES_SUPERCLASS=LocaleNamesBundle

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Rules to add data files BreakIterator (for th locale)
#
GENBIDSRC = $(BUILDDIR)/tools/GenerateBreakIteratorData
GENBIDOUT = $(TEMPDIR)/../java.text/tools/GenerateBreakIteratorData

# input
#
#   Notes: sun.text.resources.BreakIteratorRules_th no longer goes to runtime.
#     They are used at J2SE build phase in order to create $(BIFILES) which
#     are used on runtime instead.
#
BIRULES = $(SHARE_SRC)/classes/sun/text/resources/BreakIteratorRules.java \
          $(SHARE_SRC)/classes/sun/text/resources/BreakIteratorRules_th.java
BIRULES_CLASS = $(GENBIDOUT)/sun/text/resources/BreakIteratorRules.class \
                $(GENBIDOUT)/sun/text/resources/BreakIteratorRules_th.class
BIINFO = $(SHARE_SRC)/classes/sun/text/resources/BreakIteratorInfo_th.java
BIINFO_CLASS = $(GENBIDOUT)/sun/text/resources/BreakIteratorInfo_th.class
UNICODE = $(GENBIDSRC)/../GenerateCharacter/UnicodeData.txt

# output
BIFILES = $(CLASSDESTDIR)/sun/text/resources/WordBreakIteratorData_th \
          $(CLASSDESTDIR)/sun/text/resources/LineBreakIteratorData_th

# builder
BUILDER = $(GENBIDSRC)/GenerateBreakIteratorData.java \
          $(GENBIDSRC)/CharSet.java \
          $(GENBIDSRC)/CharacterCategory.java \
          $(GENBIDSRC)/DictionaryBasedBreakIteratorBuilder.java \
          $(GENBIDSRC)/RuleBasedBreakIteratorBuilder.java \
          $(GENBIDSRC)/SupplementaryCharacterData.java
BUILDER_CLASS = $(GENBIDOUT)/GenerateBreakIteratorData.class \
                $(GENBIDOUT)/CharSet.class \
                $(GENBIDOUT)/CharacterCategory.class \
                $(GENBIDOUT)/DictionaryBasedBreakIteratorBuilder.class \
                $(GENBIDOUT)/RuleBasedBreakIteratorBuilder.class \
                $(GENBIDOUT)/SupplementaryCharacterData.class

$(BUILDER_CLASS): $(GENBIDOUT) $(BUILDER)
	$(JAVAC_BOOT) -d $(GENBIDOUT) -sourcepath $(GENBIDSRC) $(BUILDER)
	@$(java-vm-cleanup)

$(GENBIDOUT):
	$(MKDIR) -p $(GENBIDOUT)

$(BIRULES_CLASS): $(GENBIDOUT)/sun/text/resources $(BIRULES)
	$(RM) $@
	$(JAVAC_BOOT) -d $(GENBIDOUT) \
		-sourcepath $(SHARE_SRC)/classes/sun/text/resources $(BIRULES)
	@$(java-vm-cleanup)

$(BIINFO_CLASS): $(GENBIDOUT)/sun/text/resources $(BIINFO)
	$(RM) $@
	$(JAVAC_BOOT) -d $(GENBIDOUT) \
		-sourcepath $(SHARE_SRC)/classes/sun/text/resources $(BIINFO)
	@$(java-vm-cleanup)

$(GENBIDOUT)/sun/text/resources:
	$(MKDIR) -p $(GENBIDOUT)/sun/text/resources

$(BIFILES): $(BUILDER_CLASS) $(BIRULES_CLASS) $(BIINFO_CLASS) $(UNICODE)
	$(RM) $@
	$(JAVA) -Xbootclasspath/p:$(GENBIDOUT) GenerateBreakIteratorData \
		-o $(CLASSDESTDIR)/sun/text/resources -spec $(UNICODE) \
		-language th
	@$(java-vm-cleanup)

clean::
	$(RM) -r $(GENBIDOUT) $(BIFILES)
#
# End of rules to add data files for BreakIterator
#

#
# Extra rules to build locale data.
#
LOCALEDATA_JAR = $(EXTDIR)/localedata.jar

SPECIALFILES = $(CLASSDESTDIR)/sun/text/resources/thai_dict

$(CLASSDESTDIR)/sun/text/resources/% : $(SHARE_SRC)/classes/sun/text/resources/%
	$(install-file)

$(LOCALEDATA_JAR): $(EXTDIR) $(FILES_class) $(BIFILES) $(SPECIALFILES)
	$(JAR) -cf $@ -C $(CLASSDESTDIR) sun \
	     $(JAR_JFLAGS)
	@$(java-vm-cleanup)

build: $(LOCALEDATA_JAR)

clean clobber::
	$(RM) -r $(CLASSDESTDIR) 
	$(RM) $(LOCALEDATA_JAR)

