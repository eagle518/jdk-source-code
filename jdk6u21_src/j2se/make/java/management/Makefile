#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
PACKAGE = java.lang.management
LIBRARY = management
PRODUCT = java
include $(BUILDDIR)/common/Defs.gmk

MGMT_SRC  = $(SHARE_SRC)/classes/java/lang/management
SMGMT_SRC = $(SHARE_SRC)/classes/sun/management

AGENTJAR = $(LIBDIR)/management-agent.jar
MANIFEST = $(SMGMT_SRC)/manifest

#
# Use mapfile
#
FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files to compile
#
include FILES_c.gmk

AUTO_FILES_JAVA_DIRS = java/lang/management com/sun/management sun/management

include Exportedfiles.gmk

ifeq ($(PLATFORM),windows)

FILES_c     +=  $(TARGDIR)OperatingSystem_md.c

FILES_export +=	com/sun/management/OperatingSystem.java

else # PLATFORM (i.e. solaris & linux)

FILES_c     +=  $(TARGDIR)UnixOperatingSystem_md.c

FILES_export +=	com/sun/management/UnixOperatingSystem.java
endif # PLATFORM

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_COMPILED_PROPERTIES = sun/management/resources/agent.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk


# Find C source files
#
vpath %.c $(SHARE_SRC)/native/sun/management
vpath %.c $(PLATFORM_SRC)/native/sun/management
vpath %.c $(PLATFORM_SRC)/native/com/sun/management

#
# Various variables
#

JAVAHFLAGS += -jni      # Generate JNI-style header files

# Access to management.h 
#

OTHER_INCLUDES += \
  -I$(SHARE_SRC)/native/sun/management 

ifeq ($(PLATFORM),windows)
  OTHER_LDLIBS += $(JVMLIB)
endif

#
# Library to compile.
#
include $(BUILDDIR)/common/Library.gmk

# Use JAR command from bootstrap binary as JAR is not built before
# the management package
$(AGENTJAR): $(LIBDIR) $(TEMPDIR)/manifest
	$(BOOTDIR)/bin/jar -cfm $(AGENTJAR) $(TEMPDIR)/manifest
                                                                                                     
$(TEMPDIR)/manifest: $(MANIFEST)
	$(install-file)

build: $(AGENTJAR)

clean clobber::
	$(RM) -r $(CLASSDESTDIR)/java/management
	$(RM) -r $(CLASSDESTDIR)/sun/management
	$(RM) $(TEMPDIR)/manifest $(AGENTJAR)

