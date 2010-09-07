#
# @(#)Makefile	1.7 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jstat
#

BUILDDIR = ../..
PACKAGE = sun.tools.jstat
PRODUCT = sun
PROGRAM = jstat
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
AUTO_FILES_JAVA_DIRS = sun/tools/jstat

FILES_copy = \
	$(CLASSDESTDIR)/sun/tools/jstat/resources/jstat_options

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.tools.jstat.Jstat\" }"
include $(BUILDDIR)/common/Program.gmk

build: copy-files
 
copy-files: $(FILES_copy)
 
$(CLASSDESTDIR)/%: $(SHARE_SRC)/classes/%
	$(install-file)
        
clean clobber::
	@$(RM) $(FILES_copy)
        
.PHONY: copy-files

