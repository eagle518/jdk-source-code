# 
# @(#)profiled.make	1.11 03/12/23 16:35:28
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Sets make macros for making profiled version of Gamma VM
# (It is also optimized.)

CFLAGS += -pg

# On x86 Solaris 2.6, 7, and 8 if LD_LIBRARY_PATH has /usr/lib in it then
# adlc linked with -pg puts out empty header files. To avoid linking adlc
# with -pg the profile flag is split out separately and used in rules.make

PROF_AOUT_FLAGS += -pg

SYSDEFS += $(REORDER_FLAG)

# To do a profiled build of the product, such as for generating the
# reordering file, set PROFILE_PRODUCT.  Otherwise the reordering file will
# contain references to functions which are not defined in the PRODUCT build.

ifdef PROFILE_PRODUCT
  SYSDEFS += -DPRODUCT
endif

