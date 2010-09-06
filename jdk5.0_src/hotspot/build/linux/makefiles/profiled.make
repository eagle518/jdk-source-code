# 
# @(#)profiled.make	1.4 03/12/23 16:35:20
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Sets make macros for making profiled version of Gamma VM
# (It is also optimized.)

CFLAGS += -pg
AOUT_FLAGS += -pg
