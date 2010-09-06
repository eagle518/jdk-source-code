# 
# @(#)sparcWorks.make	1.74 04/03/16 14:14:03
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Compiler-specific flags for sparcworks.

# tell make which C and C++ compilers to use
CC	= cc
CPP	= CC
AS	= /usr/ccs/bin/as

REORDER_FLAG = -xF

# Get the last thing on the line that looks like x.x+ (x is a digit).
COMPILER_REV := \
$(shell $(CPP) -V 2>&1 | sed -e 's/^.*\([1-9]\.[0-9][0-9]*\).*/\1/')

VALIDATED_COMPILER_REV := 5.5

ENFORCE_COMPILER_REV${ENFORCE_COMPILER_REV} := ${VALIDATED_COMPILER_REV}
ifneq (${COMPILER_REV},${ENFORCE_COMPILER_REV})
dummy_target_to_enforce_compiler_rev:
	@echo "Wrong ${CPP} version:  ${COMPILER_REV}. " \
	"Use version ${ENFORCE_COMPILER_REV}, or set" \
	"ENFORCE_COMPILER_REV=${COMPILER_REV}."
	@exit 1
endif


# Checking for the version of C compiler ($CC) used. 
# Get the last thing on the line that looks like x.x+ (x is a digit).
C_COMPILER_REV := \
$(shell $(CC) -V 2>&1 | grep -i "cc:" |  sed -e 's/^.*\([1-9]\.[0-9][0-9]*\).*/\1/')


VALIDATED_C_COMPILER_REV := 5.5

ENFORCE_C_COMPILER_REV${ENFORCE_C_COMPILER_REV} := ${VALIDATED_C_COMPILER_REV}
ifneq (${C_COMPILER_REV},${ENFORCE_C_COMPILER_REV})
dummy_target_to_enforce_c_compiler_rev:
	@echo "Wrong ${CC} version:  ${C_COMPILER_REV}. " \
	"Use version ${ENFORCE_C_COMPILER_REV}, or set" \
	"ENFORCE_C_COMPILER_REV=${C_COMPILER_REV}."
	@exit 1
endif

# Some interfaces (_lwp_create) changed with LP64 and Solaris 7
SOLARIS_7_OR_LATER := \
$(shell uname -r | awk -F. '{ if ($$2 >= 7) print "-DSOLARIS_7_OR_LATER"; }')
CFLAGS += ${SOLARIS_7_OR_LATER}

# Use these to work around compiler bugs:
OPT_CFLAGS/SLOWER=-xO3
OPT_CFLAGS/O2=-xO2
OPT_CFLAGS/NOOPT=-xO1

#################################################
# Begin current (>=5.5) Forte compiler options #
#################################################

ifeq ($(shell expr $(COMPILER_REV) \>= 5.5), 1)

ifeq ("${Platform_arch}", "sparc")

# To Build 64 Bit Sparc VM, define LP64
ifdef LP64
CFLAGS += -xarch=v9 
AOUT_FLAGS += -xarch=v9
LIB_FLAGS += -xarch=v9
LFLAGS += -xarch=v9
else # 32 bit either compiler1 or compiler2
# compiler2 can support v8plus
ifeq		($(TYPE),COMPILER2)
CFLAGS += -xarch=v8plus
AOUT_FLAGS += -xarch=v8plus
LIB_FLAGS += -xarch=v8plus
LFLAGS += -xarch=v8plus
else
# all other platforms assume v8
CFLAGS += -xarch=v8
AOUT_FLAGS += -xarch=v8
LIB_FLAGS += -xarch=v8
LFLAGS += -xarch=v8
endif

endif

# Flags for Optimization

# [phh] Commented out pending verification that we do indeed want
#       to potentially bias against u1 and u3 targets.
#CFLAGS += -xchip=ultra2

OPT_CFLAGS=-xO4 $(EXTRA_OPT_CFLAGS)

CFLAGS += $(GAMMADIR)/src/os_cpu/solaris_sparc/vm/solaris_sparc.il

endif # sparc

ifeq ("${Platform_arch}", "i486")

OPT_CFLAGS=-xtarget=pentium $(EXTRA_OPT_CFLAGS)

# UBE (CC 5.5) has bug 4923569 with -xO4
OPT_CFLAGS+=-xO3

CFLAGS += $(GAMMADIR)/src/os_cpu/solaris_i486/vm/solaris_i486.il

endif # i486

# no more exceptions
CFLAGS/NOEX=-features=no%except

# Reduce code bloat by reverting back to 5.0 behavior for static initializers
CFLAGS += -features=no%split_init

# PIC is safer for SPARC, and is considerably slower
# a file foo.o which wants to compile -pic can set "PICFLAG/foo.o = -PIC"
PICFLAG/DEFAULT = -KPIC
# [RGV] Need to figure which files to remove to get link to work
#PICFLAG/BETTER  = -pic
PICFLAG/BETTER  = $(PICFLAG/DEFAULT)
PICFLAG/BYFILE  = $(PICFLAG/$@)$(PICFLAG/DEFAULT$(PICFLAG/$@))

# We don't need libCstd.so and librwtools7.so, only libCrun.so
CFLAGS += -library=%none
LFLAGS += -library=%none

LFLAGS += -mt

endif	# COMPILER_REV >= VALIDATED_COMPILER_REV

######################################
# End 5.5 Forte compiler options     #
######################################

######################################
# Begin 5.2 Forte compiler options   #
######################################

ifeq ($(COMPILER_REV), 5.2)

ifeq ("${Platform_arch}", "sparc")

# To Build 64 Bit Sparc VM, define LP64
ifdef LP64
CFLAGS += -xarch=v9 
AOUT_FLAGS += -xarch=v9
LIB_FLAGS += -xarch=v9
LFLAGS += -xarch=v9
else # 32 bit either compiler1 or compiler2
# compiler2 can support v8plus
ifeq		($(TYPE),COMPILER2)
CFLAGS += -xarch=v8plus
AOUT_FLAGS += -xarch=v8plus
LIB_FLAGS += -xarch=v8plus
LFLAGS += -xarch=v8plus
else
# all other platforms assume v8
CFLAGS += -xarch=v8
AOUT_FLAGS += -xarch=v8
LIB_FLAGS += -xarch=v8
LFLAGS += -xarch=v8
endif

endif

# Flags for Optimization

# [phh] Commented out pending verification that we do indeed want
#       to potentially bias against u1 and u3 targets.
#CFLAGS += -xchip=ultra2

ifdef LP64
# SC5.0 tools on v9 are flakey at -xO4
# [phh] Is this still true for 6.1?
OPT_CFLAGS=-xO3 $(EXTRA_OPT_CFLAGS)
else
OPT_CFLAGS=-xO4 $(EXTRA_OPT_CFLAGS)
endif

CFLAGS += $(GAMMADIR)/src/os_cpu/solaris_sparc/vm/solaris_sparc.il

endif # sparc

ifeq ("${Platform_arch}", "i486")

OPT_CFLAGS=-xtarget=pentium $(EXTRA_OPT_CFLAGS)

# SC5.0 tools on x86 are flakey at -xO4
# [phh] Is this still true for 6.1?
OPT_CFLAGS+=-xO3

CFLAGS += $(GAMMADIR)/src/os_cpu/solaris_i486/vm/solaris_i486.il

endif # i486

# no more exceptions
CFLAGS/NOEX=-noex

# Reduce code bloat by reverting back to 5.0 behavior for static initializers
CFLAGS += -Qoption ccfe -one_static_init

# PIC is safer for SPARC, and is considerably slower
# a file foo.o which wants to compile -pic can set "PICFLAG/foo.o = -PIC"
PICFLAG/DEFAULT = -KPIC
# [RGV] Need to figure which files to remove to get link to work
#PICFLAG/BETTER  = -pic
PICFLAG/BETTER  = $(PICFLAG/DEFAULT)
PICFLAG/BYFILE  = $(PICFLAG/$@)$(PICFLAG/DEFAULT$(PICFLAG/$@))

# Would be better if these weren't needed, since we link with CC, but
# at present removing them causes run-time errors
LFLAGS += -library=Crun
LIBS   += -library=Crun -lCrun

endif	# COMPILER_REV >= VALIDATED_COMPILER_REV

##################################
# End 5.2 Forte compiler options #
##################################

##################################
# Begin old 5.1 compiler options #
##################################
ifeq ($(COMPILER_REV), 5.1)

_JUNK_ := $(shell echo >&2 \
       "*** ERROR: sparkWorks.make incomplete for 5.1 compiler")
	@exit 1
endif
##################################
# End old 5.1 compiler options   #
##################################

##################################
# Begin old 5.0 compiler options #
##################################

ifeq	(${COMPILER_REV}, 5.0)

# Had to hoist this higher apparently because of other changes. Must
# come before -xarch specification.
CFLAGS += -xtarget=native

# We are using SC5.0 but are we building in 32 or 64 bit mode?
# To Build 64 Bit Sparc VM, define LP64
ifdef LP64
CFLAGS += -xarch=v9
AOUT_FLAGS += -xarch=v9
LIB_FLAGS += -xarch=v9
LFLAGS += -xarch=v9
else  # 32 bit either compiler1 or compiler2
# compiler2 can support v8plus
ifeq		($(TYPE),COMPILER2)
CFLAGS += -xarch=v8plus
AOUT_FLAGS += -xarch=v8plus
LIB_FLAGS += -xarch=v8plus
LFLAGS += -xarch=v8plus
endif
endif

CFLAGS += -library=iostream
LFLAGS += -library=iostream  -library=Crun
LIBS += -library=iostream -library=Crun -lCrun

# Flags for Optimization
ifdef LP64
# SC5.0 tools on v9 are flakey at -xO4
OPT_CFLAGS=-xO3 $(EXTRA_OPT_CFLAGS)
else
OPT_CFLAGS=-xO4 $(EXTRA_OPT_CFLAGS)
endif

ifeq ("${Platform_arch}", "sparc")

CFLAGS += $(GAMMADIR)/src/os_cpu/solaris_sparc/vm/atomic_solaris_sparc.il

endif # sparc

ifeq ("${Platform_arch}", "i486")
OPT_CFLAGS=-xtarget=pentium $(EXTRA_OPT_CFLAGS)
ifeq ("${COMPILER_REV}", "5.0")
# SC5.0 tools on x86 are flakey at -xO4
OPT_CFLAGS+=-xO3
else
OPT_CFLAGS+=-xO4
endif

CFLAGS += $(GAMMADIR)/src/os_cpu/solaris_i486/vm/solaris_i486.il

endif  # i486

# The following options run into misaligned ldd problem (raj)
#OPT_CFLAGS = -fast -O4 -xarch=v8 -xchip=ultra

# no more exceptions
CFLAGS/NOEX=-noex

# PIC is safer for SPARC, and is considerably slower
# a file foo.o which wants to compile -pic can set "PICFLAG/foo.o = -PIC"
PICFLAG/DEFAULT = -PIC
# [RGV] Need to figure which files to remove to get link to work
#PICFLAG/BETTER  = -pic
PICFLAG/BETTER  = -PIC
PICFLAG/BYFILE  = $(PICFLAG/$@)$(PICFLAG/DEFAULT$(PICFLAG/$@))

endif	# COMPILER_REV = 5.0

################################
# End old 5.0 compiler options #
################################

ifeq ("${COMPILER_REV}", "4.2")
# 4.2 COMPILERS SHOULD NO LONGER BE USED
_JUNK_ := $(shell echo >&2 \
       "*** ERROR: SC4.2 compilers are not supported by this code base!")
	@exit 1
endif

# do not include shared lib path in a.outs
AOUT_FLAGS += -norunpath
LFLAGS_VM = -norunpath -z noversion -h $@

# need position-indep-code for shared libraries
# (ild appears to get errors on PIC code, so we'll try non-PIC for debug)
ifeq ($(PICFLAGS),DEFAULT)
PICFLAG/LIBJVM  = $(PICFLAG/DEFAULT)
else
PICFLAG/LIBJVM  = $(PICFLAG/BYFILE)
endif
PICFLAG/AOUT    =

PICFLAG = $(PICFLAG/$(LINK_INTO))
CFLAGS += $(PICFLAG)

# less dynamic linking (no PLTs, please)
#LIB_FLAGS += $(LINK_MODE)
# %%%%% despite -znodefs, -Bsymbolic gets link errors -- Rose

LINK_MODE = $(LINK_MODE/$(VERSION))
LINK_MODE/debug     =
LINK_MODE/optimized = -Bsymbolic -znodefs

# Have thread local errnos
ifeq ($(shell expr $(COMPILER_REV) \>= 5.5), 1)
CFLAGS += -mt
else
CFLAGS += -D_REENTRANT
endif

ifdef CC_INTERP
# C++ Interpreter
CFLAGS += -DCC_INTERP
endif

# Flags for Debugging
DEBUG_CFLAGS = -g
FASTDEBUG_CFLAGS = -g0
# The -g0 setting allows the C++ frontend to inline, which is a big win.

ifeq	(${COMPILER_REV}, 5.2)
COMPILER_DATE := $(shell $(CPP) -V 2>&1 | awk '{ print $$NF; }')
ifeq	(${COMPILER_DATE}, 2001/01/31)
# disable -g0 in fastdebug since SC6.1 dated 2001/01/31 seems to be buggy
# use an innocuous value because it will get -g if it's empty
FASTDEBUG_CFLAGS = -c
endif
endif

# Uncomment or 'gmake CFLAGS_BROWSE=-sbfast' to get source browser information.
# CFLAGS_BROWSE	= -sbfast
CFLAGS		+= $(CFLAGS_BROWSE)

# use ild when debugging (but when optimizing we want reproducible results)
ILDFLAG = $(ILDFLAG/$(VERSION))
ILDFLAG/debug     = -xildon
ILDFLAG/optimized =
AOUT_FLAGS += $(ILDFLAG)

# Where to put the *.o files (a.out, or shared library)?
LINK_INTO = $(LINK_INTO/$(VERSION))
LINK_INTO/debug = LIBJVM
LINK_INTO/optimized = LIBJVM

# We link the debug version into the a.out because:
#  1. ild works on a.out but not shared libraries, and using ild
#     can cut rebuild times by 25% for small changes.
#  2. dbx cannot gracefully set breakpoints in shared libraries
#

# apply this setting to link into the shared library even in the debug version:
ifdef LP64
LINK_INTO = LIBJVM
else
#LINK_INTO = LIBJVM
endif

MCS	= /usr/ccs/bin/mcs
STRIP	= /usr/ccs/bin/strip

# Solaris platforms collect lots of redundant file-ident lines,
# to the point of wasting a significant percentage of file space.
# (The text is stored in ELF .comment sections, contributed by
# all "#pragma ident" directives in header and source files.)
# This command "compresses" the .comment sections simply by
# removing repeated lines.  The data can be extracted from
# binaries in the field by using "mcs -p libjvm.so" or the older
# command "what libjvm.so".
LINK_LIB.CC/POST_HOOK += $(MCS) -c $@ || exit 1;
# (The exit 1 is necessary to cause a build failure if the command fails and
# multiple commands are strung together, and the final semicolon is necessary
# since the hook must terminate itself as a valid command.)

# Also, strip debug and line number information (worth about 1.7Mb).
STRIP_LIB.CC/POST_HOOK = $(STRIP) -x $@ || exit 1;
# STRIP_LIB.CC/POST_HOOK is incorporated into LINK_LIB.CC/POST_HOOK
# in certain configurations, such as product.make.  Other configurations,
# such as debug.make, do not include the strip operation.

# Enable "#pragma ident" directives.  They are conditionally compiled because
# redundant copies from header files can bloat the binaries on some platforms.
SYSDEFS += -DUSE_PRAGMA_IDENT_HDR -DUSE_PRAGMA_IDENT_SRC
