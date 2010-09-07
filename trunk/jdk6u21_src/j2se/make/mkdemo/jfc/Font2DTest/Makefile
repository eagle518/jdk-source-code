#
# @(#)Makefile	1.24 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.24 10/03/23
#
# Makefile for building the Font2DTest Demo
#

BUILDDIR = ../../..
DEMOS    = Font2DTest

# identify this as a demo/jfc/Font2DTest product - used to identify a
# unique name for OBJDIR
PRODUCT = demo/jfc/Font2DTest

include $(BUILDDIR)/common/Defs.gmk

#
# What do we compile?
#
DEMODST  = $(DEMODIR)/jfc/Font2DTest
DEMOSRC  = $(SHARE_SRC)/demo/jfc/Font2DTest
include FILES.gmk

#
# Activate some hooks so Classes.gmk looks elsewhere for sources, and
# dumps classes where we want.
#
SOURCEPATH              = $(DEMOSRC)

# use "simple expansion" (GNU Make 6.2) for setting BOOTCLASSPATH 
# because CLASSBINDIR is changed a few lines down:
BOOTCLASSPATH           := $(CLASSBINDIR)

DEMOJAR	                = $(DEMODST)/Font2DTest.jar
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

src: $(FILES_src)

$(SRCDST)/%.java: $(SRCSRC)/%.java
	$(install-file)

#
# Docs.
#
docs: $(DEMODST)/README.txt $(DEMODST)/Font2DTest.html

$(DEMODST)/README.txt: $(DEMOSRC)/README.txt
	$(install-file)

$(DEMODST)/Font2DTest.html: $(DEMOSRC)/Font2DTest.html
	$(install-file)

#
# Jar file.
#
jar: $(DEMODST)/Font2DTest.jar

$(DEMODST)/Font2DTest.jar: $(FILES_class) $(FILES_java:%.java=$(DEMOSRC)/%.java)
	$(prep-target)
	$(RM) $(TEMPDIR)/manifest
	$(ECHO) "Main-Class: Font2DTest" > $(TEMPDIR)/manifest
	$(RM) $(TEMPDIR)/class/blank.gif
	$(CP) $(DEMOSRC)/images/blank.gif $(TEMPDIR)/class/blank.gif
	$(RM) $(TEMPDIR)/class/yes.gif
	$(CP) $(DEMOSRC)/images/yes.gif $(TEMPDIR)/class/yes.gif
	$(JAR) -cvfm $@ $(TEMPDIR)/manifest -C $(TEMPDIR)/class . \
	     $(JAR_JFLAGS)
	$(RM) $(TEMPDIR)/manifest
	@$(java-vm-cleanup)

