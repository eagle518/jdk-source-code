#
# @(#)Makefile	1.31 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..

PACKAGE = org.omg.CORBA
PRODUCT = org

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/

include $(CORBA_JMK_DIRECTORY)org_omg_CORBA.jmk
include $(CORBA_JMK_DIRECTORY)org_omg_SendingContext.jmk
include $(CORBA_JMK_DIRECTORY)org_omg_CORBA_2_3.jmk


ifdef STANDALONE_CORBA_WS
	ORG_OMG_CORBA_java = $(org_omg_CORBA_java_1_2) $(org_omg_CORBA_java_1_3) $(org_omg_CORBA_java_new) $(org_omg_CORBA30_java) $(org_omg_CORBA_TSIdentification_java)
else
	ORG_OMG_CORBA_java = $(org_omg_CORBA_java_1_2) $(org_omg_CORBA_java_1_3) $(org_omg_CORBA_java_new) $(org_omg_CORBA30_java)
endif

FILES_java = \
        $(ORG_OMG_SENDINGCONTEXT_java) \
        $(ORG_OMG_CORBA_2_3_java) \
        $(ORG_OMG_CORBA_java)

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk

