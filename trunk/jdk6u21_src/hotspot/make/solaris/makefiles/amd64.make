#
# Copyright (c) 2004, 2008, Oracle and/or its affiliates. All rights reserved.
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

# Must also specify if CPU is little endian
CFLAGS += -DVM_LITTLE_ENDIAN

# Not included in includeDB because it has no dependencies
Obj_Files += solaris_x86_64.o

#
# Special case flags for compilers and compiler versions on amd64.
#
ifeq ("${Platform_compiler}", "sparcWorks")

# Temporary until C++ compiler is fixed

# _lwp_create_interpose must have a frame
OPT_CFLAGS/os_solaris_x86_64.o = -xO1

# Temporary until SS10 C++ compiler is fixed
OPT_CFLAGS/generateOptoStub.o = -xO2
OPT_CFLAGS/thread.o = -xO2

else

ifeq ("${Platform_compiler}", "gcc")
# gcc
# The serviceability agent relies on frame pointer (%rbp) to walk thread stack
CFLAGS += -fno-omit-frame-pointer

else
# error
_JUNK2_ := $(shell echo >&2 \
       "*** ERROR: this compiler is not yet supported by this code base!")
       @exit 1
endif
endif
