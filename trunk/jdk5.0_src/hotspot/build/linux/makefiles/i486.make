# 
# @(#)i486.make	1.13 04/04/27 15:40:36
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# TLS helper, assembled from .s file
# Not included in includeDB because it has no dependencies
Obj_Files += linux_i486.o

# The copied fdlibm routines in sharedRuntimeTrig.o must not be optimized
OPT_CFLAGS/sharedRuntimeTrig.o = $(OPT_CFLAGS/NOOPT)
# Must also specify if CPU is little endian
CFLAGS += -DVM_LITTLE_ENDIAN

CFLAGS += -march=i586
