#
# @(#)Makefile	1.6 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../..
PACKAGE = javax.sound
LIBRARY = jsoundds
PRODUCT = sun
CPLUSPLUSLIBRARY = true
include $(BUILDDIR)/common/Defs.gmk

# this Makefile compiles the native files for DirectSound support in java sound

# include defines for sound
include ../SoundDefs.gmk

#
# Files
#

FILES_c = \
	$(CTARGDIR)Utilities.c \
	$(DAUDIOFILES_c)

FILES_cpp = \
	$(CTARGDIR)PLATFORM_API_WinOS_DirectSound.cpp

FILES_export = \
	$(DAUDIOFILES_export)

LDLIBS += dsound.lib
CPPFLAGS += -DUSE_DAUDIO=TRUE 


#
# Include rules
#
include $(BUILDDIR)/common/Library.gmk
