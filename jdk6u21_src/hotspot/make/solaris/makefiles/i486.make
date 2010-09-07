#
# Copyright (c) 1998, 2008, Oracle and/or its affiliates. All rights reserved.
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

# TLS helper, assembled from .s file
# Not included in includeDB because it has no dependencies
Obj_Files += solaris_x86_32.o

#
# Special case flags for compilers and compiler versions on i486.
#
ifeq ("${Platform_compiler}", "sparcWorks")

# _lwp_create_interpose must have a frame
OPT_CFLAGS/os_solaris_x86.o = -xO1
else

ifeq ("${Platform_compiler}", "gcc")
# gcc
# _lwp_create_interpose must have a frame
OPT_CFLAGS/os_solaris_x86.o = -fno-omit-frame-pointer
#
else
# error
_JUNK2_ := $(shell echo >&2 \
       "*** ERROR: this compiler is not yet supported by this code base!")
	@exit 1
endif
endif

ifeq ("${Platform_compiler}", "sparcWorks")
# ILD is gone as of SS11 (5.8), not supported in SS10 (5.7)
ifeq ($(shell expr $(COMPILER_REV_NUMERIC) \< 507), 1)
  #
  # Bug in ild causes it to fail randomly. Until we get a fix we can't
  # use ild.
  #
  ILDFLAG/debug     = -xildoff
endif
endif
