#
# @(#)Makefile	1.6 10/03/24
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

DEPLOY_TOPDIR = ../../../..

LIBRARY        = jp2native
SRCDIR         = $(DEPLOY_TOPDIR)/src/plugin/win32/plugin2/$(LIBRARY)

OTHER_LDLIBS   = user32.lib

include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

CPPINCPATH = $(SRCDIR)

RSCINC = -i $(SRCDIR) -i $(TEMPDIR) -i $(OBJDIR) -i $(subst $(LIBRARY),,$(TEMPDIR))

$(OBJDIR)/Windows_JNI.obj: EXTRA_CPPFLAGS :=  

vpath %.c $(SRCDIR)

vpath %.rc $(SRCDIR)

include ./FileList.gmk 

include $(J2SE_TOPDIR)/make/common/Library.gmk
include $(DEPLOY_TOPDIR)/make/plugin/common/rules.gmk

clean:: clobber

clobber::
	$(RM) $(OBJDIR)/*.obj
