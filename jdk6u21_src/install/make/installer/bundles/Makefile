#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#  @(#)Makefile	1.8 10/03/23
#
#
# Most frequently used targets:
#
#    all            -- builds
#    clobber        -- clobber the builds
#

INSTALL_BUILDDIR=../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

all:
	if [ -r $(PLATFORM) ]; then \
	   ( $(CD) $(PLATFORM) ; $(MAKE) all ); \
	fi

clean::
	if [ -r $(PLATFORM) ]; then \
	   ( $(CD) $(PLATFORM) ; $(MAKE) clean ); \
	fi

#
# Phonies to avoid accidents.
#
.PHONY: all clean 
