# 
# @(#)gcc.make	1.13 03/12/23 16:35:25
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# put the *.o files in the a.out, not the shared library
LINK_INTO = $(LINK_INTO/$(VERSION))
LINK_INTO/debug     = AOUT
LINK_INTO/optimized = LIBJVM

# apply this setting to link into the shared library even in the debug version:
LINK_INTO = LIBJVM

# If MAKE_OPTO is null, then we will compile with the -DOPTO.
# That switch, despite its name, has the effect of __disabling__ OPTO.
$(MAKE_OPTO)CFLAGS += -DOPTO

# no warnings till we get passed the errors!
CFLAGS += -w

PICFLAG = -fPIC

CFLAGS += $(PICFLAG)
CFLAGS += -fcheck-new
CFLAGS += -Winline
CFLAGS += -fwritable-strings
CFLAGS += -fno-rtti
CFLAGS += -fno-exceptions
CFLAGS += -D__STDC__=0 -D_LARGEFILE64_SOURCE
# -DTEMPLATE_TABLE_BUG not needed in g++ 2.95.2
CFLAGS += -D__GNU__ -D_REENTRANT

ifdef CC_INTERP
# C++ Interpreter
CFLAGS += -DCC_INTERP
endif

# Set GCC_SAVE_TEMPS=-save-temps to keep temporary files (.ii, .s).
CFLAGS += $(GCC_SAVE_TEMPS)

OPT_CFLAGS += -O3
OPT_CFLAGS/NOOPT=-O0

# Set the environment variable HOTSPARC_HOTSPARC_GENERIC to "true"
# to inhibit the effect of the previous line on CFLAGS.

CPP = g++
CC  = gcc

AOUT_FLAGS += -v
DEBUG_CFLAGS += -g
LFLAGS_VM = -shared -mimpure-text 
