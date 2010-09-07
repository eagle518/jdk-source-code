#
# @(#)Makefile	1.5 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Build rmi tools, and sun.rmi.* implementation classes
#
BUILDDIR = ../..
include $(BUILDDIR)/common/Defs.gmk

ifdef J2RE_ONLY
TOOLS_SUBDIRS = 
else
TOOLS_SUBDIRS = oldtools rmic
endif

LOG_GENDIRECTORY  = $(GENSRCDIR)/com/sun/corba/se/impl/logging
LOG_CLASS_OUTPUTDIRECTORY = $(CLASSBINDIR)/com/sun/corba/se/impl/logging
LOGSTRINGS_PROPERTIES     = LogStrings.properties
#
# Order is important -- you can't build stubs without building rmic first.
# We need corbalogsources to be built first, since it builds LogWrappers to
# log the exceptions and generate the right minorcodes. These classes are
# references in com/sun/corba/se/impl/io and util classes which in turn will
# be built from rmic Makefile.
#
CORBALOG_SUBDIRS = corbalogsources corbalogcompile
ifdef STANDALONE_CORBA_WS
SUBDIRS = rmic
else
SUBDIRS = $(TOOLS_SUBDIRS) rmi registry rmid
ifeq ($(PLATFORM), windows)
SUBDIRS += cgi
endif # PLATFORM
endif # STANDALONE_CORBA_WS

# Do these subdirs first
OTHERSUBDIRS=$(CORBALOG_SUBDIRS)
all build clean clobber ::
	$(OTHERSUBDIRS-loop)

$(LOG_CLASS_OUTPUTDIRECTORY)/$(LOGSTRINGS_PROPERTIES):  $(LOG_GENDIRECTORY)/$(LOGSTRINGS_PROPERTIES)
	$(install-file)

# Then do the rest
all build clean clobber :: $(LOG_CLASS_OUTPUTDIRECTORY)/$(LOGSTRINGS_PROPERTIES)
	$(SUBDIRS-loop)

clean::
	$(RM) -r $(CLASSBINDIR)/sun/rmi $(CLASSBINDIR)/com/sun/rmi

