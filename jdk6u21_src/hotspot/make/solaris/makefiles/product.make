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

# Sets make macros for making optimized version of Gamma VM
# (This is the "product", not the "release" version.)

# Compiler specific OPT_CFLAGS are passed in from gcc.make, sparcWorks.make
OPT_CFLAGS/DEFAULT= $(OPT_CFLAGS)
OPT_CFLAGS/BYFILE = $(OPT_CFLAGS/$@)$(OPT_CFLAGS/DEFAULT$(OPT_CFLAGS/$@))

# Workaround for a bug in dtrace.  If ciEnv::post_compiled_method_load_event()
# is inlined, the resulting dtrace object file needs a reference to this
# function, whose symbol name is too long for dtrace.  So disable inlining
# for this method for now. (fix this when dtrace bug 6258412 is fixed)
ifndef USE_GCC
OPT_CFLAGS/ciEnv.o = $(OPT_CFLAGS) -xinline=no%__1cFciEnvbFpost_compiled_method_load_event6MpnHnmethod__v_
endif

# (OPT_CFLAGS/SLOWER is also available, to alter compilation of buggy files)
ifeq ("${Platform_compiler}", "sparcWorks")

# Problem with SS12 compiler, dtrace doesn't like the .o files  (bug 6693876)
ifeq ($(COMPILER_REV_NUMERIC),509)
  # Not clear this workaround could be skipped in some cases.
  OPT_CFLAGS/vmGCOperations.o = $(OPT_CFLAGS/SLOWER) -g
  OPT_CFLAGS/java.o = $(OPT_CFLAGS/SLOWER) -g
  OPT_CFLAGS/jni.o = $(OPT_CFLAGS/SLOWER) -g
endif

# Workaround SS11 bug 6345274 (all platforms) (Fixed in SS11 patch and SS12)
ifeq ($(COMPILER_REV_NUMERIC),508)
OPT_CFLAGS/ciTypeFlow.o = $(OPT_CFLAGS/O2)
endif # COMPILER_REV_NUMERIC == 508

endif # Platform_compiler == sparcWorks

# If you set HOTSPARC_GENERIC=yes, you disable all OPT_CFLAGS settings
CFLAGS$(HOTSPARC_GENERIC) += $(OPT_CFLAGS/BYFILE)
# Set the environment variable HOTSPARC_GENERIC to "true"
# to inhibit the effect of the previous line on CFLAGS.

# Linker mapfiles
# NOTE: inclusion of nonproduct mapfile not necessary; read it for details
ifdef USE_GCC
MAPFILE = $(GAMMADIR)/make/solaris/makefiles/mapfile-vers
else
MAPFILE = $(GAMMADIR)/make/solaris/makefiles/mapfile-vers \
          $(GAMMADIR)/make/solaris/makefiles/mapfile-vers-nonproduct

# This mapfile is only needed when compiling with dtrace support, 
# and mustn't be otherwise.
MAPFILE_DTRACE = $(GAMMADIR)/make/solaris/makefiles/mapfile-vers-$(TYPE)

REORDERFILE = $(GAMMADIR)/make/solaris/makefiles/reorder_$(TYPE)_$(BUILDARCH)
endif

# Don't strip in VM build; JDK build will strip libraries later
# LINK_LIB.CC/POST_HOOK += $(STRIP_LIB.CC/POST_HOOK)

G_SUFFIX =
SYSDEFS += -DPRODUCT
SYSDEFS += $(REORDER_FLAG)
VERSION = optimized
