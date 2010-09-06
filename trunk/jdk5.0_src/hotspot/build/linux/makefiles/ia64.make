# 
# @(#)ia64.make	1.18 03/12/23 16:35:18
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

#
# IA64 only uses c++ based interpreter
CFLAGS += -DCC_INTERP -D_LP64=1 -DVM_LITTLE_ENDIAN
# Hotspot uses very unstrict aliasing turn this optimization off
OPT_CFLAGS += -fno-strict-aliasing
ifeq ($(VERSION),debug)
ASM_FLAGS= -DDEBUG
else
ASM_FLAGS=
endif
# workaround gcc bug in compiling varargs
OPT_CFLAGS/jni.o = -O0

# gcc/ia64 has a bug that internal gcc functions linked with libjvm.so
# are made public. Hiding those symbols will cause undefined symbol error
# when VM is dropped into older JDK. We probably will need an IA64
# mapfile to include those symbols as a workaround. Disable linker mapfile 
# for now.
LDNOMAP=true
