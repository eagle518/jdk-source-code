#
# @(#)Makefile	1.12 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JDBC API and JDBC RowSet
#

BUILDDIR = ../..
PACKAGE = javax.sql
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile.
#
AUTO_FILES_JAVA_DIRS = javax/sql

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

# 
# Resources
#
OTHER_PROPERTIES  = rowset.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/$(PKGDIR)/rowset
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)/rowset
include $(BUILDDIR)/common/Resources.gmk
