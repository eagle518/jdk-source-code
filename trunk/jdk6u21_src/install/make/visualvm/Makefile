#
# Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.2 10/03/23
#
#
# Most frequently used targets:
#
#    all            --  extracts visualvm binaries which must be
#                       done before update to get the binaries into
#                       installer correctly.
INSTALL_BUILDDIR=..

include $(INSTALL_BUILDDIR)/common/Defs.gmk

all: extract-visualvm 


# dummy targets, all cleaning/clobbering will be done in binaries
clean:
clobber:

include $(INSTALL_BUILDDIR)/common/Binaries.gmk
#
# Phonies to avoid accidents.
#
.PHONY: all clean clobber extract-visualvm
