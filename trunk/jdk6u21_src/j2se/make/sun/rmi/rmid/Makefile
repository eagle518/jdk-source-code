#
# @(#)Makefile	1.38 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Install a wrapper for the "rmid" command. The associated classfiles
# are built at a higher level.
#

BUILDDIR = ../../..
PACKAGE = sun.rmi.activation
PRODUCT = sun
PROGRAM = rmid
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level 
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
# 
BUILD_MANIFEST=true

build: stubs

#
# Rules to install the wrapper script
#
JAVA_ARGS = "{ \"-J-ms8m\", \"sun.rmi.server.Activation\" }"
include $(BUILDDIR)/common/Program.gmk

#
# Resources.
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_PROPERTIES = rmid.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/sun/rmi/server/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/sun/rmi/rmid/resources
include $(BUILDDIR)/common/Resources.gmk

#
# Extra dependencies.
#
classes: stubs

#
# Rules to build activation-system stubs
#

# Full package names of implementations requiring stubs
REMOTE_impls = \
	sun.rmi.server.Activation$$ActivationSystemImpl \
	java.rmi.activation.ActivationGroup

REMOTE_files = $(subst .,/,$(REMOTE_impls))
FILES_stubs = $(REMOTE_files:%=$(CLASSBINDIR)/%_Stub.class)

#
# Compile stubs for remote implementations
# (use -v1.2 stubs for activation system)
#

$(CLASSBINDIR)/%_Stub.class: $(CLASSBINDIR)/%.class
	$(RMIC) -v1.2 -classpath $(CLASSBINDIR)	\
	    	-d $(CLASSBINDIR) '$(subst /,.,$(<:$(CLASSBINDIR)/%.class=%))'
	@$(java-vm-cleanup)

stubs: $(FILES_stubs)

