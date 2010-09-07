# @(#)Makefile	1.43 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building the new java compiler.
#

BUILDDIR = ../../..
PRODUCT = sun
PACKAGE = com.sun.tools.javac
PROGRAM = javac
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files to compile.
#
include FILES_java.gmk

#
# Resources (must be made before .recompile).
#
LOCALE_SET_DEFINITION = j2sdk
RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
        com/sun/tools/javac/resources/compiler.properties \
        com/sun/tools/javac/resources/javac.properties
NEW_RESOURCE_BUNDLES_COMPILED_PROPERTIES = \
        com/sun/tools/javac/resources/legacy.properties
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
include $(BUILDDIR)/common/Resources.gmk

$(GENSRCDIR)/com/sun/tools/javac/resources/version.java: \
		$(SHARE_SRC)/classes/com/sun/tools/javac/resources/version.java
	$(SED) 	-e 's/$$(JDK_VERSION)/$(JDK_VERSION)/'  \
		-e 's/$$(FULL_VERSION)/$(FULL_VERSION)/' \
		-e 's/$$(RELEASE)/$(RELEASE)/' \
		< $< > $@

FILES_java += com/sun/tools/javac/resources/version.java

#
# Rules.
#
JAVA_ARGS = "{ \"-J-ms8m\", \"-J-Xss4m\", \"-J-ea:com.sun.tools...\", \"com.sun.tools.javac.Main\" }"
include $(BUILDDIR)/common/Program.gmk
OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS

# Use bootstrap compiler.
JAVAC_CMD = $(JAVAC_BOOT_CMD) 
