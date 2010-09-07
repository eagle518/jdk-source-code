#
# @(#)Makefile	1.27 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..

PACKAGE = java.text
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

#
# Rules to add data files BreakIterator
#
GENBIDSRC = $(BUILDDIR)/tools/GenerateBreakIteratorData
GENBIDOUT = $(TEMPDIR)/tools/GenerateBreakIteratorData

# input
#
#   Notes: sun.text.resources.BreakIteratorRules no longer goes to runtime.
#     They are used at J2SE build phase in order to create $(BIFILES) which
#     are used on runtime instead.
#
BIRULES = $(SHARE_SRC)/classes/sun/text/resources/BreakIteratorRules.java
BIRULES_CLASS = $(GENBIDOUT)/sun/text/resources/BreakIteratorRules.class
BIINFO = $(SHARE_SRC)/classes/sun/text/resources/BreakIteratorInfo.java
BIINFO_CLASS = $(GENBIDOUT)/sun/text/resources/BreakIteratorInfo.class
UNICODE = $(GENBIDSRC)/../GenerateCharacter/UnicodeData.txt

# output
BIFILES = $(CLASSBINDIR)/sun/text/resources/CharacterBreakIteratorData \
          $(CLASSBINDIR)/sun/text/resources/WordBreakIteratorData \
          $(CLASSBINDIR)/sun/text/resources/LineBreakIteratorData \
          $(CLASSBINDIR)/sun/text/resources/SentenceBreakIteratorData

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

$(BUILDER_CLASS): $(BUILDER)
	$(prep-target)
	$(JAVAC_BOOT) -d $(GENBIDOUT) -sourcepath $(GENBIDSRC) $(BUILDER)
	@$(java-vm-cleanup)

$(BIRULES_CLASS): $(BIRULES)
	$(prep-target)
	$(JAVAC_BOOT) -d $(GENBIDOUT) \
		-sourcepath $(SHARE_SRC)/classes/sun/text/resources $(BIRULES)
	@$(java-vm-cleanup)

$(BIINFO_CLASS): $(BIINFO)
	$(prep-target)
	$(JAVAC_BOOT) -d $(GENBIDOUT) \
		-sourcepath $(SHARE_SRC)/classes/sun/text/resources $(BIINFO)
	@$(java-vm-cleanup)

$(BIFILES): $(BUILDER_CLASS) $(BIRULES_CLASS) $(BIINFO_CLASS) $(UNICODE)
	$(prep-target)
	$(MKDIR) -p  $(CLASSBINDIR)/sun/text/resources
	$(JAVA) -Xbootclasspath/p:$(GENBIDOUT) GenerateBreakIteratorData \
		-o $(CLASSBINDIR)/sun/text/resources -spec $(UNICODE)
	@$(java-vm-cleanup)

bifiles: $(BIFILES) $(CLASSBINDIR)/sun/text/resources/unorm.icu $(CLASSBINDIR)/sun/text/resources/uprops.icu

build: bifiles

clean::
	$(RM) -r $(GENBIDOUT) $(BIFILES)

#
# Extra rules to copy unorm.icu and uprops.icu
#
$(CLASSBINDIR)/sun/text/resources/unorm.icu : $(SHARE_SRC)/classes/sun/text/resources/unorm.icu
	$(install-file)

$(CLASSBINDIR)/sun/text/resources/uprops.icu : $(SHARE_SRC)/classes/sun/text/resources/uprops.icu
	$(install-file)

clean clobber::
	$(RM) -f $(CLASSBINDIR)/sun/text/resources/*.icu

