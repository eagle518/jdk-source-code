#
# @(#)Makefile	1.47 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building sun.rmi.rmic.* and the "rmic" command
#

BUILDDIR = ../../../../../..
PACKAGE = sun.rmi.rmic
PRODUCT = sun
PROGRAM = rmic
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#

include SUN_RMI_RMIC_IIOP_java.gmk
include SUN_RMI_RMIC_java.gmk

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

ifndef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/$(PROGRAM).c $(SHARE_SRC)/native/bin/utility.c
include $(BUILDDIR)/common/Program.gmk
else
include $(BUILDDIR)/common/Classes.gmk
endif

#
# Resources.
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_PROPERTIES = rmic.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/sun/rmi/rmic/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/rmi/rmic/resources
include $(BUILDDIR)/common/Resources.gmk

