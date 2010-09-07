#
# @(#)Makefile	1.27 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of java
#

BUILDDIR = ../../..

PACKAGE = com.sun.tools.corba.se.idl
PRODUCT = sun
ifndef STANDALONE_CORBA_WS
PROGRAM = idlj
endif

include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files
#
ifndef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/$(PROGRAM).c $(SHARE_SRC)/native/bin/utility.c
endif

include FILES_java.gmk

JAVA_ARGS = "{ \"com.sun.tools.corba.se.idl.toJavaPortable.Compile\" }"

ifndef STANDALONE_CORBA_WS
#
# Rules.
#
include $(BUILDDIR)/common/Program.gmk
else
include $(BUILDDIR)/common/Classes.gmk
endif

#
# Resources.
#

LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = idl.prp toJavaPortable/toJavaPortable.prp
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/com/sun/tools/corba/se/idl
RESOURCE_DEST_DIR = $(CLASSBINDIR)/com/sun/tools/corba/se/idl
include $(BUILDDIR)/common/Resources.gmk

#
# IDL files
#

IDL_FILES = orb.idl ir.idl

IDL_SRC_DIR  = $(SHARE_SRC)/classes/com/sun/tools/corba/se/idl
IDL_DEST_DIR = $(LIBDIR)

IFILES = $(IDL_FILES:%=$(IDL_DEST_DIR)/%)

build: $(IFILES)

$(IDL_DEST_DIR)/%: $(IDL_SRC_DIR)/%
	$(install-file)

