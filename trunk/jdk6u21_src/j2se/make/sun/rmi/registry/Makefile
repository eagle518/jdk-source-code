#
# @(#)Makefile	1.33 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building sun.rmi.registry.* and the "rmiregistry" command
#

BUILDDIR = ../../..
PACKAGE = sun.rmi.registry
PRODUCT = sun
PROGRAM = rmiregistry
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
AUTO_FILES_JAVA_DIRS = sun/rmi/registry

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.rmi.registry.RegistryImpl\" }"
include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_PROPERTIES = rmiregistry.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes/sun/rmi/registry/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/rmi/registry/resources
include $(BUILDDIR)/common/Resources.gmk

