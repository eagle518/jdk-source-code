#
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
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
# 
# Sets make macros for making kernel version of VM.
# This target on solaris is just tempoarily for debugging the kernel build.

TYPE=KERNEL

VM_SUBDIR = client

CFLAGS += -DKERNEL
