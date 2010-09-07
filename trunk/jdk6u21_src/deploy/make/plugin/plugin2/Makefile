#
# @(#)Makefile	1.6 10/03/24
#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#

DEPLOY_TOPDIR = ../../..
include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

ifeq ($(PLATFORM), windows)
  SUBDIRS += jp2native jp2iexp npjp2 jp2ssv jp2launcher
endif
ifeq ($(PLATFORM), solaris)
  SUBDIRS += npjp2
endif
ifeq ($(PLATFORM), linux)
  SUBDIRS += npjp2
endif

all build clobber:: $(SUBDIRS)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

#
# Phonies to avoid accidents.
#
.PHONY: all build clobber $(SUBDIRS)
