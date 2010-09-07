#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# Makefile to build the JQS (Java Quick Starter) binary
#
#

DEPLOY_TOPDIR = ../../..

EXECUTABLE = jqs
PRODUCT = deploy
PROGRAM = jqs


#
# Source file directories
#

SRCDIR = $(DEPLOY_TOPDIR)/src/jqs/share \
	 $(DEPLOY_TOPDIR)/src/jqs/windows \
	 $(DEPLOY_TOPDIR)/src/jqs/windows/resources


#
# Include standard build definitions and environment settings
#

include $(DEPLOY_TOPDIR)/make/common/Defs.gmk

OBJDIR = $(OUTPUTDIR)/tmp/deploy/jqs/$(PROGRAM)


#
# Additions to standard build variables.
#

CPPINCPATH = $(SRCDIR)

EXTRA_CPPFLAGS := -Gm -EHsc -GS -DCRT_SECURE_NO_DEPRECATE -DJ2SE_BUILD_ID="\"$(FULL_VERSION)\""

LDLIBS += ws2_32.lib ole32.lib advapi32.lib


#
# Source file search paths and file lists
#

vpath %.cpp $(SRCDIR)
vpath %.rc $(SRCDIR)

include ./FileList.gmk 


#
# Include standard build targets
#

include $(DEPLOY_TOPDIR)/make/plugin/common/exec.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk


clobber::
	$(RM) -r $(OBJDIR)

.PHONY: all clean
