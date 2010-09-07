#
# @(#)Makefile	1.7 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building sun.rmi.rmic.* and the "rmic" command
#

BUILDDIR = ../../..
PACKAGE = sun.rmi.rmic
PRODUCT = sun
PROGRAM = rmic
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

include FILES.gmk

ifdef PARTNERS_CORBA
FILES_java = \
	$(RMIC_java) \
	$(IIOP_java) \
	$(PARTNERS_EDITION_java)
else
FILES_java = \
	$(RMIC_java) \
	$(IIOP_java)
endif

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.rmi.rmic.Main\" }"

ifdef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/$(PROGRAM).c $(SHARE_SRC)/native/bin/utility.c
endif

include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = rmic.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/sun/rmi/rmic/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/rmi/rmic/resources
include $(BUILDDIR)/common/Resources.gmk

