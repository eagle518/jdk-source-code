#
# Copyright (c) 2006, 2008, Oracle and/or its affiliates. All rights reserved.
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

# Sets make macros for making tiered version of VM

TYPE=TIERED

VM_SUBDIR = server

CFLAGS += -DCOMPILER2 -DCOMPILER1
