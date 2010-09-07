#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of JIIO (Java Image I/O) API
#

BUILDDIR = ../..
PACKAGE = javax.imageio
PRODUCT = jiio
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = javax/imageio com/sun/imageio 

#
# Properties
#
OTHER_PROPERTIES = com/sun/imageio/plugins/common/iio-plugin.properties

LOCALE_SET_DEFINITION = j2re
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
RESOURCE_DEST_DIR = $(CLASSBINDIR)
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

#
# Rules that copy files.
#

build: copy-files $(PFILES)

copy-files: $(FILES_copy)

#	
# Declare extra phony targets.
#
.PHONY: copy-files
