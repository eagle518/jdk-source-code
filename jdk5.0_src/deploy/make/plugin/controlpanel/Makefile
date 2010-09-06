#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.5 03/12/19
#
#
# Makefile to build control panel subdirectories
#
# Build each subdirectory

PLUGIN_TOP = ../../..
include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), windows)
  SUBDIRS += cpl cplexec
else
  SUBDIRS += cplscript
endif


all optimized debug clobber:: $(SUBDIRS)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber $(SUBDIRS)
