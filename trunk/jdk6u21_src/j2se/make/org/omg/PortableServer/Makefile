#
# @(#)Makefile	1.29 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..

PACKAGE = org.omg.PortableServer
PRODUCT = org

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#

CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)org_omg_PortableServer.jmk


# order of building files is important

FILES_java = $(PortableServer_java) $(POA_GENERATED_java)

include $(BUILDDIR)/common/Classes.gmk
