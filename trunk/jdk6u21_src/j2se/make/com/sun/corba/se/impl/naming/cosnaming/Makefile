#
# @(#)Makefile  1.10 03/01/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build the tnameserv executable.
#

BUILDDIR = ../../../../../../..
PACKAGE = com.sun.corba.se.impl.naming.cosnaming
PRODUCT = sun
PROGRAM = tnameserv
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files.
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_naming_cosnaming.jmk
FILES_java = $(com_sun_corba_se_impl_naming_cosnaming_java)

#
# Rules.
#
JAVA_ARGS = "{ \"-J-Dcom.sun.CORBA.activation.DbDir=./orb.db\",\"-J-Djava.util.logging.LoggingPermission=contol\", \"-J-Dcom.sun.CORBA.POA.ORBServerId=1\", \"com.sun.corba.se.impl.naming.cosnaming.TransientNameServer\" }"

ifdef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/idlj.c $(SHARE_SRC)/native/bin/utility.c
OTHER_CPPFLAGS = -DMAIN_CLASS=\"com.sun.corba.se.internal.CosNaming.TransientNam
eServer\"
endif

include $(BUILDDIR)/common/Program.gmk

