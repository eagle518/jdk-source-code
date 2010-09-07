#
# @(#)Makefile	1.4 10/03/23 
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jrunscript
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.script.shell
PRODUCT = sun
PROGRAM = jrunscript
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
AUTO_FILES_JAVA_DIRS = com/sun/tools/script/shell

#
# Files to be copied
#

RESOURCEDIR = \
	$(CLASSDESTDIR)/com/sun/tools/script/shell

FILES_copy = \
	$(RESOURCEDIR)/init.js \
	$(RESOURCEDIR)/messages.properties 

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.script.shell.Main\" }"
include $(BUILDDIR)/common/Program.gmk

build: copy-files

copy-files : $(FILES_copy)

$(CLASSDESTDIR)/%: $(SHARE_SRC)/classes/%
	$(install-file)

clean clobber::
	@$(RM) $(FILES_copy)

.PHONY: copy-files


