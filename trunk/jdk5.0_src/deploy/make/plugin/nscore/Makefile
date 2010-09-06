#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.3 03/12/19 
#
#
# Makefile to build OJI plugin subdirectories
#
# Build each subdirectory

PLUGIN_TOP = ../../..
include $(PLUGIN_TOP)/make/plugin/common/config.gmk

ifeq ($(PLATFORM), windows)
  SUBDIRS += win32
else
  SUBDIRS += unix
endif

ifeq ($(PLATFORM), linux)
  SUBDIRS += linux-gcc29
endif

all optimized debug clobber:: $(SUBDIRS)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber $(SUBDIRS)
