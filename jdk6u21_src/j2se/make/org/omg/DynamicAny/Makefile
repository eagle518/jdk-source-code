#
# @(#)Makefile	1.23 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# This makefile generates the classes defined in DynamicAny.idl.
# It doesn't override the existing ones because we are adding javadoc manually
# and therefore we have checked the files into the workspace.
# Ultimately we want to generate those files from idl with javadoc generated from
# idl comments.

BUILDDIR = ../../..
include $(BUILDDIR)/common/Defs.gmk

#
# What to compile?
#

PACKAGE = org.omg.DynamicAny
PRODUCT = org

#
# Files
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)org_omg_DynamicAny.jmk

FILES_java = $(DYNANY_GENERATED_java)

include $(BUILDDIR)/common/Classes.gmk
