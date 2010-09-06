# 
# @(#)i486.make	1.22 04/03/05 10:10:06
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Must also specify if CPU is little endian
CFLAGS += -DVM_LITTLE_ENDIAN

# TLS helper, assembled from .s file
# Not included in includeDB because it has no dependencies
Obj_Files += solaris_i486.o

#
# Special case flags for compilers and compiler versions on i486.
#
ifeq ("${Platform_compiler}", "sparcWorks")

# _lwp_create_interpose must have a frame
OPT_CFLAGS/os_solaris_i486.o = -xO1
# force C++ interpreter to be full optimization
OPT_CFLAGS/interpret.o = -fast -O4
else

ifeq ("${Platform_compiler}", "gcc")
# gcc
# _lwp_create_interpose must have a frame
OPT_CFLAGS/os_solaris_i486.o = -fno-omit-frame-pointer
# force C++ interpreter to be full optimization
OPT_CFLAGS/interpret.o = -O3
#
else
# error
_JUNK2_ := $(shell echo >&2 \
       "*** ERROR: this compiler is not yet supported by this code base!")
	@exit 1
endif
endif
#
# Bug in ild causes it to fail randomly. Until we get a fix we can't
# use ild.
#
ILDFLAG/debug     = -xildoff
