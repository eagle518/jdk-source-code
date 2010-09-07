#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.8 10/03/24 
#
#
# Makefile to build OJI plugin subdirectories
#
# Build each subdirectory

DEPLOY_TOPDIR = ../../..
include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

ifeq ($(PLATFORM), windows)
  SUBDIRS += win32
else
  SUBDIRS += unix
endif

ifeq ($(PLATFORM), linux)
  SUBDIRS += linux-gcc29
endif

all build clobber:: $(SUBDIRS)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

#
# Phonies to avoid accidents.
#
.PHONY: all build clobber $(SUBDIRS)
