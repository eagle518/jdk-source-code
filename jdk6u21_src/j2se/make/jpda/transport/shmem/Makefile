#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# @(#)Makefile	1.22 10/03/23
#
# Makefile for building JDI shared memory transport implementation
#

BUILDDIR = ../../..
LIBRARY  = dt_shmem
PRODUCT = jbug

# Mapfile only used on Solaris/Linux
FILES_m = mapfile-vers

include $(BUILDDIR)/common/Defs.gmk

# 64-bit windows does not build at -W3 if warnings are fatal
ifeq ($(PLATFORM), windows)
  ifeq ($(ARCH_DATA_MODEL), 64)
    COMPILER_WARNINGS_FATAL=false
  endif
endif

#
# Has been converted to the JNI: generate JNI-style header files
#
JAVAHFLAGS += -jni

FILES_c = \
        $(CTARGDIR)SharedMemoryTransport.c \
        $(CTARGDIR)SharedMemoryConnection.c \
        $(CTARGDIR)shmemBack.c \
        $(CTARGDIR)shmemBase.c \
        $(CTARGDIR)shmem_md.c

OTHER_INCLUDES = -I$(INCLUDEDIR) -I$(PLATFORM_INCLUDE) \
                 -I$(SHARE_SRC)/transport/export \
                 -I$(SHARE_SRC)/transport/shmem \
                 -I$(SHARE_SRC)/back/export \
                 -I$(PLATFORM_SRC)/transport/shmem

#
# Stuff to feed into javah.
#
FILES_export = \
	com/sun/tools/jdi/SharedMemoryConnection.java \
	com/sun/tools/jdi/SharedMemoryTransportService.java

# Use the mapfile-vers (See the mapfile located with this Makefile)
ifdef FILES_m
  include $(BUILDDIR)/common/Mapfile-vers.gmk
endif

#
# Rules.
#
include $(BUILDDIR)/common/Library.gmk

# Add -export options to explicitly spell exported symbols
ifeq ($(PLATFORM), windows)
  OTHER_LCF += -export:jdwpTransport_OnLoad
endif

#
# vpaths for the transport (should not be seen by others)
#
vpath %.c $(SHARE_SRC)/native/com/sun/tools/jdi:$(SHARE_SRC)/transport/shmem:$(PLATFORM_SRC)/transport/shmem
