#
# Copyright (c) 2005, 2008, Oracle and/or its affiliates. All rights reserved.
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
Obj_Files += linux_sparc.o

# gcc 4.0 miscompiles this code in -m64
OPT_CFLAGS/macro.o = -O0

CFLAGS += -D_LP64=1
