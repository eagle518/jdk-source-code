#
# @(#)Makefile	1.34 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building javax
#

BUILDDIR = ..
include $(BUILDDIR)/common/Defs.gmk

# imageio uses xml, so build it last

SUBDIRS = \
	activation \
	accessibility \
	naming \
	print \
	swing \
	security \
	sound \
	xa \
	sql \
	rmi \
	management \
	tools \
	xml \
	imageio \
	script \
	annotation \
	smartcardio
all build clean clobber::
	$(SUBDIRS-loop)

