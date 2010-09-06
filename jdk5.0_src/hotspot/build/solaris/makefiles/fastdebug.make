# 
# @(#)fastdebug.make	1.22 03/12/23 16:35:25
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Sets make macros for making debug version of VM

# Compiler specific DEBUG_CFLAGS are passed in from gcc.make, sparcWorks.make
# They may also specify FASTDEBUG_CFLAGS, but it defaults to DEBUG_FLAGS.

FASTDEBUG_CFLAGS$(FASTDEBUG_CFLAGS) = $(DEBUG_CFLAGS)

# Compiler specific OPT_CFLAGS are passed in from gcc.make, sparcWorks.make
OPT_CFLAGS/DEFAULT= $(OPT_CFLAGS)
OPT_CFLAGS/BYFILE = $(OPT_CFLAGS/$@)$(OPT_CFLAGS/DEFAULT$(OPT_CFLAGS/$@))

ifeq ("${Platform_compiler}", "sparcWorks")
OPT_CFLAGS/SLOWER = -xO2
ifeq ($(shell expr $(COMPILER_REV) \>= 5.5), 1)
# CC 5.5 has bug 4908364 with -xO4 
OPT_CFLAGS/library_call.o = $(OPT_CFLAGS/SLOWER)
else
# Compilation of *_<arch>.cpp can take an hour or more at O3.  Use O2
# See comments at top of sparc.make.
OPT_CFLAGS/ad_$(Platform_arch).o = $(OPT_CFLAGS/SLOWER)
OPT_CFLAGS/dfa_$(Platform_arch).o = $(OPT_CFLAGS/SLOWER)
endif

ifeq (${COMPILER_REV}, 5.0)
# Avoid a compiler bug caused by using -xO<level> -g<level>
# Since the bug also occurs with -xO0, use an innocuous value (must not be null)
OPT_CFLAGS/c1_LIROptimizer_i486.o = -c
endif

ifeq ($(shell expr $(COMPILER_REV) \< 5.5), 1)
# Same problem with Solaris/x86 compiler (both 5.0 and 5.2) on ad_i486.cpp.
# CC build time is also too long for ad_i486_{gen,misc}.o
OPT_CFLAGS/ad_i486.o = -c
OPT_CFLAGS/ad_i486_gen.o = -c
OPT_CFLAGS/ad_i486_misc.o = -c
ifeq ($(Platform_arch), i486)
# Same problem for the wrapper roosts: jni.o jvm.o jvmdi.o
OPT_CFLAGS/jni.o = -c
OPT_CFLAGS/jvm.o = -c
OPT_CFLAGS/jvmdi.o = -c
# Same problem in parse2.o (probably the Big Switch over bytecodes)
OPT_CFLAGS/parse2.o = -c
endif # Platform_arch == i486
endif
endif # Platform_compiler == sparcWorks

# (OPT_CFLAGS/SLOWER is also available, to alter compilation of buggy files)

# If you set HOTSPARC_GENERIC=yes, you disable all OPT_CFLAGS settings
CFLAGS$(HOTSPARC_GENERIC) += $(OPT_CFLAGS/BYFILE)

# Set the environment variable HOTSPARC_GENERIC to "true"
# to inhibit the effect of the previous line on CFLAGS.

# The following lines are copied from debug.make, except that we
# consult FASTDEBUG_CFLAGS instead of DEBUG_CFLAGS.
# Compiler specific DEBUG_CFLAGS are passed in from gcc.make, sparcWorks.make
DEBUG_CFLAGS/DEFAULT= $(FASTDEBUG_CFLAGS)
DEBUG_CFLAGS/BYFILE = $(DEBUG_CFLAGS/$@)$(DEBUG_CFLAGS/DEFAULT$(DEBUG_CFLAGS/$@))
CFLAGS += $(DEBUG_CFLAGS/BYFILE)

# Linker mapfiles
MAPFILE = $(GAMMADIR)/build/solaris/makefiles/mapfile-vers \
	  $(GAMMADIR)/build/solaris/makefiles/mapfile-vers-debug \
          $(GAMMADIR)/build/solaris/makefiles/mapfile-vers-$(TYPE) \
	  $(GAMMADIR)/build/solaris/makefiles/mapfile-vers-nonproduct


G_SUFFIX =
VERSION = optimized
SYSDEFS += -DASSERT -DFASTDEBUG
PICFLAGS = DEFAULT
