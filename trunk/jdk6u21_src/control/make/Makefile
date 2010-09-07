#
# Copyright (c) 2009, 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)Makefile	1.297 06/12/09
#

TOPDIR=../..
include ./common/Defs.gmk

all::
	@$(ECHO) $(PLATFORM) $(ARCH) $(RELEASE) build started: `$(DATE) '+%y-%m-%d %H:%M'`

include ./sanity-rules.gmk
include ./control-rules.gmk
include ./hotspot-rules.gmk
include ./motif-rules.gmk
include ./j2se-rules.gmk
include ./install-rules.gmk
include ./sponsors-rules.gmk
include ./deploy-rules.gmk
include ./pubs-rules.gmk

ifndef SKIP_DEBUG_BUILD
  SKIP_DEBUG_BUILD=true
endif

ifndef SKIP_FASTDEBUG_BUILD
  SKIP_FASTDEBUG_BUILD=false
endif

all:: setup build

setup:
	@$(MKDIR) -p $(OUTPUTDIR)

build:: sanity 

clobber::

ifeq ($(BUILD_HOTSPOT), true)
  build:: $(HOTSPOT) 
  clobber:: hotspot-clobber
endif

ifeq ($(BUILD_MOTIF), true)
  build:: $(MOTIF)
  clobber:: motif-clobber
endif

ifeq ($(BUILD_J2SE), true)
  build:: $(J2SE)
  clobber:: j2se-clobber
endif

ifeq ($(BUILD_DEPLOY), true)
  build:: $(DEPLOY)
  clobber:: deploy-clobber
endif

#
# Generic debug build, fastdebug or debug. Needs special handling.
#  Note that debug builds do NOT do INSTALL steps, but must be done
#  after the product build and before the INSTALL step of the product build.
#
#   DEBUG_NAME is fastdebug or debug
#   ALT_OUTPUTDIR is changed to have -debug or -fastdebug suffix
#   The resulting j2sdk-image is copied into the product OUTPUTDIR as subdir
#     of $(OUTPUTDIR)/j2sdk-debug-image. With the name debug or fastdebug.
#     When the product build INSTALL step happens, it will create a
#     jdk-*-debug-** bundle (tar or zip) that includes whatever is in
#     $(OUTPUTDIR)/j2sdk-debug-image, and since it uses subdirs, untar'ing
#     these bundles will not clobber the product bits.
#

COMMON_DEBUG_FLAGS= \
	DEBUG_NAME=$(DEBUG_NAME) \
	ALT_OUTPUTDIR=$(OUTPUTDIR)-$(DEBUG_NAME) \
	NO_DOCS=true

#
# The following target is for plus builds.  It will step into the deploy,
# install, and sponsors and build the "plus" targets for each
#
plus_build:
	@$(ECHO) $@ deploy plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(DEPLOY_TOPDIR)/make/ ; $(MAKE) plus
	@$(ECHO) $@ deploy plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ install plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(INSTALL_TOPDIR)/make/ ; $(MAKE) plus
	@$(ECHO) $@ install plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ sponsors plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(SPONSORS_TOPDIR)/make/ ; $(MAKE) plus
	@$(ECHO) $@ sponsors plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`

#
# The following target is for plus builds that need to copy in the au msi/cab
# It will step into the deploy, install, and sponsors and 
# build the "plus" targets for each
#
au_plus_build:
	@$(ECHO) $@ deploy plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(DEPLOY_TOPDIR)/make/ ; $(MAKE) plus
	@$(ECHO) $@ deploy plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ install plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(INSTALL_TOPDIR)/make/ ; $(MAKE) au_plus
	@$(ECHO) $@ install plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ sponsors plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(SPONSORS_TOPDIR)/make/ ; $(MAKE) plus
	@$(ECHO) $@ sponsors plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`

#
# The following target is for enhanced plus builds.  It will step into the deploy,
# install, and sponsors and build the "enhanced_plus" targets for each ws. This
# target will regenerate all iftw.exe's, msi's, patches, and sponsor transforms
#
enhanced_plus_build:
	@$(ECHO) $@ deploy enhanced_plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(DEPLOY_TOPDIR)/make/ ; $(MAKE) KERNEL=off FASTJRE=TRUE enhanced_plus
	@$(ECHO) $@ deploy enhanced_plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ install enhanced_plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(INSTALL_TOPDIR)/make/ ; $(MAKE) KERNEL=off FASTJRE=TRUE enhanced_plus
	@$(ECHO) $@ install enhanced_plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ sponsors plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(SPONSORS_TOPDIR)/make/ ; $(MAKE) KERNEL=off FASTJRE=TRUE plus
	@$(ECHO) $@ sponsors plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`

#
# The following target is for enhanced plus builds.  It will step into the deploy,
# install, and sponsors and build the "enhanced_plus" targets for each ws. This
# target will regenerate all iftw.exe's, msi's, patches, and sponsor transforms
#
enhanced_au_plus_build:
	@$(ECHO) $@ deploy enhanced_plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(DEPLOY_TOPDIR)/make/ ; $(MAKE) enhanced_plus
	@$(ECHO) $@ deploy enhanced_plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ install enhanced_plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(INSTALL_TOPDIR)/make/ ; $(MAKE) enhanced_plus
	@$(ECHO) $@ install enhanced_plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`
	@$(ECHO) $@ sponsors plus build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(SPONSORS_TOPDIR)/make/ ; $(MAKE) plus
	@$(ECHO) $@ sponsors plus build finished: `$(DATE) '+%y-%m-%d %H:%M'`


combo_build:
	@$(ECHO) $@ installer combo build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(CD) $(INSTALL_TOPDIR)/make/installer/bundles/windows/ishield/wrapper/wrapper.jreboth ; $(MAKE) all

product_build:
	@$(ECHO) $@ build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(MAKE) SKIP_DEBUG_BUILD=true SKIP_FASTDEBUG_BUILD=true all
	@$(ECHO) $@ build finished: `$(DATE) '+%y-%m-%d %H:%M'`

generic_debug_build:
	@$(ECHO) $@ build started: `$(DATE) '+%y-%m-%d %H:%M'`
	$(MAKE) $(COMMON_DEBUG_FLAGS) setup build
	$(RM) -r $(OUTPUTDIR)/j2sdk-debug-image/$(DEBUG_NAME)
	$(MKDIR) -p $(OUTPUTDIR)/j2sdk-debug-image/$(DEBUG_NAME)
	( $(CD) $(OUTPUTDIR)-$(DEBUG_NAME)/j2sdk-image && \
	  $(TAR) cf - * ) | \
	  ( $(CD) $(OUTPUTDIR)/j2sdk-debug-image/$(DEBUG_NAME) && \
	    $(TAR) xf - )
	@$(ECHO) $@ build finished: `$(DATE) '+%y-%m-%d %H:%M'`

debug_build:
	$(MAKE) DEBUG_NAME=debug generic_debug_build

fastdebug_build:
	$(MAKE) DEBUG_NAME=fastdebug generic_debug_build

ifeq ($(SKIP_FASTDEBUG_BUILD), false)
  all:: fastdebug_build
endif

ifeq ($(SKIP_DEBUG_BUILD), false)
  all:: debug_build
endif

ifeq ($(BUILD_INSTALL), true)
  all :: $(INSTALL)
  clobber:: install-clobber
endif

ifeq ($(BUILD_SPONSORS), true)
  all :: $(SPONSORS)
  clobber:: sponsors-clobber
endif

ifndef DEV_ONLY
  ifndef SKIP_COMPARE_IMAGES
    all :: compare-images
  endif
endif

clobber:: clean-check
	$(RM) -r $(OUTPUTDIR)/*
	$(RM) -r $(OUTPUTDIR)-debug/*
	$(RM) -r $(OUTPUTDIR)-fastdebug/*
	-($(RMDIR) -p $(OUTPUTDIR) > $(DEV_NULL) 2>&1; $(TRUE))

refresh bringover: control-refresh hotspot-refresh motif-refresh j2se-refresh install-refresh deploy-refresh sponsors-refresh

source: source-sanity j2se-source deploy-source sponsors-source pubs-source

all:: 
	@$(ECHO) Control workspace build finished: `$(DATE) '+%y-%m-%d %H:%M'`

#
# Quick j2se verification build
#
j2se_only:
	$(MAKE) SKIP_FASTDEBUG_BUILD=true BUILD_HOTSPOT=false all


#
# Quick j2se verification fastdebug build
#
j2se_fastdebug_only:
	$(MAKE) DEBUG_NAME=fastdebug BUILD_HOTSPOT=false BUILD_DEPLOY=false BUILD_INSTALL=false BUILD_SPONSORS=false generic_debug_build

#
# Quick deploy verification fastdebug build
#
deploy_fastdebug_only:
	$(MAKE) DEBUG_NAME=fastdebug BUILD_HOTSPOT=false BUILD_J2SE=false BUILD_INSTALL=false BUILD_SPONSORS=false generic_debug_build

#
# Product build (skip debug builds)
#
product_only:
	$(MAKE) SKIP_FASTDEBUG_BUILD=true all

#
# Check target
#

check: variable_check

#
# Help target
#
help: intro_help target_help variable_help notes_help examples_help

# Intro help message
intro_help:
	@$(ECHO) "\
Makefile for the JDK control builds (all the JDK). \n\
"

# Target help
target_help:
	@$(ECHO) "\
--- Common Targets ---  \n\
all               -- build the core JDK (default target) \n\
help              -- Print out help information \n\
check             -- Check make variable values for correctness \n\
sanity            -- Perform detailed sanity checks on system and settings \n\
fastdebug_build   -- build the core JDK in 'fastdebug' mode (-g -O) \n\
debug_build       -- build the core JDK in 'debug' mode (-g) \n\
clean             -- remove all built and imported files \n\
clobber           -- same as clean \n\
"

# Variable help (only common ones used by this workspace)
variable_help: variable_help_intro variable_list variable_help_end
variable_help_intro:
	@$(ECHO) "--- Common Variables ---"
variable_help_end:
	@$(ECHO) " "

# One line descriptions for the variables
OUTPUTDIR.desc             = Output directory
PARALLEL_COMPILE_JOBS.desc = Solaris/Linux parallel compile run count
SLASH_JAVA.desc            = Root of all build tools, e.g. /java or J:
BOOTDIR.desc               = JDK used to boot the build
JDK_IMPORT_PATH.desc       = JDK used to import components of the build
COMPILER_PATH.desc         = Compiler install directory
CACERTS_FILE.desc          = Location of certificates file
DEVTOOLS_PATH.desc         = Directory containing zip and gnumake
MOTIF_DIR.desc             = Root directory for motif directories
CUPS_HEADERS_PATH.desc     = Include directory location for CUPS header files
ODBCDIR.desc               = Root directory for jdbc odbc files
DXSDK_PATH.desc            = Root directory of DirectX SDK
MSDEVTOOLS_PATH.desc       = Root directory of VC++ tools (e.g. rc.exe)
MSVCRT_DLL_PATH.desc       = Directory containing mscvrt.dll
UNICOWS_DLL_PATH.desc      = Directory containing unicows.dll
UNICOWS_LIB_PATH.desc      = Directory containing unicows.lib

# Make variables to print out (description and value)
VARIABLE_PRINTVAL_LIST +=       \
    OUTPUTDIR                   \
    PARALLEL_COMPILE_JOBS       \
    SLASH_JAVA                  \
    BOOTDIR                     \
    JDK_IMPORT_PATH             \
    COMPILER_PATH               \
    CACERTS_FILE                \
    DEVTOOLS_PATH               \
    MOTIF_DIR                   \
    ODBCDIR

# Make variables that should refer to directories that exist
VARIABLE_CHECKDIR_LIST +=       \
    SLASH_JAVA                  \
    BOOTDIR                     \
    JDK_IMPORT_PATH             \
    COMPILER_PATH               \
    DEVTOOLS_PATH               \
    ODBCDIR

# Make variables that should refer to files that exist
VARIABLE_CHECKFIL_LIST +=       \
    CACERTS_FILE

# Some are windows specific
ifeq ($(PLATFORM), windows)

VARIABLE_PRINTVAL_LIST +=       \
    DXSDK_PATH                  \
    MSDEVTOOLS_PATH             \
    MSVCRT_DLL_PATH             \
    UNICOWS_DLL_PATH            \
    UNICOWS_LIB_PATH

VARIABLE_CHECKDIR_LIST +=       \
    DXSDK_PATH                  \
    MSDEVTOOLS_PATH             \
    MSVCRT_DLL_PATH             \
    UNICOWS_DLL_PATH            \
    UNICOWS_LIB_PATH

endif

# For pattern rules below, so all are treated the same
DO_PRINTVAL_LIST=$(VARIABLE_PRINTVAL_LIST:%=%.printval)
DO_CHECKDIR_LIST=$(VARIABLE_CHECKDIR_LIST:%=%.checkdir)
DO_CHECKFIL_LIST=$(VARIABLE_CHECKFIL_LIST:%=%.checkfil)

# Complete variable check
variable_check: $(DO_CHECKDIR_LIST) $(DO_CHECKFIL_LIST)
variable_list: $(DO_PRINTVAL_LIST) variable_check

# Pattern rule for printing out a variable
%.printval:
	@$(ECHO) "  ALT_$* - $($*.desc)"
	@$(ECHO) "  \t $*=$($*)"

# Pattern rule for checking to see if a variable with a directory exists
%.checkdir:
	@if [ ! -d $($*) ] ; then \
	    $(ECHO) "WARNING: $* does not exist, try $(MAKE) sanity"; \
	fi

# Pattern rule for checking to see if a variable with a file exists
%.checkfil:
	@if [ ! -f $($*) ] ; then \
	    $(ECHO) "WARNING: $* does not exist, try $(MAKE) sanity"; \
	fi

# Misc notes on help
notes_help:
	@$(ECHO) "\
--- Notes --- \n\
- All builds use same output directory unless overridden with \n\
 \t ALT_OUTPUTDIR=<dir>, changing from product to fastdebug you may want \n\
 \t to use the clean target first. \n\
- JDK_IMPORT_PATH must refer to a compatible build, not all past promoted \n\
 \t builds or previous release JDK builds will work. \n\
- The fastest builds have been when the workspace and the BOOTDIR are on \n\
 \t local disk. \n\
"

examples_help:
	@$(ECHO) "\
--- Examples --- \n\
  $(MAKE) fastdebug_build \n\
  $(MAKE) ALT_OUTPUTDIR=/tmp/foobar all \n\
  $(MAKE) ALT_OUTPUTDIR=/tmp/foobar fastdebug_build \n\
  $(MAKE) ALT_OUTPUTDIR=/tmp/foobar all \n\
  $(MAKE) ALT_BOOTDIR=/opt/java/jdk1.5.0 \n\
  $(MAKE) ALT_JDK_IMPORT_PATH=/opt/java/jdk1.6.0 \n\
"

################################################################
# JPRT rule to build this workspace
################################################################

include jprt.gmk

################################################################
#  PHONY
################################################################

.PHONY: all build what clobber insane refresh bringover \
	j2se-source deploy-source fastdebug_build debug_build product_build \
	pubs-source
