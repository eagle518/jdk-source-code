#
# @(#)Makefile	1.6 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for sun.util.resources that don't need
# to be part of the bootstrap build.
#

BUILDDIR = ../..

PACKAGE = java.util.resources
PRODUCT = java

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

#  Adds more files than before?: AUTO_FILES_JAVA_DIRS=sun/util/resources
include FILES_java.gmk

#  Adds more files than before?: AUTO_FILES_PROPERTIES_DIRS=sun/util/resources
include FILES_properties.gmk

#
# Compile properties
#
COMPILED_PROPERTIES_SUPERCLASS=LocaleNamesBundle

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

