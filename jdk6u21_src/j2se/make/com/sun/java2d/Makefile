#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = com.sun.java2d.fontchecker
PRODUCT = fontchecker
# Destination directory looks like  :-
# ../../../../build/solaris-sparc/tmp/fontchecker/com.sun.java2d.fontchecker
# This is the location the installer needs to get the FontChecker.jar from
FCDEST = $(TEMPDIR)
FCJAR = $(FCDEST)/FontChecker.jar
ALT_CLASSES_DISPOSITION = $(FCJAR)
ALT_CLASSBINDIR		= $(TEMPDIR)/classes
CLASSDESTDIR		= $(ALT_CLASSBINDIR)
CLASSBINDIR		= $(ALT_CLASSBINDIR)

AUTO_FILES_JAVA_DIRS = com/sun/java2d/fontchecker

include $(BUILDDIR)/common/Defs.gmk

# Only need to build this on windows, but no harm to make sure
# it builds properly elsewhere.
#
#ifneq ($(PLATFORM), windows)
#classes:
#else

include $(BUILDDIR)/common/Classes.gmk

build: jar

jar:	$(FCJAR)

$(FCJAR): $(FILES_class) 
	$(ECHO) Main-Class: com.sun.java2d.fontchecker.FontChecker > $(TEMPDIR)/manifest
	$(JAR) -cvfm $@ $(TEMPDIR)/manifest -C $(TEMPDIR)/classes . \
	     $(JAR_JFLAGS)
	$(RM) $(TEMPDIR)/manifest
	@$(java-vm-cleanup)

.PHONY: jar

#endif

clobber clean::
	$(ECHO) Removing $(TEMPDIR)
	$(RM) -r $(TEMPDIR)

