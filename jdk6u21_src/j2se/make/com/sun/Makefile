#
# @(#)Makefile	1.57 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building com/sun
#

BUILDDIR = ../..
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

ORG_EXISTS := $(call DirExists,$(SHARE_SRC)/classes/sun/org,,)

ifneq ("$(ORG_EXISTS)", "") 
  SCRIPT_SUBDIR = script
endif

# Omit mirror since it's built with the apt tool.
SUBDIRS = $(SCRIPT_SUBDIR) image security jndi jmx corba java inputmethods java_cup org java2d xml rowset net demo servicetag

TOOLS_SUBDIRS = extcheck tools jarsigner

ifndef J2RE_ONLY
  SUBDIRS += $(TOOLS_SUBDIRS)
endif

all build clean clobber::
	$(SUBDIRS-loop)
