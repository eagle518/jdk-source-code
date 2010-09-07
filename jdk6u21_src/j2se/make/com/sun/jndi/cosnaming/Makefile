#
# @(#)Makefile	1.13 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JNDI COS Naming service provider
#

BUILDDIR = ../../../..
PACKAGE = com.sun.jndi.cosnaming
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/jndi/cosnaming com/sun/jndi/url

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Resources
#
OTHER_PROPERTIES  = jndiprovider.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)
include $(BUILDDIR)/common/Resources.gmk

