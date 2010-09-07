#
# @(#)Makefile	1.4 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# Use is subject to license terms.
#

#
# Java Launcher Infrastructure Library (libjli)
#
# This library provides shared support for the Java launcher in all of
# its manifestations (java, javaw, javac, ...).
#
BUILDDIR = ../..
LIBRARY = jli
PRODUCT = java

#
# Must be included before Defs.gmk to be functional.
#
# Note that for Windows, both a dynamic and static version are built.
# Doing the compiles with the static library specified can be overridden
# by the link step, but not the reverse.
#
MS_RUNTIME_STATIC = true

include $(BUILDDIR)/common/Defs.gmk

ZLIB_VERSION = 1.1.3
ZIP_SRC = $(SHARE_SRC)/native/java/util/zip/zlib-$(ZLIB_VERSION)
LAUNCHER_SHARE_SRC = $(SHARE_SRC)/bin
LAUNCHER_PLATFORM_SRC = $(PLATFORM_SRC)/bin

#
# Files to compile.
#
FILES_c = \
	$(CTARGDIR)parse_manifest.c \
	$(CTARGDIR)version_comp.c \
	$(CTARGDIR)wildcard.c \
	$(CTARGDIR)jli_util.c \
	$(CTARGDIR)inflate.c \
	$(CTARGDIR)infblock.c \
	$(CTARGDIR)inftrees.c \
	$(CTARGDIR)infcodes.c \
	$(CTARGDIR)inffast.c \
	$(CTARGDIR)infutil.c \
	$(CTARGDIR)zadler32.c \
	$(CTARGDIR)zutil.c

OTHER_CPPFLAGS += -DEXPAND_CLASSPATH_WILDCARDS

ifneq (,$(findstring $(PLATFORM), linux solaris))	# UNIX systems
	LIB_LOCATION = $(LIBDIR)/$(LIBARCH)/jli
	# Guarantee very limited dependencies
	LDLIBS = -lc
endif

ifeq ($(PLATFORM), windows)
	EXTRA_LIBS =
	JAVALIB =
	OTHER_LCF =  -export:JLI_ParseManifest \
		     -export:JLI_JarUnpackFile \
		     -export:JLI_FreeManifest \
		     -export:JLI_ManifestIterate
	OTHER_LCF += -export:JLI_ExactVersionId \
		     -export:JLI_PrefixVersionId \
		     -export:JLI_AcceptableRelease \
		     -export:JLI_ValidVersionString
	OTHER_LCF += -export:JLI_WildcardExpandClasspath
	OTHER_LCF += -export:JLI_MemAlloc \
		     -export:JLI_MemRealloc \
		     -export:JLI_StringDup \
		     -export:JLI_MemFree

	# Files from zlib built here do not compile with warning level 3
	# if warnings are fatal
	COMPILER_WARNINGS_FATAL=false
endif

OTHER_INCLUDES += -I$(LAUNCHER_SHARE_SRC)
OTHER_INCLUDES += -I$(LAUNCHER_PLATFORM_SRC)
OTHER_INCLUDES += -I$(ZIP_SRC)

#
# Library to compile.
#
include $(BUILDDIR)/common/Mapfile-vers.gmk
include $(BUILDDIR)/common/Library.gmk

#
# On Windows, some executable objects need to be statically linked against
# the jli library.  Hence, we need both a standard library (archive) and
# an import library (associated with a dll).  These both usually have the
# extension .LIB, so they need to be placed in different directories.  The
# import library is build (as usual) in the $(OBJDIR) directory while the
# standard library is built in a "static" subdirectory.  The standard library
# is not delivered as part of the product, but is only needed as part of
# the build process.  The import library is built by the standard rules
# in Library.gmk.  The additional rules which follow build the standard
# library.
#
ifeq ($(PLATFORM), windows)

STATIC_LIBRARY_DIR = $(OBJDIR)/static
STATIC_LIBRARY_NAME = $(LIBPREFIX)$(LIBRARY).lib
STATIC_LIBRARY = $(STATIC_LIBRARY_DIR)/$(STATIC_LIBRARY_NAME)

$(STATIC_LIBRARY_DIR): $(OBJDIR)
	@$(prep-target)
	@$(MKDIR) $(STATIC_LIBRARY_DIR)

$(STATIC_LIBRARY): $(STATIC_LIBRARY_DIR)
	@$(prep-target)
	$(LIBEXE) -nologo -out:$@ $(FILES_o)

library:: $(STATIC_LIBRARY)

endif	# PLATFORM

#
# Add to ambient vpath so we pick up the library files
#
vpath %.c $(LAUNCHER_SHARE_SRC) $(ZIP_SRC)

