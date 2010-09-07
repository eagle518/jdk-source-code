#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../../../../../..
PACKAGE = com.sun.corba.se.impl.activation
PRODUCT = Activation
PROGRAM = orbd

include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_activation.jmk

#
# Rules.
#
# default values passed using the -J property
JAVA_ARGS = "{ \"-J-Dcom.sun.CORBA.activation.DbDir=./orb.db\", \"-J-Dcom.sun.CORBA.activation.Port=1049\", \"-J-Dcom.sun.CORBA.POA.ORBServerId=1\",\"com.sun.corba.se.impl.activation.ORBD\" }"

ifdef STANDALONE_CORBA_WS
FILES_c = $(SHARE_SRC)/native/bin/idlj.c $(SHARE_SRC)/native/bin/utility.c
OTHER_CPPFLAGS = -DMAIN_CLASS=\"com.sun.corba.se.impl.activation.ORBD\"
endif

include $(BUILDDIR)/common/Program.gmk

