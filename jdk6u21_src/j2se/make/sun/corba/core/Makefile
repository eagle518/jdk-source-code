#
# @(#)Makefile	1.72 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building RMI/IIOP
#

BUILDDIR = ../../..
PACKAGE = com.sun.corba.se.internal.io
PRODUCT = sun
LIBRARY = ioser12
include $(BUILDDIR)/common/Defs.gmk

#
# Use mapfile
#
FILES_m = mapfile-vers
include $(BUILDDIR)/common/Mapfile-vers.gmk

#
# Files to compile.
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)javax_rmi.jmk
include $(CORBA_JMK_DIRECTORY)javax_rmi_CORBA.jmk
include $(CORBA_JMK_DIRECTORY)javax_transaction.jmk
include $(CORBA_JMK_DIRECTORY)javax_activity.jmk
include $(CORBA_JMK_DIRECTORY)ioser_io.jmk
include $(CORBA_JMK_DIRECTORY)sun_corba.jmk

ifdef STANDALONE_CORBA_WS
OTHER_LDFLAGS=-L$(BOOTDIR)/jre/lib/$(ARCH) -L$(BOOTDIR)/jre/lib/$(LIBARCH)/native_threads -ljvm
OTHER_INCLUDES+=-ICClassHeaders -I$(BOOTDIR)/include -I$(BOOTDIR)/include/$(PLATFORM)
else
OTHER_LDLIBS=$(JVMLIB)
endif


FILES_c = ioser.c

FILES_java = \
	$(javax_rmi_java) \
	$(javax_rmi_CORBA_java) \
	$(javax_transaction_java) \
	$(javax_activity_java) \
	$(IOSER_IO_java) \
	$(sun_corba_java)

#
# Generate header files for.
#
FILES_export = \
	com/sun/corba/se/internal/io/IIOPInputStream.java \
	com/sun/corba/se/internal/io/IIOPOutputStream.java \
	com/sun/corba/se/internal/io/ObjectStreamClass.java \
	com/sun/corba/se/internal/io/LibraryManager.java 
#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

#
# Resources
#
LOCALE_SET_DEFINITION = j2re
RESOURCE_BUNDLES_PROPERTIES = sunorb.properties
RESOURCE_SRC_DIR  = $(SHARE_SRC)/classes/com/sun/corba/se/impl/orbutil/resources
RESOURCE_DEST_DIR = $(CLASSBINDIR)/com/sun/corba/se/impl/orbutil/resources
include $(BUILDDIR)/common/Resources.gmk



#
# Extra clean rules because we build more than one package.
#
clean:: classheaders.clean objects.clean
	$(RM) -r $(CLASSBINDIR)/javax/rmi
	$(RM) -r $(CLASSBINDIR)/javax/transaction
	$(RM) -r $(CLASSBINDIR)/javax/activity
	$(RM) -r $(CLASSBINDIR)/com/sun/corba/se/impl

