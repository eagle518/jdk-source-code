#
# Copyright (c) 1999, 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#  
#

#------------------------------------------------------------------------
# CC, CPP & AS

CPP = CC
CC  = cc
AS  = $(CC) -c

ARCHFLAG = $(ARCHFLAG/$(BUILDARCH))
ARCHFLAG/i486    = -m32
ARCHFLAG/amd64   = -m64

CFLAGS     += $(ARCHFLAG)
AOUT_FLAGS += $(ARCHFLAG)
LFLAGS     += $(ARCHFLAG)
ASFLAGS    += $(ARCHFLAG)

#------------------------------------------------------------------------
# Compiler flags

# position-independent code
PICFLAG = -KPIC

CFLAGS += $(PICFLAG)
# no more exceptions
CFLAGS += -features=no%except
# Reduce code bloat by reverting back to 5.0 behavior for static initializers
CFLAGS += -features=no%split_init
# allow zero sized arrays
CFLAGS += -features=zla

# Use C++ Interpreter
ifdef CC_INTERP
  CFLAGS += -DCC_INTERP
endif

# We don't need libCstd.so and librwtools7.so, only libCrun.so
CFLAGS += -library=Crun
LIBS += -lCrun

CFLAGS += -mt
LFLAGS += -mt

# Compiler warnings are treated as errors
#WARNINGS_ARE_ERRORS = -errwarn=%all
CFLAGS_WARN/DEFAULT = $(WARNINGS_ARE_ERRORS) 
# Special cases
CFLAGS_WARN/BYFILE = $(CFLAGS_WARN/$@)$(CFLAGS_WARN/DEFAULT$(CFLAGS_WARN/$@)) 

# The flags to use for an Optimized build
OPT_CFLAGS+=-xO4
OPT_CFLAGS/NOOPT=-xO0

#------------------------------------------------------------------------
# Linker flags

# Use $(MAPFLAG:FILENAME=real_file_name) to specify a map file.
MAPFLAG = -Wl,--version-script=FILENAME

# Use $(SONAMEFLAG:SONAME=soname) to specify the intrinsic name of a shared obj
SONAMEFLAG = -h SONAME

# Build shared library
SHARED_FLAG = -G

#------------------------------------------------------------------------
# Debug flags
DEBUG_CFLAGS += -g
FASTDEBUG_CFLAGS = -g0

