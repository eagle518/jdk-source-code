# 
# @(#)amd64.make	1.4 04/04/28 19:22:17
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Not included in includeDB because it has no dependencies
Obj_Files += linux_amd64.o

# The copied fdlibm routines in sharedRuntimeTrig.o must not be optimized
OPT_CFLAGS/sharedRuntimeTrig.o = $(OPT_CFLAGS/NOOPT)
# Must also specify if CPU is little endian
CFLAGS += -DVM_LITTLE_ENDIAN

CFLAGS += -D_LP64=1

# The serviceability agent relies on frame pointer (%rbp) to walk thread stack
CFLAGS += -fno-omit-frame-pointer
