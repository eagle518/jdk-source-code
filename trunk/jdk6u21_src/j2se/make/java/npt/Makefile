#
# @(#)Makefile	1.11 10/03/23
# 
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../..
LIBRARY = npt
PRODUCT = sun
LIBRARY_OUTPUT = npt

# Configure the CFLAGS for this library.

FILES_m = mapfile-vers

include $(BUILDDIR)/common/Defs.gmk

SRCDIR=$(SHARE_SRC)/npt
PSRCDIR=$(PLATFORM_SRC)/npt

# Use the mapfile-vers (See the mapfile located with this Makefile)
ifdef FILES_m
include $(BUILDDIR)/common/Mapfile-vers.gmk
endif

#
# Files to compile.
#
FILES_c = 			\
	$(CTARGDIR)npt.c 	\
	$(CTARGDIR)utf.c 	\
	$(CTARGDIR)utf_md.c

OTHER_INCLUDES = -I$(SRCDIR) -I$(PSRCDIR)

ifeq ($(PLATFORM), solaris)
  # Use of lint has been helpful, but lots of errors needed to be suppressed:
  OTHER_LINTFLAGS += -errtags=yes
  # We only care about errors in user include files
  OTHER_LINTFLAGS += -errhdr=%user
  OTHER_LINTFLAGS += -erroff=E_CAST_INT_TO_SMALL_INT
  # Logging gives lint fits, so we turn it off
  OTHER_LINTFLAGS += -DNDEBUG
  # We don't use the -Ncheck feature yet, it seems somewhat broken
  #OTHER_LINTFLAGS += -Ncheck -Nlevel=2
  # Even with above suppression, some files/code are hopelessly non-lintable
  #   socket.h has some old style C function prototypes in it (Solaris 9u2)
  MY_LINT_FILTER += | $(GREP) -v /usr/include
endif # PLATFORM

#
# This removes all asserts in the optimized version
#
CPPFLAGS_OPT += -DNDEBUG

#
# Library to compile.
#
include $(BUILDDIR)/common/Library.gmk

# We don't want to link against -ljava
JAVALIB=

# Add -export options to explicitly spell exported symbols
ifeq ($(PLATFORM), windows)
  OTHER_LCF += -export:nptInitialize -export:nptTerminate
endif

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SRCDIR):$(PSRCDIR)

#
# Lint checking rules (we filter out the lint pass and only list errors)
#
linterrs: $(TEMPDIR)/linterrs
$(TEMPDIR)/linterrs: $(FILES_o)
	$(RM) $@
	$(MAKE) lint.clean
	$(MAKE) lint.errors > $@ 2>&1
	($(CAT) $@ | $(GREP) 'warning:' $(MY_LINT_FILTER) || exit 0)

