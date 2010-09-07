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

# Sets make macros for making profiled version of Gamma VM
# (It is also optimized.)

CFLAGS += -pg
AOUT_FLAGS += -pg
LDNOMAP = true
