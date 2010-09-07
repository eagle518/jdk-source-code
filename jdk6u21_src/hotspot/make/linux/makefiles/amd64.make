#
# Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
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

# Not included in includeDB because it has no dependencies
Obj_Files += linux_x86_64.o

# The copied fdlibm routines in sharedRuntimeTrig.o must not be optimized
OPT_CFLAGS/sharedRuntimeTrig.o = $(OPT_CFLAGS/NOOPT)
# The copied fdlibm routines in sharedRuntimeTrans.o must not be optimized
OPT_CFLAGS/sharedRuntimeTrans.o = $(OPT_CFLAGS/NOOPT)
# Must also specify if CPU is little endian
CFLAGS += -DVM_LITTLE_ENDIAN

CFLAGS += -D_LP64=1

# The serviceability agent relies on frame pointer (%rbp) to walk thread stack
ifndef USE_SUNCC
  CFLAGS += -fno-omit-frame-pointer
endif

OPT_CFLAGS/compactingPermGenGen.o = -O1
