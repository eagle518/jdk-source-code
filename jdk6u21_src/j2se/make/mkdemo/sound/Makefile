#
# @(#)Makefile	1.23 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the Java Sound demo
#

BUILDDIR = ../..
DEMOS    = JavaSound

# identify this as a demo/sound product - used to identify a
# unique name for OBJDIR
PRODUCT = demo/sound

include $(BUILDDIR)/common/Defs.gmk

#
# What do we compile?
#
DEMODST  = $(DEMODIR)/sound
DEMOSRC  = $(SHARE_SRC)/demo/sound
include FILES.gmk


#
# Activate some hooks so Classes.gmk looks elsewhere for sources, and
# dumps classes where we want.
#
SOURCEPATH              = $(DEMOSRC)

# use "simple expansion" (GNU Make 6.2) for setting BOOTCLASSPATH 
# because CLASSBINDIR is changed a few lines down:
BOOTCLASSPATH           := $(CLASSBINDIR)

DEMOJAR			= $(DEMODST)/JavaSound.jar
ALT_CLASSES_DISPOSITION = src $(DEMOJAR) docs
ALT_CLASSBINDIR         = $(TEMPDIR)/class
CLASSDESTDIR            = $(ALT_CLASSBINDIR)
CLASSBINDIR             = $(ALT_CLASSBINDIR)

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

build: src $(DEMOJAR) docs

#
# Sources.
#
SRCSRC = $(DEMOSRC)
SRCDST = $(DEMODST)/src
FILES_src = $(FILES_java:%.java=$(SRCDST)/%.java)

src: $(FILES_src)

$(SRCDST)/Runner.java:
	@$(ECHO) Skipping Runner.java

$(SRCDST)/%.java: $(SRCSRC)/%.java
	$(install-file)

#
# Docs.
#
docs: $(DEMODST)/README.txt $(DEMODST)/JavaSound.html

$(DEMODST)/README.txt: $(DEMOSRC)/README.txt
	$(install-file)

$(DEMODST)/JavaSound.html: $(DEMOSRC)/JavaSound.html
	$(install-file)

#
# Jar file.
#
jar: $(DEMODST)/JavaSound.jar

$(DEMODST)/JavaSound.jar: $(FILES_class) $(FILES_java:%.java=$(DEMOSRC)/%.java)
		$(ECHO) Main-Class: JavaSound > $(TEMPDIR)/manifest
		$(JAR) -cvfm $@ $(TEMPDIR)/manifest -C $(TEMPDIR)/class .  \
		     $(JAR_JFLAGS)
		$(RM) $(TEMPDIR)/manifest
		@$(java-vm-cleanup)

