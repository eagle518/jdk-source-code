#
# @(#)Makefile	1.6 10/03/23 
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building intrinsic.properties for Pack200
# Unfortunately this file needs to be on its own due
# to a collision of rules with Classes.gmk and Library.gmk

BUILDDIR = ../../../../..
PACKAGE = com.sun.java.util.jar.pack
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Rules.
#
include $(BUILDDIR)/common/Classes.gmk

# 
# Resources
#
OTHER_PROPERTIES  = intrinsic.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/$(PKGDIR)
RESOURCE_DEST_DIR = $(CLASSDESTDIR)/$(PKGDIR)
include $(BUILDDIR)/common/Resources.gmk
