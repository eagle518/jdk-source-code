#
# @(#)Makefile	1.100 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of java
#

PRODUCT=java
BUILDDIR=..
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
  TOOLS_SUBDIRS = 
else
  TOOLS_SUBDIRS = javac javah
endif

#
# The order of subdirs here is important -- we want to bootstrap as
# quickly as possible.
#

# Bootstrap
BOOTSUBDIRS=hpi version jvm redist verify fdlibm java sun_nio jli main \
	zip $(TOOLS_SUBDIRS)

# Others
#    Note: java_crw_demo java_hprof_demo are demos but must be delivered built in sdk
SUBDIRS    =$(BOOTSUBDIRS) security \
	    npt \
	    java_crw_demo java_hprof_demo \
	    math awt util text applet net nio \
	    sql rmi jar beans logging management instrument


ifeq ($(PLATFORM), solaris)
  ifeq ($(ARCH_DATA_MODEL), 32)
    SUBDIRS += jexec
  endif
endif # PLATFORM

ifeq ($(PLATFORM), linux)
  SUBDIRS += jexec
endif # PLATFORM

all build clean clobber::
	$(SUBDIRS-loop)

