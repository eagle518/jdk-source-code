#
# Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.66 10/03/23
#
# Makefile for character converters.
#

BUILDDIR = ../..

PACKAGE = sun.nio
PRODUCT = sun

# This re-directs all the class files to a separate location
CLASSDESTDIR = $(TEMPDIR)/classes

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include FILES_java.gmk
AUTO_FILES_JAVA_DIRS = sun/nio/cs/ext

# Exclude a few sources on windows
ifeq ($(PLATFORM), windows)
  AUTO_JAVA_PRUNE = sun/nio/cs/ext/COMPOUND_TEXT.java         \
                    sun/nio/cs/ext/COMPOUND_TEXT_Decoder.java \
                    sun/nio/cs/ext/COMPOUND_TEXT_Encoder.java \
                    sun/nio/cs/ext/CompoundTextSupport.java
endif # PLATFORM

# For Cygwin, command line arguments that are paths must be converted to
# windows style paths. These paths cannot be used as targets, however, because 
# the ":" in them  will interfere with GNU Make rules, generating "multiple
# target pattern" errors.

# this define is for the rule:
CHARSETS_JAR = $(LIBDIR)/charsets.jar

# absolute paths
ABS_CHARSETS_JAR = $(ABS_LIBDIR)/charsets.jar

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

build: $(CHARSETS_JAR)

#
# Extra rules to build character converters.

SERVICE_DESCRIPTION = java.nio.charset.spi.CharsetProvider
SERVICE_DESCRIPTION_PATH = META-INF/services/$(SERVICE_DESCRIPTION)

GENCSDATASRC = $(BUILDDIR)/tools/CharsetData
GENCSDATATMP = $(TEMPDIR)/CharsetData
FILES_map = $(GENCSDATASRC)/sjis0213.map
FILES_dat = $(CLASSDESTDIR)/sun/nio/cs/ext/sjis0213.dat

$(FILES_dat): $(FILES_map)
	$(RM) -r $(GENCSDATATMP)
	$(MKDIR) -p $(GENCSDATATMP)
	$(JAVAC) -d $(GENCSDATATMP) \
		-sourcepath $(GENCSDATASRC) $(GENCSDATASRC)/GenCSData.java
	$(JAVA) -classpath $(GENCSDATATMP) GenCSData $(FILES_map) $(FILES_dat)

$(CLASSDESTDIR)/$(SERVICE_DESCRIPTION_PATH): \
  $(SHARE_SRC)/classes/sun/nio/cs/ext/$(SERVICE_DESCRIPTION_PATH)
	$(install-file)

$(CHARSETS_JAR): jfbcompile $(FILES_class) $(CLASSDESTDIR)/$(SERVICE_DESCRIPTION_PATH) \
	$(FILES_dat)
	$(CD) $(CLASSDESTDIR) ; \
	    $(ABS_JAR) cf $(ABS_CHARSETS_JAR) sun $(SERVICE_DESCRIPTION_PATH)  \
	    $(JAR_JFLAGS)
	@$(CD) $(CLASSDESTDIR) ; $(java-vm-cleanup)
	
jfbcompile:
ifeq ($(J4B),true)
	@$(ECHO) "Starting jfb specific compilation"
	$(JAVAC_CMD) ../../../src/share/classes.jfb/sun/nio/cs/ext/ExtendedCharsets.java
endif


clean::
	$(RM) -r $(CLASSDESTDIR)
	$(RM) $(CHARSETS_JAR)

