#
# @(#)Makefile	1.3 10/03/23 16:59:35
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of sun.org.mozilla.javascript.internal.*
#

BUILDDIR = ../../../..
PACKAGE = sun.org.mozilla.javascript.internal
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

AUTO_FILES_JAVA_DIRS = sun/org/mozilla/javascript/internal

RESOURCEDIR = \
	$(CLASSDESTDIR)/sun/org/mozilla/javascript/internal/resources
	
FILES_copy = \
	$(RESOURCEDIR)/Messages.properties \
	$(RESOURCEDIR)/Messages_fr.properties

#
# Rules
#

include $(BUILDDIR)/common/Classes.gmk

all: classes copy-files

#
# Copy resource messages file for Rhino JavaScript interpreter
#

copy-files: $(FILES_copy)

$(RESOURCEDIR)/%: $(SHARE_SRC)/classes/sun/org/mozilla/javascript/internal/resources/%
	$(install-file)

.PHONY: copy-files

clean clobber::
	$(RM) -rf $(CLASSDESTDIR)/sun/org/mozilla/javascript
