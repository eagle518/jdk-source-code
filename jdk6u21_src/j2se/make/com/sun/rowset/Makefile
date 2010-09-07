#
# @(#)Makefile	1.6 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JDBC API and JDBC RowSet
#

BUILDDIR = ../../..
PACKAGE = com.sun.rowset
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
AUTO_FILES_JAVA_DIRS = com/sun/rowset

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_PROPERTIES = RowSetResourceBundle.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/com/sun/rowset
RESOURCE_DEST_DIR = $(CLASSBINDIR)/com/sun/rowset
include $(BUILDDIR)/common/Resources.gmk
