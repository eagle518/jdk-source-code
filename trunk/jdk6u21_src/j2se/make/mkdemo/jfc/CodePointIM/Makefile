#
# @(#)Makefile	1.8 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the CodePointInputMethod
#

BUILDDIR = ../../..
DEMOS    = CodePointIM

# identify this as a demo/jfc/CodePointIM product - used to identify
# a unique name for OBJDIR
PRODUCT = demo/jfc/CodePointIM

include $(BUILDDIR)/common/Defs.gmk

#
# What do we compile?
#
DEMODST  = $(DEMODIR)/jfc/CodePointIM
DEMOSRC  = $(SHARE_SRC)/demo/jfc/CodePointIM
include FILES.gmk

#
# Activate some hooks so Classes.gmk looks elsewhere for sources, and
# dumps classes where we want.
#
SOURCEPATH              = $(DEMOSRC)

# use "simple expansion" (GNU Make 6.2) for setting BOOTCLASSPATH 
# because CLASSBINDIR is changed a few lines down:
BOOTCLASSPATH           := $(CLASSBINDIR)

DEMOJAR	                = $(DEMODST)/CodePointIM.jar
ALT_CLASSES_DISPOSITION = src resources docs $(DEMOJAR)
ALT_CLASSBINDIR         = $(TEMPDIR)/class
CLASSDESTDIR            = $(ALT_CLASSBINDIR)
CLASSBINDIR             = $(ALT_CLASSBINDIR)

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

build: src resources docs $(DEMOJAR)

#
# Sources.
#
SRCSRC = $(DEMOSRC)
SRCDST = $(DEMODST)/src
FILES_src = $(FILES_java:%.java=$(SRCDST)/%.java)

src: $(FILES_src)

$(SRCDST)/%.java: $(SRCSRC)/%.java
	$(install-file)

#
# Resources.
#
FILES_res = $(FILES_resources:%.properties=$(SRCDST)/%.properties)

resources: $(FILES_res)

$(SRCDST)/%.properties: $(SRCSRC)/%.properties
	$(install-file)

#
# Docs.
#
FILES_doc = $(FILES_readme:%.html=$(DEMODST)/%.html)

docs: $(FILES_doc)

$(DEMODST)/%.html: $(DEMOSRC)/%.html
	$(install-file)

#
# Jar file.
#
DESCRIPTOR_DIR = $(TEMPDIR)/class/META-INF/services
DESCRIPTOR = $(DEMOSRC)/java.awt.im.spi.InputMethodDescriptor
RESOURCES_DIR = $(TEMPDIR)/class/resources
RESOURCES = $(FILES_resources:%.properties=$(DEMOSRC)/%.properties)

jar: $(DEMODST)/CodePointIM.jar

$(DEMODST)/CodePointIM.jar: $(DESCRIPTOR) $(FILES_class) $(FILES_java:%.java=$(DEMOSRC)/%.java) $(RESOURCES)
	$(ECHO) Main-Class: CodePointIM > $(TEMPDIR)/manifest
	$(MKDIR) -p $(DESCRIPTOR_DIR) $(RESOURCES_DIR)
	$(CP) -f $(DESCRIPTOR) $(DESCRIPTOR_DIR)
	$(CP) -f $(RESOURCES) $(RESOURCES_DIR)
	$(JAR) -cvfm $@ $(TEMPDIR)/manifest -C $(TEMPDIR)/class . \
	     $(JAR_JFLAGS)
	$(RM) $(TEMPDIR)/manifest
	@$(java-vm-cleanup)

