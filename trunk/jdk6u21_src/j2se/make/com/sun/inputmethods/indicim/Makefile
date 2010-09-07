#
# @(#)Makefile	1.11 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for indic input method.
#

BUILDDIR = ../../../..

PACKAGE = com.sun.inputmethods.internal.indicim
PRODUCT = sun

CLASSDESTDIR = $(TEMPDIR)/classes
SERVICESDIR=META-INF/services
IMJAR = $(IMJARDIR)/indicim.jar
IMJARDIR=$(LIBDIR)/im
include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
AUTO_FILES_JAVA_DIRS = com/sun/inputmethods/internal/indicim

FILES_copy = \
	$(CLASSDESTDIR)/com/sun/inputmethods/internal/indicim/resources/DisplayNames.properties

PROVIDER_CONF_FILE = \
	$(CLASSDESTDIR)/$(SERVICESDIR)/java.awt.im.spi.InputMethodDescriptor

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

build: $(IMJAR)

#
# Extra rules to build im.
#

$(CLASSDESTDIR)/com/sun/inputmethods/internal/indicim/resources/% : $(SHARE_SRC)/classes/com/sun/inputmethods/internal/indicim/resources/%
	$(install-file)

$(CLASSDESTDIR)/$(SERVICESDIR)/java.awt.im.spi.InputMethodDescriptor : $(SHARE_SRC)/classes/com/sun/inputmethods/internal/indicim/java.awt.im.spi.InputMethodDescriptor
	$(install-file)

$(IMJAR): $(FILES_class) $(FILES_copy) $(PROVIDER_CONF_FILE)
	$(prep-target)
	$(JAR) -cf $@ -C $(CLASSDESTDIR) com -C $(CLASSDESTDIR) $(SERVICESDIR) \
	     $(JAR_JFLAGS)
	@$(java-vm-cleanup)

clean::
	$(RM) -r $(CLASSDESTDIR)
	$(RM) $(IMJAR)
