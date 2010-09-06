# 
# @(#)Makefile	1.42 03/12/23 16:35:22
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# This makefile creates a build tree and lights off a build.
# You can go back into the build tree and perform rebuilds or
# incremental builds as desired.

# The make process now relies on java and javac. These can be
# specified implicitly on the PATH, by setting the
# (JDK-inherited) ALT_BOOTDIR environment variable to a full path to a
# JDK in which bin/java and bin/javac are present and working (e.g.,
# /usr/local/java/jdk1.3/solaris), or via the (JDK-inherited) default
# BOOTDIR path to a bootsrap java and javac. Note that one of ALT_BOOTDIR
# or BOOTDIR has to be set. We do *not* use javac, javah, rmic etc. 
# from the PATH.

# Along with VM, Serviceability Agent (SA) is built for SA/JDI binding. 
# JDI binding on SA produces two binaries: 
#  1. sa-jdi.jar       - This is build before building libjvm[_g].so
#                        Please refer to ./makefiles/sa.make
#  2. libsaproc[_g].so - Native library for SA - This is built after
#                        libjsig.so (signal interposition library)
#                        Please refer to ./makefiles/vm.make 
# If $(GAMMADIR)/agent dir is not present, then SA components are not built.

# Unless GAMMADIR is set on the command line, search upward from
# the current directory for a parent directory containing "src/share/vm".
# If that fails, look for $GAMMADIR in the environment.
# When the tree of subdirs is built, this setting is stored in each flags.make.
GAMMADIR := $(shell until ([ -d dev ]&&echo $${GAMMADIR:-/GAMMADIR/}) || ([ -d src/share/vm ]&&pwd); do cd ..; done)
unexport GAMMADIR

include $(GAMMADIR)/build/solaris/makefiles/rules.make

# The <arch> below is determined by the machine you are building
# on. On a sparc system <arch> will be sparc. On an intel system
# <arch> will be i486.

OS		:= $(subst SunOS,solaris,$(shell uname -s))
ARCH		:= $(subst i386,i486,$(shell uname -p))
HOST		:= $(shell uname -n)
ifdef LP64
# For the 64 bit Sparc build, we want to
# build it into a unique directory (solaris_sparcv9_xxxx)
# even though the platform is still sparc
BUILDARCH = sparcv9
else
BUILDARCH = $(ARCH)
endif


# The following renders pathnames in generated Makefiles valid on
# machines other than the machine containing the build tree.
#
# For example, let's say my build tree lives on /files12 on
# exact.east.sun.com.  This logic will cause GAMMADIR to begin with
# /net/exact/files12/...
#
# We only do this on SunOS variants, for a couple of reasons:
#  * It is extremely rare that source trees exist on other systems
#  * It has been claimed that the Linux automounter is flakey, so
#    changing GAMMADIR in a way that exercises the automounter could
#    prove to be a source of unreliability in the build process.
# Obviously, this Makefile is only relevant on SunOS boxes to begin
# with, but the SunOS conditionalization will make it easier to
# combine Makefiles in the future (assuming we ever do that).

ifeq ($(OS),solaris)

  #   prepend current directory to relative pathnames.
  NEW_GAMMADIR :=                                    \
    $(shell echo $(GAMMADIR) |                       \
      sed -e "s=^\([^/].*\)=$(shell pwd)/\1="        \
     )
  unexport NEW_GAMMADIR

  # If NEW_GAMMADIR doesn't already start with "/net/":
  ifeq ($(strip $(filter /net/%,$(NEW_GAMMADIR))),)
    #   prepend /net/$(HOST)
    #   remove /net/$(HOST) if name already began with /home/
    #   remove /net/$(HOST) if name already began with /java/
    #   remove /net/$(HOST) if name already began with /lab/
    NEW_GAMMADIR :=                                     \
         $(shell echo $(NEW_GAMMADIR) |                 \
                 sed -e "s=^\(.*\)=/net/$(HOST)\1="     \
                     -e "s=^/net/$(HOST)/home/=/home/=" \
                     -e "s=^/net/$(HOST)/java/=/java/=" \
                     -e "s=^/net/$(HOST)/lab/=/lab/="   \
          )
    # Don't use the new value for GAMMADIR unless a file with the new
    # name actually exists.
    ifneq ($(wildcard $(NEW_GAMMADIR)),)
      GAMMADIR := $(NEW_GAMMADIR)
    endif
  endif

endif


# There is a (semi-) regular correspondence between make targets and actions:
#
#	Target		Tree Type	Build Dir
#
#	debug		compiler2	<os>_<arch>_compiler2/debug
#	fastdebug	compiler2	<os>_<arch>_compiler2/fastdebug
#	jvmg		compiler2	<os>_<arch>_compiler2/jvmg
#	optimized	compiler2	<os>_<arch>_compiler2/optimized
#	profiled	compiler2	<os>_<arch>_compiler2/profiled
#	product		compiler2	<os>_<arch>_compiler2/product
#
#	debug1		compiler1	<os>_<arch>_compiler1/debug
#	fastdebug1	compiler1	<os>_<arch>_compiler1/fastdebug
#	jvmg1		compiler1	<os>_<arch>_compiler1/jvmg
#	optimized1	compiler1	<os>_<arch>_compiler1/optimized
#	profiled1	compiler1	<os>_<arch>_compiler1/profiled
#	product1	compiler1	<os>_<arch>_compiler1/product
#
#	debugcore	core		<os>_<arch>_core/debug
#	fastdebugcore	core		<os>_<arch>_core/fastdebug
#	jvmgcore	core		<os>_<arch>_core/jvmg
#	optimizedcore	core		<os>_<arch>_core/optimized
#	profiledcore	core		<os>_<arch>_core/profiled
#	productcore	core		<os>_<arch>_core/product
#
# What you get with each target:
#
# debug*	- "thin" libjvm_g - debug info linked into the gamma_g launcher
# fastdebug*	- optimized compile, but with asserts enabled
# jvmg*		- "fat" libjvm_g - debug info linked into libjvm_g.so
# optimized*	- optimized compile, no asserts
# profiled*	- gprof
# product*	- the shippable thing:  optimized compile, no asserts, -DPRODUCT

# This target list needs to be coordinated with the usage message
# in the build.sh script:
TARGETS		= debug jvmg fastdebug optimized profiled product

OS      = solaris

SUBDIR_DOCS	= $(OS)_$(BUILDARCH)_docs
SUBDIRS_C1	= $(addprefix $(OS)_$(BUILDARCH)_compiler1/,$(TARGETS))
SUBDIRS_C2	= $(addprefix $(OS)_$(BUILDARCH)_compiler2/,$(TARGETS))
SUBDIRS_CORE	= $(addprefix $(OS)_$(BUILDARCH)_core/,$(TARGETS))

TARGETS_C2	= $(TARGETS)
TARGETS_C1	= $(addsuffix 1,$(TARGETS))
TARGETS_CORE	= $(addsuffix core,$(TARGETS))

BUILDTREE_MAKE	= $(GAMMADIR)/build/solaris/makefiles/buildtree.make
BUILDTREE_VARS	= GAMMADIR=$(GAMMADIR) OS_FAMILY=$(OS) ARCH=$(ARCH)
BUILDTREE	= $(MAKE) -f $(BUILDTREE_MAKE) $(BUILDTREE_VARS)

.PHONY: $(TARGETS_C2) $(TARGETS_C1) $(TARGETS_CORE)
.PHONY: tree tree1 treecore
.PHONY: all compiler1 compiler2 core
.PHONY: clean clean_compiler1 clean_compiler2 clean_core docs clean_docs
.PHONY: checks check_os_version check_j2se_version

# Could make everything by default, but that would take a while.
all:
	@echo "Try '$(MAKE) <target> ...'  where <target> is one or more of"
	@echo "  $(TARGETS_C2)"
	@echo "  $(TARGETS_C1)"
	@echo "  $(TARGETS_CORE)"

checks: check_os_version check_j2se_version

# We do not want people accidentally building on old Solaris releases
# such as:
#     2.5.1 (uname -r = 5.5.1)
#     2.6   (uname -r = 5.6)
check_os_version:
	$(QUIETLY) case ${DISABLE_HOTSPOT_OS_VERSION_CHECK},`uname -r` in \
	  ,5.10) :;; \
	  ,5.[789]) :;; \
	  ,*) >&2 echo "*** This OS is not supported:" `uname -a`; exit 1;; \
	  esac
# Disable this check by setting DISABLE_HOTSPOT_OS_VERSION_CHECK=ok.

# jvmti.make requires XSLT (J2SE 1.4.x or newer):
# If not found then fail fast.
check_j2se_version:
	$(QUIETLY) if [ `$(RUN.JAVA) -fullversion 2>&1 | grep -c '1\.[45]'` -eq 0 ] ; then \
	  $(RUN.JAVA) -version; \
	  echo "*** An XSLT processor (J2SE 1.4.x or newer) is required to bootstrap this build"; \
	  exit 1; \
	fi

$(SUBDIRS_C2): $(BUILDTREE_MAKE)
	$(QUIETLY) $(MAKE) -f $(GAMMADIR)/build/solaris/Makefile checks
	$(BUILDTREE) VARIANT=compiler2

$(SUBDIRS_C1): $(BUILDTREE_MAKE)
	$(QUIETLY) $(MAKE) -f $(GAMMADIR)/build/solaris/Makefile checks
	$(BUILDTREE) VARIANT=compiler1

$(SUBDIRS_CORE): $(BUILDTREE_MAKE)
	$(QUIETLY) $(MAKE) -f $(GAMMADIR)/build/solaris/Makefile checks
	$(BUILDTREE) VARIANT=core

$(TARGETS_C2):  $(SUBDIRS_C2)
	cd $(OS)_$(BUILDARCH)_compiler2/$@ && $(MAKE) $(MFLAGS)
	cd $(OS)_$(BUILDARCH)_compiler2/$@ && ./test_gamma

$(TARGETS_C1):  $(SUBDIRS_C1)
	cd $(OS)_$(BUILDARCH)_compiler1/$(patsubst %1,%,$@) && $(MAKE) $(MFLAGS)
	-cd $(OS)_$(BUILDARCH)_compiler1/$(patsubst %1,%,$@) && ./test_gamma

$(TARGETS_CORE):  $(SUBDIRS_CORE)
	cd $(OS)_$(BUILDARCH)_core/$(patsubst %core,%,$@) && $(MAKE) $(MFLAGS)
	cd $(OS)_$(BUILDARCH)_core/$(patsubst %core,%,$@) && ./test_gamma

# Just build the tree, and nothing else:
tree:      $(SUBDIRS_C2)
tree1:     $(SUBDIRS_C1)
treecore:  $(SUBDIRS_CORE)

# Doc target.
docs: checks
	$(QUIETLY) mkdir -p $(SUBDIR_DOCS)
	$(MAKE) -f $(GAMMADIR)/build/solaris/makefiles/jvmti.make $(MFLAGS) $(BUILDTREE_VARS) JvmtiOutDir=$(SUBDIR_DOCS) jvmtidocs

# Synonyms for win32-like targets.
compiler2:  jvmg product

compiler1:  jvmg1 product1

core: jvmgcore productcore

clean_docs:
	rm -rf $(SUBDIR_DOCS)

clean_compiler1 clean_compiler2 clean_core:
	rm -rf $(OS)_$(ARCH)_$(subst clean_,,$@)
	rm -rf $(OS)_$(ARCH)v9_$(subst clean_,,$@)

clean:  clean_compiler2 clean_compiler1 clean_core clean_docs

include $(GAMMADIR)/build/solaris/makefiles/cscope.make
