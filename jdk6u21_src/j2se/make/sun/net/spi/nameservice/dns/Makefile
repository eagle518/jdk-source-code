#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building JNDI service provider toolkit
#

BUILDDIR = ../../../../..
PACKAGE = sun.net.spi.nameservice.dns
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk
JARFILE = $(EXTDIR)/dnsns.jar

#
# Files to compile
#
FILES_java = \
	sun/net/spi/nameservice/dns/DNSNameServiceDescriptor.java \
        sun/net/spi/nameservice/dns/DNSNameService.java

#
# Rules
#
CLASSDESTDIR = $(TEMPDIR)/classes

FILES_class = $(FILES_java:%.java=$(CLASSDESTDIR)/%.class)


all : build $(JARFILE)

#
# META-INF/services
#
SERVICE_DESCRIPTION = sun.net.spi.nameservice.NameServiceDescriptor

$(CLASSDESTDIR)/META-INF/services/$(SERVICE_DESCRIPTION): \
  $(SHARE_SRC)/classes/$(PKGDIR)/META-INF/services/$(SERVICE_DESCRIPTION)
	$(install-file)

#
# JAR file
#
$(JARFILE): $(CLASSDESTDIR)/META-INF/services/$(SERVICE_DESCRIPTION) \
	$(FILES_class)
	$(JAR) -cf $(JARFILE) -C $(CLASSDESTDIR) sun -C $(CLASSDESTDIR) META-INF \
	     $(JAR_JFLAGS)
	@$(java-vm-cleanup)

include $(BUILDDIR)/common/Classes.gmk

clean clobber::
	$(RM) -r $(CLASSDESTDIR) \
		 $(EXTDIR)/$(JARFILE)
