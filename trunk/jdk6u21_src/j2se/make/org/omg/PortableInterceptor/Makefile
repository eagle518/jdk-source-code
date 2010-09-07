#
# @(#)Makefile	1.10 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..

PACKAGE = org.omg.PortableInterceptor
PRODUCT = org

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)org_omg_Messaging.jmk
include $(CORBA_JMK_DIRECTORY)org_omg_IOP.jmk
include $(CORBA_JMK_DIRECTORY)org_omg_CORBAX.jmk
include $(CORBA_JMK_DIRECTORY)org_omg_PortableInterceptor.jmk

FILES_java = $(PI_GENERATED_java) $(Messaging_java) $(IOP_java) $(CORBA_java)

#
# Include
#

include $(BUILDDIR)/common/Classes.gmk

