# 
# @(#)Makefile	1.18 03/12/23 16:35:14
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# This makefile creates a build tree and lights off a build.
# You can go back into the build tree and perform rebuilds or
# incremental builds as desired.  Be sure to reestablish
# environment variable settings for LD_LIBRARY_PATH and JAVA_HOME.

# The make process now relies on java and javac. These can be
# specified either implicitly on the PATH, by setting the
# (JDK-inherited) ALT_BOOTDIR environment variable to full path to a
# JDK in which bin/java and bin/javac are present and working (e.g.,
# /usr/local/java/jdk1.3/solaris), or via the (JDK-inherited) 
# default BOOTDIR path value. Note that one of ALT_BOOTDIR
# or BOOTDIR has to be set. We do *not* search javac, javah, rmic etc.
# from the PATH.

# Along with VM, Serviceability Agent (SA) is built for SA/JDI binding. 
# JDI binding on SA produces two binaries: 
#  1. sa-jdi.jar   - This is build before building libjvm[_g].so
#                    Please refer to ./makefiles/sa.make
#  2. libsa[_g].so - Native library for SA - This is built after
#                    libjsig[_g].so (signal interposition library)
#                    Please refer to ./makefiles/vm.make 
# If $(GAMMADIR)/agent dir is not present, SA components are not built.

# Unless GAMMADIR is set on the command line, search upward from
# the current directory for a parent directory containing "src/share/vm".
# If that fails, look for $GAMMADIR in the environment.
# When the tree of subdirs is built, this setting is stored in each flags.make.
GAMMADIR := $(shell until ([ -d dev ]&&echo $${GAMMADIR:-/GAMMADIR/}) || ([ -d src/share/vm ]&&pwd); do cd ..; done)
unexport GAMMADIR

# The <arch> below is determined by the machine you are building
# on. On a sparc system <arch> will be sparc. On an intel system
# <arch> will be i486.

# There is an irregular correspondence between make targets and actions:
#	Target		Tree Type	Build Dir
#	debug		compiler2	<os>_<arch>_compiler2/debug
#	profiled	compiler2		<os>_<arch>_compiler2/profiled
#	(none)		compiler2		<os>_<arch>_compiler2/product
#	product		compiler2		<os>_<arch>_compiler2/product
#	optimized	compiler2		<os>_<arch>_compiler2/optimized
#	debug1		compiler1	<os>_<arch>_compiler1/debug
#	profiled1	compiler1		<os>_<arch>_compiler1/profiled
#	product1	compiler1		<os>_<arch>_compiler1/product
#	optimized1	compiler1		<os>_<arch>_compiler1/optimized
#	basicdebug	core		<os>_<arch>_core/debug
# Hmm how do these work QQQ
#	(none)		core		<os>_<arch>_core/profiled
#	(none)		core		<os>_<arch>_core/product
#	basic		core		<os>_<arch>_core/optimized

# Target with suffix of 1 build the VM with Compiler1

include $(GAMMADIR)/build/linux/makefiles/rules.make

OS      = linux
ARCH    = $(shell uname -m)
LIB_ARCH= $ARCH

ifdef ITANIUM
  ARCH=ia64
endif

ifeq ($(ARCH), x86_64)
  ARCH=amd64
endif

ifneq ($(ARCH), sparc)
  ifneq ($(ARCH), ia64)
    ifneq ($(ARCH), amd64)
      ARCH=i486
    endif
  endif
endif

export ARCH

ifeq ($(ARCH), i486)
    TEST_ARCH=i386
else
    TEST_ARCH=$(ARCH)
endif

SUBDIR_DOCS= $(OS)_$(ARCH)_docs
buildATree = ${GAMMADIR}/build/${OS}/makefiles/buildATree.sh

.PHONY: checks check_os_version check_j2se_version docs clean_docs

# For now, until the compiler is less wobbly:
TESTFLAGS = -Xbatch

RUN_TEST = \
	echo "Testing $$gamma...";                                      \
	echo "\#!/bin/csh" > test;                                      \
	echo "setenv LD_LIBRARY_PATH .:${JAVA_HOME}/jre/lib/${TEST_ARCH}:${JAVA_HOME}/jre/lib/${TEST_ARCH}/native_threads:${LD_LIBRARY_PATH}" >> test; \
	echo "./$$gamma < /dev/null $(TESTFLAGS) -classpath ${GAMMADIR}/build/${OS}:${JAVA_HOME}/jre/lib/lib/rt.jar:${JAVA_HOME}/classes Queens" >> test;          \
	chmod +x ./test; ./test

${OS}_${ARCH}_compiler2/debug ${OS}_${ARCH}_compiler2/optimized ${OS}_${ARCH}_compiler2/fastdebug ${OS}_${ARCH}_compiler2/jvmg ${OS}_${ARCH}_compiler2/profiled ${OS}_${ARCH}_compiler2/product : checks $(buildATree)
	sh $(buildATree) compiler2 ${GAMMADIR} ${OS} ${ARCH}

checks: check_os_version check_j2se_version

# We do not want people accidentally building on Linux 2.1.x or earlier.
check_os_version:
	@ case ${DISABLE_HOTSPOT_OS_VERSION_CHECK},`uname -r` in \
	  ,2.[0-1].*) >&2 echo "*** This OS is not supported:" `uname -a`; exit 1;; \
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

debug: ${OS}_${ARCH}_compiler2/debug
	(cd ${OS}_${ARCH}_compiler2/debug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/debug; gamma=gamma_g; $(RUN_TEST))

optimized: ${OS}_${ARCH}_compiler2/optimized
	(cd ${OS}_${ARCH}_compiler2/optimized; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/optimized; gamma=gamma; $(RUN_TEST))

fastdebug: ${OS}_${ARCH}_compiler2/fastdebug
	(cd ${OS}_${ARCH}_compiler2/fastdebug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/fastdebug; gamma=gamma; $(RUN_TEST))

jvmg: ${OS}_${ARCH}_compiler2/jvmg
	(cd ${OS}_${ARCH}_compiler2/jvmg; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/jvmg; gamma=gamma_g; $(RUN_TEST))

product: ${OS}_${ARCH}_compiler2/product
	(cd ${OS}_${ARCH}_compiler2/product; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/product; gamma=gamma; $(RUN_TEST))

profiled: ${OS}_${ARCH}_compiler2/profiled
	(cd ${OS}_${ARCH}_compiler2/profiled; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler2/profiled; gamma=gamma; $(RUN_TEST))

${OS}_${ARCH}_compiler1/debug ${OS}_${ARCH}_compiler1/optimized ${OS}_${ARCH}_compiler1/fastdebug ${OS}_${ARCH}_compiler1/jvmg ${OS}_${ARCH}_compiler1/profiled ${OS}_${ARCH}_compiler1/product : checks $(buildATree)
	sh $(buildATree) compiler1 ${GAMMADIR} ${OS} ${ARCH}

debug1: ${OS}_${ARCH}_compiler1/debug
	(cd ${OS}_${ARCH}_compiler1/debug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/debug; gamma=gamma_g; $(RUN_TEST))

optimized1: ${OS}_${ARCH}_compiler1/optimized
	(cd ${OS}_${ARCH}_compiler1/optimized; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/optimized; gamma=gamma; $(RUN_TEST))

fastdebug1: ${OS}_${ARCH}_compiler1/fastdebug
	(cd ${OS}_${ARCH}_compiler1/fastdebug; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/fastdebug; gamma=gamma; $(RUN_TEST))

jvmg1: ${OS}_${ARCH}_compiler1/jvmg
	(cd ${OS}_${ARCH}_compiler1/jvmg; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/jvmg; gamma=gamma_g; $(RUN_TEST))

product1: ${OS}_${ARCH}_compiler1/product
	(cd ${OS}_${ARCH}_compiler1/product; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/product; gamma=gamma; $(RUN_TEST))

profiled1: ${OS}_${ARCH}_compiler1/profiled
	(cd ${OS}_${ARCH}_compiler1/profiled; $(MAKE))
#	@(cd ${OS}_${ARCH}_compiler1/profiled; gamma=gamma; $(RUN_TEST))

${OS}_${ARCH}_core/debug ${OS}_${ARCH}_core/optimized ${OS}_${ARCH}_core/fastdebug ${OS}_${ARCH}_core/jvmg ${OS}_${ARCH}_core/profiled ${OS}_${ARCH}_core/product : checks $(buildATree)
	sh $(buildATree) core ${GAMMADIR} ${OS} ${ARCH}

debugcore: ${OS}_${ARCH}_core/debug
	(cd ${OS}_${ARCH}_core/debug; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/debug; gamma=gamma_g; $(RUN_TEST))

optimizedcore: ${OS}_${ARCH}_core/optimized
	(cd ${OS}_${ARCH}_core/optimized; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/optimized; gamma=gamma; $(RUN_TEST))

fastdebugcore: ${OS}_${ARCH}_core/fastdebug
	(cd ${OS}_${ARCH}_core/fastdebug; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/fastdebug; gamma=gamma; $(RUN_TEST))

jvmgcore: ${OS}_${ARCH}_core/jvmg
	(cd ${OS}_${ARCH}_core/jvmg; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/jvmg; gamma=gamma_g; $(RUN_TEST))

productcore: ${OS}_${ARCH}_core/product
	(cd ${OS}_${ARCH}_core/product; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/product; gamma=gamma; $(RUN_TEST))

profiledcore: ${OS}_${ARCH}_core/profiled
	(cd ${OS}_${ARCH}_core/profiled; $(MAKE))
#	@(cd ${OS}_${ARCH}_core/profiled; gamma=gamma; $(RUN_TEST))

basicdebug: debugcore
basic: optimizedcore

# Doc target.  This is the same for all build options.
#     Hence create a docs directory beside ...$(ARCH)_[...]
docs: checks
	$(QUIETLY) mkdir -p $(SUBDIR_DOCS)
	$(MAKE) -f $(GAMMADIR)/build/linux/makefiles/jvmti.make $(MFLAGS) GAMMADIR=$(GAMMADIR) JvmtiOutDir=$(SUBDIR_DOCS) jvmtidocs

# Intel-style names:
compiler1: jvmg1 product1
compiler2: jvmg product
core: jvmgcore productcore

clean:
	(rm -fr ${OS}_${ARCH}_core; rm -fr ${OS}_${ARCH}_compiler2 ; rm -fr ${OS}_${ARCH}_compiler1; rm -fr $(SUBDIR_DOCS))
