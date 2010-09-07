#
# @(#)Makefile	1.84 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.awt
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
AUTO_FILES_JAVA_DIRS = java/awt sun/awt/geom

build: sources properties cursors

#
# Resources
#
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/awt/resources/awt.properties
LOCALE_SET_DEFINITION = j2re
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

sources: debughelper

clean:: debughelper.clean properties.clean sun.geom.clean cursors.clean

debughelper:
	$(CD) $(BUILDDIR)/sun/awt; $(MAKE) debughelper

debughelper.clean:
	$(CD) $(BUILDDIR)/sun/awt; $(MAKE) debughelper.clean


_LIBPROPS = \
	flavormap.properties

LIBPROPS = $(_LIBPROPS:%=$(LIBDIR)/%)

properties: $(LIBDIR) $(LIBPROPS)

$(LIBDIR)/%.properties: $(PLATFORM_SRC)/lib/%.properties
	$(install-file)

properties.clean :
	$(RM) $(LIBPROPS)

.SUFFIXES : $(SUFFIXES) .gif .properties

CURSORSRC     = $(TOPDIR)/src/share/lib/images/cursors
CURSORDIR     = $(LIBDIR)/images/cursors

ifeq ($(PLATFORM), windows)
CURSORPROPSRC = $(TOPDIR)/src/windows/lib/images/cursors
CURSORS   = $(CURSORDIR)/win32_CopyDrop32x32.gif      \
            $(CURSORDIR)/win32_MoveDrop32x32.gif      \
            $(CURSORDIR)/win32_LinkDrop32x32.gif      \
            $(CURSORDIR)/win32_CopyNoDrop32x32.gif    \
            $(CURSORDIR)/win32_MoveNoDrop32x32.gif    \
            $(CURSORDIR)/win32_LinkNoDrop32x32.gif    \
            $(CURSORDIR)/invalid32x32.gif
else # PLATFORM
CURSORPROPSRC = $(TOPDIR)/src/solaris/lib/images/cursors
CURSORS   = $(CURSORDIR)/motif_CopyDrop32x32.gif	\
	    $(CURSORDIR)/motif_MoveDrop32x32.gif	\
	    $(CURSORDIR)/motif_LinkDrop32x32.gif	\
	    $(CURSORDIR)/motif_CopyNoDrop32x32.gif	\
	    $(CURSORDIR)/motif_MoveNoDrop32x32.gif	\
	    $(CURSORDIR)/motif_LinkNoDrop32x32.gif	\
	    $(CURSORDIR)/invalid32x32.gif
endif # PLATFORM

$(CURSORDIR)/cursors.properties : $(CURSORPROPSRC)/cursors.properties
	$(install-file)

$(CURSORDIR)/%.gif : $(CURSORSRC)/%.gif
	$(install-file)

cursors : $(CURSORDIR)/cursors.properties $(CURSORS)

sun.geom.clean :
	$(RM) -r $(CLASSBINDIR)/sun/awt/geom

cursors.clean :
	$(RM) -r $(CURSORDIR)

.PHONY: sources debughelper debughelper.clean properties properties.clean \
	cursors sun.geom.clean cursors.clean

