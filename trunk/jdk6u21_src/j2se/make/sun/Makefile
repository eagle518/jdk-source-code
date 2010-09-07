#
# @(#)Makefile	1.116 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building all of sun tools
#

BUILDDIR = ..
PRODUCT  = sun
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
  TOOLS_SUBDIRS =
else
  ifeq ($(PLATFORM), windows)
    TOOLS_SUBDIRS = javac javap native2ascii serialver hprof jps jstat jstatd
  else # PLATFORM
    TOOLS_SUBDIRS = javap native2ascii serialver hprof jps jstat jstatd
  endif # PLATFORM
  ifeq ($(INCLUDE_SA), true)
    TOOLS_SUBDIRS += jsadebugd
  endif
  TOOLS_SUBDIRS += jstack jmap jinfo jconsole jrunscript jhat
endif # J2RE_ONLY

HAVE_DGA     = true
ifeq ($(PLATFORM), windows)
  HAVE_DGA   = false
endif
ifeq ($(PLATFORM), solaris)
  ifeq ($(ARCH), amd64)
    HAVE_DGA = false
  endif
endif 

ifeq ($(HAVE_DGA),true)
DGA_SUBDIR = jdga
endif

ORG_EXISTS := $(call DirExists,$(SHARE_SRC)/classes/sun/org,,)

ifneq ("$(ORG_EXISTS)", "")
  ORG_SUBDIR = org
endif

#
# corba must always be built before rmi, due to rmi-iiop support
#

ifeq ($(PLATFORM), windows)
SUBDIRS = security jar javazic dcpr image awt splashscreen font jpeg cmm \
	  audio net applet misc jkernel corba rmi jdbc beans jawt text nio \
	  management jvmstat $(ORG_SUBDIR) xjc schemagen wsgen wsimport $(TOOLS_SUBDIRS)
else # PLATFORM
SUBDIRS = security jar javazic misc jkernel net audio dcpr image awt splashscreen xawt motif12 motif21 xjc schemagen wsgen wsimport \
          headless $(DGA_SUBDIR) font jpeg cmm applet corba rmi beans \
          jdbc jawt text nio management jvmstat $(ORG_SUBDIR) $(TOOLS_SUBDIRS)
endif # PLATFORM

all build clean clobber::
	$(SUBDIRS-loop)

