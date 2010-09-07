# @(#)Makefile	1.1 05/03/18
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# Makefile to build DeployHelper plugin subdirectories
#
# Build each subdirectory

DEPLOY_TOPDIR = ../../..
include $(DEPLOY_TOPDIR)/make/plugin/common/Defs.gmk

SUBDIRS += mozilla iexplorer

all optimized debug clobber:: $(SUBDIRS)

$(SUBDIRS)::
	$(MAKE) -C $@ $(MAKECMDGOALS)

#
# Phonies to avoid accidents.
#
.PHONY: all optimized debug clobber $(SUBDIRS)
