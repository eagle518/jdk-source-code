#
# @(#)Makefile	1.5 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building jvmstat support libraries
#

BUILDDIR = ../..
PACKAGE = sun.jvmstat
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
AUTO_FILES_JAVA_DIRS = sun/jvmstat

FILES_copy = \
	$(CLASSDESTDIR)/sun/jvmstat/perfdata/resources/aliasmap

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

build: copy-files

copy-files: $(FILES_copy)

$(CLASSDESTDIR)/%: $(SHARE_SRC)/classes/%
	$(install-file)

clean clobber::
	@$(RM) $(FILES_copy)

.PHONY: copy-files
