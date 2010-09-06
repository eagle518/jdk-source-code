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
INSTALL_BUILDDIR=../../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

all: sanity-binaries post-sanity-binaries docs-tar binaries-jdk binaries-jdk-debug binaries-jre

binaries-clobber: binaries-jre-clobber binaries-jdk-clobber binaries-jdk-debug-clobber

clean:: binaries-clobber

include $(INSTALL_BUILDDIR)/common/Binaries.gmk
include $(INSTALL_BUILDDIR)/common/Sanity.gmk

#
# Phonies to avoid accidents.
#
.PHONY: all clean docs-tar binaries-jre binaries-jdk binaries-jdk-debug \
	binaries-jre-clobber binaries-jdk-clobber \
	binaries-jdk-debug-clobber
