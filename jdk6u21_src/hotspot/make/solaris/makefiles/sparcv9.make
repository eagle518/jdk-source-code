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

Obj_Files += solaris_sparc.o
ASFLAGS += $(AS_ARCHFLAG)

ifeq ("${Platform_compiler}", "sparcWorks")
ifeq ($(shell expr $(COMPILER_REV_NUMERIC) \< 505), 1)
# When optimized fully, stubGenerator_sparc.cpp 
# has bogus code for the routine 
# StubGenerator::generate_flush_callers_register_windows() 
OPT_CFLAGS/stubGenerator_sparc.o = $(OPT_CFLAGS/SLOWER)

# For now ad_sparc file is compiled with -O2 %%%% remove when adlc is fixed
OPT_CFLAGS/ad_sparc.o = $(OPT_CFLAGS/SLOWER)
OPT_CFLAGS/dfa_sparc.o = $(OPT_CFLAGS/SLOWER)

# CC brings an US-II to its knees compiling the vmStructs asserts under -xO4
OPT_CFLAGS/vmStructs.o = $(OPT_CFLAGS/O2)
endif

else
#Options for gcc
OPT_CFLAGS/stubGenerator_sparc.o = $(OPT_CFLAGS/SLOWER)
OPT_CFLAGS/ad_sparc.o = $(OPT_CFLAGS/SLOWER)
OPT_CFLAGS/dfa_sparc.o = $(OPT_CFLAGS/SLOWER)
OPT_CFLAGS/vmStructs.o = $(OPT_CFLAGS/O2)
endif
