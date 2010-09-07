#
# @(#)Makefile	1.35 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the Java 2D Demo
#

BUILDDIR = ../../..
DEMOS    = Java2Demo

# identify this as a demo/jfc/Java2D product - used to identify a
# unique name for OBJDIR
PRODUCT = demo/jfc/Java2D

include $(BUILDDIR)/common/Defs.gmk

#
# What do we compile?
#
DEMODST  = $(DEMODIR)/jfc/Java2D
DEMOSRC  = $(SHARE_SRC)/demo/jfc/Java2D
include FILES.gmk

#
# Activate some hooks so Classes.gmk looks elsewhere for sources, and
# dumps classes where we want.
#
SOURCEPATH              = $(DEMOSRC)

# use "simple expansion" (GNU Make 6.2) for setting BOOTCLASSPATH 
# because CLASSBINDIR is changed a few lines down:
BOOTCLASSPATH           := $(CLASSBINDIR)

DEMOJAR			= $(DEMODST)/Java2Demo.jar
ALT_CLASSES_DISPOSITION = src docs $(DEMOJAR)
ALT_CLASSBINDIR         = $(TEMPDIR)/class
CLASSDESTDIR            = $(ALT_CLASSBINDIR)
CLASSBINDIR             = $(ALT_CLASSBINDIR)

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

build: src docs $(DEMOJAR)

#
# Sources.
#
SRCSRC = $(DEMOSRC)
SRCDST = $(DEMODST)/src
FILES_src = $(FILES_java:%.java=$(SRCDST)/%.java)

src: $(FILES_src) $(FILES_other)

$(SRCDST)/%.java: $(SRCSRC)/%.java
	$(install-file)

$(SRCDST)/%: $(DEMOSRC)/%
	$(install-file)

#
# Docs.
#
docs: $(DEMODST)/README.txt $(DEMODST)/Java2Demo.html

$(DEMODST)/README.txt: $(DEMOSRC)/README.txt
	$(install-file)

$(DEMODST)/Java2Demo.html: $(DEMOSRC)/Java2Demo.html
	$(install-file)

#
# Jar file.
#

#
# Since there is no easy way to exclude SCCS files from
# being 'jar'ed together while using -C flag,
# temporarily copy only resource files into separate directory
# and then use jar on it
#

define copy-resourcefiles-to-temp
$(MKDIR) $(TEMPDIR)/images
$(CP) $(DEMOSRC)/images/*.* $(TEMPDIR)/images
$(MKDIR) $(TEMPDIR)/fonts
$(CP) $(DEMOSRC)/fonts/*.* $(TEMPDIR)/fonts
endef

define remove-temp-resourcefiles
$(RM) -rf $(TEMPDIR)/images
$(RM) -rf $(TEMPDIR)/fonts
endef

jar: $(DEMODST)/Java2Demo.jar

$(DEMODST)/Java2Demo.jar: $(FILES_class) $(FILES_java:%.java=$(DEMOSRC)/%.java)
	$(ECHO) Main-Class: java2d.Java2Demo > $(TEMPDIR)/manifest
	$(copy-resourcefiles-to-temp)
	$(JAR) -cvfm $@ $(TEMPDIR)/manifest -C $(TEMPDIR)/class . \
		     -C $(TEMPDIR) images -C $(TEMPDIR) fonts \
			  $(JAR_JFLAGS)
	$(remove-temp-resourcefiles)
	$(RM) $(TEMPDIR)/manifest
	@$(java-vm-cleanup)
