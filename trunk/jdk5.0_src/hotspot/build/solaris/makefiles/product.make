# 
# @(#)product.make	1.16 03/12/23 16:35:28
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Sets make macros for making optimized version of Gamma VM
# (This is the "product", not the "release" version.)

# Compiler specific OPT_CFLAGS are passed in from gcc.make, sparcWorks.make
OPT_CFLAGS/DEFAULT= $(OPT_CFLAGS)
OPT_CFLAGS/BYFILE = $(OPT_CFLAGS/$@)$(OPT_CFLAGS/DEFAULT$(OPT_CFLAGS/$@))

# (OPT_CFLAGS/SLOWER is also available, to alter compilation of buggy files)

# If you set HOTSPARC_GENERIC=yes, you disable all OPT_CFLAGS settings
CFLAGS$(HOTSPARC_GENERIC) += $(OPT_CFLAGS/BYFILE)
# Set the environment variable HOTSPARC_GENERIC to "true"
# to inhibit the effect of the previous line on CFLAGS.

# Linker mapfiles
# NOTE: inclusion of nonproduct mapfile not necessary; read it for details
MAPFILE = $(GAMMADIR)/build/solaris/makefiles/mapfile-vers \
          $(GAMMADIR)/build/solaris/makefiles/mapfile-vers-$(TYPE) \
          $(GAMMADIR)/build/solaris/makefiles/mapfile-vers-nonproduct

# From top level Makefile, not exported ---
OS		= $(subst SunOS,solaris,$(shell uname))
ifdef LP64
ARCH		= sparcv9
else
ARCH		= $(subst i386,i486,$(shell uname -p))
endif

REORDERFILE = $(GAMMADIR)/build/solaris/makefiles/reorder_$(TYPE)_$(ARCH)

# Take a platform-specific action to strip libjvm.so to its minimum size.
LINK_LIB.CC/POST_HOOK += $(STRIP_LIB.CC/POST_HOOK)

G_SUFFIX =
SYSDEFS += -DPRODUCT
SYSDEFS += $(REORDER_FLAG)
VERSION = optimized
