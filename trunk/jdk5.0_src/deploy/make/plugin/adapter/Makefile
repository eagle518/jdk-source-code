#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.1 02/11/06
#
#
# Makefile to build OJI plugin subdirectories
#
# Build each subdirectory

PLUGIN_TOP = ../../..
include $(PLUGIN_TOP)/make/plugin/common/config.gmk

SUBDIRS += ns7-adapter

ifeq ($(PLATFORM), solaris)
  SUBDIRS += ns4-adapter
endif

ifeq ($(PLATFORM), linux)
  SUBDIRS += ns7-gcc29-adapter
endif

all optimized debug clobber:: $(SUBDIRS)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber $(SUBDIRS)
