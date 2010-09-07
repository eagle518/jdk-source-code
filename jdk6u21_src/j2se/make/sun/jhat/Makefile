#
# @(#)Makefile	1.6 10/03/23 
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jhat
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.hat
PRODUCT = sun
PROGRAM = jhat
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files.
#
AUTO_FILES_JAVA_DIRS = com/sun/tools/hat

#
# Files to be copied
#

RESOURCEDIR = \
	$(CLASSDESTDIR)/com/sun/tools/hat/resources

FILES_copy = \
	$(RESOURCEDIR)/hat.js \
	$(RESOURCEDIR)/oqlhelp.html \
	$(RESOURCEDIR)/platform_names.txt

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.hat.Main\" }"
include $(BUILDDIR)/common/Program.gmk

build: copy-files

copy-files : $(FILES_copy)

$(CLASSDESTDIR)/%: $(SHARE_SRC)/classes/%
	$(install-file)

clean clobber::
	@$(RM) $(FILES_copy)

.PHONY: copy-files


