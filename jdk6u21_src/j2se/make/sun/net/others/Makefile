#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = sun.net
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
include $(BUILDDIR)/sun/net/FILES_java.gmk

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

build: properties nameprep-profile

clean clobber:: properties.clean nameprep-profile.clean

#
# Install/nuke properties.
#
PROPS = $(PLATFORM_SRC)/lib/content-types.properties

$(LIBDIR)/content-types.properties: $(PROPS)
	$(install-file)

properties: $(LIBDIR)/content-types.properties

properties.clean:
	$(RM) -r $(LIBDIR)/content-types.properties

#
# Install/nuke nameprep profile.
#
NAMEPREP_PROFILE_SRC   = $(TOPDIR)/src/share/classes/sun/net/idn/uidna.spp
NAMEPREP_PROFILE_BUILD = $(CLASSBINDIR)/sun/net/idn/uidna.spp

nameprep-profile: $(NAMEPREP_PROFILE_BUILD)

nameprep-profile.clean:
	$(RM) -r $(NAMEPREP_PROFILE_BUILD)

$(NAMEPREP_PROFILE_BUILD): $(NAMEPREP_PROFILE_SRC)
	$(install-file)

$(NAMEPREP_PROFILE_SRC):

.PHONY: properties properties.clean nameprep-profile nameprep-profile.clean

