#
# @(#)Makefile	1.15 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for building javazic utility & TimeZone resource files


BUILDDIR = ../..
PACKAGE = sun.tools.javazic
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
AUTO_FILES_JAVA_DIRS= sun/tools/javazic

# Time zone data file creation
TZDATA = ./tzdata/
TZDATA_VER = `cat $(TZDATA)VERSION`
TZFILE = \
    africa antarctica asia australasia europe northamerica \
    pacificnew southamerica backward \
    etcetera solar87 solar88 solar89 systemv
JDKTZDATA = ./tzdata_jdk/
JDKTZFILES = gmt jdk11_backward
TZFILES = \
    $(addprefix $(TZDATA),$(TZFILE)) \
    $(addprefix $(JDKTZDATA),$(JDKTZFILES))

WORKDIR = $(TEMPDIR)/zi
INSTALLDIR = $(LIBDIR)/zi
MAPFILE = ZoneInfoMappings

# Change the destination of the classfiles
CLASSDESTDIR=$(TEMPDIR)/classes

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

# Add to the build rule
build: $(INSTALLDIR)/$(MAPFILE)

$(WORKDIR)/$(MAPFILE): $(TZFILES)
	$(RM) -r $(@D)
	$(prep-target)
	$(JAVA) -classpath $(CLASSDESTDIR) sun.tools.javazic.Main -V "$(TZDATA_VER)" -d $(WORKDIR) $(TZFILES)
	@$(java-vm-cleanup)

$(INSTALLDIR)/$(MAPFILE): $(WORKDIR)/$(MAPFILE)
	$(RM) -r $(@D)
	$(prep-target)
	$(CP) -r $(WORKDIR)/* $(@D)

clean clobber::
	$(RM) -r $(TEMPDIR) $(INSTALLDIR)
