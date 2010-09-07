#
# @(#)Makefile	1.14 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JNDI RMI registry service provider
#

BUILDDIR = ../../../../..
PACKAGE = com.sun.jndi.rmi.registry
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#
AUTO_FILES_JAVA_DIRS = com/sun/jndi/rmi/registry com/sun/jndi/url/rmi

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

build: stubs

#
# The remainder of this makefile contains defs and rules
# for generating RMI stubs.
# With inspiration from build/solaris/sun/rmi/rmi/GNUmakefile.
#

#
# Full class names of implementations requiring stubs
#
REMOTE_impls = \
	$(PACKAGE).ReferenceWrapper

REMOTE_files = $(subst .,/,$(REMOTE_impls))
FILES_stubs  = $(REMOTE_files:%=$(CLASSDESTDIR)/%_Stub.class)
FILES_skels  = $(REMOTE_files:%=$(CLASSDESTDIR)/%_Skel.class)

#
# Compile stubs and skeletons for remote implementations
#
$(CLASSDESTDIR)/%_Skel.class $(CLASSDESTDIR)/%_Stub.class:	\
		$(CLASSDESTDIR)/%.class
	$(RMIC) -classpath "$(CLASSDESTDIR)"	\
		-d $(CLASSDESTDIR) 				\
		$(subst /,.,$(<:$(CLASSDESTDIR)/%.class=%))
	@$(java-vm-cleanup)

stubs: $(FILES_stubs) $(FILES_skels)
