#
# @(#)Makefile	1.19 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.19 10/03/23
#
# Makefile for building rmiregistry program
#

BUILDDIR = ../../..
PACKAGE = sun.rmi
PRODUCT = sun
PROGRAM = java-rmi
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level 
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files to compile.
#
FILES_c = $(TARGDIR)java-rmi.c

#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk

build: $(BINDIR)/$(PROGRAM).cgi

#
# Rename .exe to .cgi.
#

$(BINDIR)/$(PROGRAM).cgi: $(BINDIR)/$(PROGRAM).exe
	$(install-file)

clean clobber:: 
	$(RM) $(BINDIR)/$(PROGRAM).cgi

