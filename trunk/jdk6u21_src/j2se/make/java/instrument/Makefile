#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

# Makefile for building the Java Programming Language Instrumentation Services
# agent, supporting java.lang.instrument

BUILDDIR = ../..
PACKAGE = sun.instrument
LIBRARY = instrument
PRODUCT = sun

# Configure the CFLAGS for this library.  Use static binding so that there
# are not dependencies on modules not on the search patch when invoked from
# the Windows system directory (or elsewhere).
MS_RUNTIME_STATIC=true

FILES_m = mapfile-vers

include $(BUILDDIR)/common/Defs.gmk

# Use the mapfile-vers (See the mapfile located with this Makefile)
ifdef FILES_m
include $(BUILDDIR)/common/Mapfile-vers.gmk
endif

#
# Files to compile.
#
include FILES_c.gmk

#
# -jaragent support requires zip and jar manfiest parser to be compiled
# into the instrument library.
#
IO_PLATFORM_SRC = $(PLATFORM_SRC)/native/java/io
LAUNCHER_SHARE_SRC = $(SHARE_SRC)/bin
LAUNCHER_PLATFORM_SRC = $(PLATFORM_SRC)/bin

FILES_c += \
	canonicalize_md.c

FILES_export = \
	sun/instrument/InstrumentationImpl.java

OTHER_INCLUDES = -I$(SHARE_SRC)/instrument
OTHER_INCLUDES += -I$(PLATFORM_SRC)/instrument
OTHER_INCLUDES += -I$(IO_PLATFORM_SRC)
OTHER_INCLUDES += -I$(LAUNCHER_SHARE_SRC) -I$(LAUNCHER_PLATFORM_SRC)

#
# Create a dependency on libjli (Java Launcher Infrastructure)
#
# On UNIX, this is a relative dependency using $ORIGIN. Unfortunately, to
# do this reliably on Linux takes a different syntax than Solaris.
#
# On Windows, this is done by using the same directory as the executable
# itself, as with all the Windows libraries.
#
ifneq (,$(findstring $(PLATFORM), linux solaris))       # UNIX systems
  LDFLAGS += -L $(LIBDIR)/$(LIBARCH)/jli
  OTHER_LDLIBS += -ljli
  ifeq ($(PLATFORM), solaris)
    LDFLAGS += -R \$$ORIGIN/jli
  endif
  ifeq ($(PLATFORM), linux)
    LDFLAGS += -z origin
    LDFLAGS += -Wl,--allow-shlib-undefined
    LDFLAGS += -Wl,-rpath -Wl,\$$ORIGIN/jli
  endif
endif

ifeq ($(PLATFORM), windows)
  OTHER_LDLIBS += $(OUTPUTDIR)/tmp/java/jli/$(OBJDIRNAME)/static/jli.lib
  OTHER_LDLIBS += $(JVMLIB)
  # Files built here do not compile with warning level 3 if warnings are fatal
  # Also zlib files do not compile with warning level 3 if warnings are fatal
  COMPILER_WARNINGS_FATAL=false
endif

ifeq ($(PLATFORM), linux)
  # fork1() is declared in libjvm.so
  OTHER_LDLIBS += -ldl $(JVMLIB)
endif # PLATFORM

ifeq ($(PLATFORM), solaris)
  OTHER_LDLIBS += -ldl 
  # Use of lint has been helpful, but lots of errors needed to be suppressed:
  OTHER_LINTFLAGS += -errtags=yes
  # We only care about errors in user include files
  OTHER_LINTFLAGS += -errhdr=%user
  # Logging gives lint fits, so we turn it off
  # OTHER_LINTFLAGS += -DNO_JPLIS_LOGGING
  # We don't use the -Ncheck feature yet, it seems somewhat broken
  # OTHER_LINTFLAGS += -Ncheck -Nlevel=2
  # Even with above suppression, some files/code are hopelessly non-lintable
  MY_LINT_FILTER += | $(GREP) -v /usr/include
endif # PLATFORM

#
# This controls the ability to do logging in the library.
#
CPPFLAGS_DBG += -DJPLIS_LOGGING
CPPFLAGS_OPT += -DNO_JPLIS_LOGGING

ifeq ($(PLATFORM), windows)
  # equivalent of strcasecmp is stricmp on Windows
  CPPFLAGS_COMMON += -Dstrcasecmp=stricmp
endif

#
# Library to compile.
#
include $(BUILDDIR)/common/Library.gmk

# We don't want to link against -ljava
JAVALIB=

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(SHARE_SRC)/instrument $(PLATFORM_SRC)/instrument
vpath %.c $(IO_PLATFORM_SRC)

# Exported symbols 
ifeq ($(PLATFORM), windows)
  OTHER_LCF += -export:Agent_OnAttach
endif


#
# Lint checking rules (we filter out the lint pass and only list errors)
#
linterrs: $(FILES_c)
	$(RM) $@
	$(MAKE) lint.clean
	$(MAKE) lint.errors > $@ 2>&1
	$(CAT) $@ | $(GREP) 'warning:' $(MY_LINT_FILTER)

