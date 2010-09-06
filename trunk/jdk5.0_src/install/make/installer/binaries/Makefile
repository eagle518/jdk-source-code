#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.1 02/09/30
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
	$(CD) $(PLATFORM) ; $(MAKE) all

clean::
	$(CD) $(PLATFORM) ; $(MAKE) clean

#
# Phonies to avoid accidents.
#
.PHONY: all clean 
