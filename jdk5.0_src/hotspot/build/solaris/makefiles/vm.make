# 
# @(#)vm.make	1.79 04/02/20 13:12:58
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# This makefile (vm.make) is included from the vm.make in the
# build directories.
# It knows how to compile and link the VM.

# It assumes the following flags are set:
# Platform_file, Src_Dirs, Obj_Files, SYSDEFS, INCLUDES, CFLAGS, LFLAGS, LIBS

# And it assumes that the deps and incls have already been built.

# -- D. Ungar (5/97) from a file by Bill Bush

# build whole VM by default, I can also build foo.o or foo.i

# Common build rules.
include $(GAMMADIR)/build/solaris/makefiles/rules.make

AOUT   = gamma$(G_SUFFIX)
GENERATED  = ../generated

# set VPATH so make knows where to look for source files
# Src_Dirs is everything in src/share/vm/*, plus the right os/*/vm and cpu/*/vm
# The incls directory contains generated header file lists for inclusion.
# The adfiles directory contains ad_<arch>.[ch]pp.
# The jvmtifiles directory contains jvmti*.[ch]pp and jvmdi*.[ch]pp
Src_Dirs_V = $(GENERATED)/adfiles $(GENERATED)/jvmtifiles ${Src_Dirs} $(GENERATED)/incls
VPATH    += $(Src_Dirs_V:%=%:)

# set INCLUDES for C preprocessor
Src_Dirs_I = $(GENERATED)/adfiles $(GENERATED)/jvmtifiles ${Src_Dirs} $(GENERATED) 
INCLUDES += $(Src_Dirs_I:%=-I%)

BUILD_VERSION = -DHOTSPOT_BUILD_VERSION="\"$(HOTSPOT_BUILD_VERSION)\""
BUILD_VERSION$(HOTSPOT_BUILD_VERSION) = 

BUILD_USER = -DHOTSPOT_BUILD_USER="$(HOTSPOT_BUILD_USER)"
BUILD_USER$(HOTSPOT_BUILD_USER) = 

CPPFLAGS = ${SYSDEFS} ${INCLUDES} ${BUILD_VERSION} ${BUILD_USER}

# CFLAGS_WARN holds compiler options to suppress/enable warnings.
CFLAGS	+= $(CFLAGS_WARN)

# Extra flags from gnumake's invocation or environment
CFLAGS += $(EXTRA_CFLAGS)

# Do not use C++ exception handling
CFLAGS += $(CFLAGS/NOEX)

# The whole megilla:
ifeq ($(shell expr $(COMPILER_REV) \>= 5.5), 1)
# List the libraries in the order the compiler was designed for
LIBS += -lsocket -lsched -ldl -lCrun -lm -lthread -lc
else
LIBS += -ldl -lthread -lsocket -lm -lsched
endif

JVM = jvm$(G_SUFFIX)
LIBJVM = lib$(JVM).so

ifeq ("${G_SUFFIX}", "_g")
LN_LIBJVM = libjvm.so
else
LN_LIBJVM = libjvm_g.so
endif

# By default, link the *.o into the library, not the executable.
LINK_INTO$(LINK_INTO) = LIBJVM

# making launch:
AOUT.o = $(AOUT.o/LINK_INTO_$(LINK_INTO))
AOUT.o/LINK_INTO_AOUT    = $(Obj_Files)
AOUT.o/LINK_INTO_LIBJVM  =

# For now, build a copy of the JDK1.2beta4 launcher.
# This is useful, because the development cycle goes faster
# if we can use ild to statically link the VM into the launcher.
# Eventually, we should expunge all use of $(AOUT) from the makefiles.
# The optimized (product) build should certainly avoid making a new launcher.
AOUT.o += launcher.o

LAUNCHER = $(GAMMADIR)/src/os/$(Platform_os_family)/launcher
ifdef LP64
LAUNCHERFLAGS = -xarch=v9 -I$(LAUNCHER) -I$(GAMMADIR)/src/share/vm/prims
ARCH = sparcv9
else
LAUNCHERFLAGS = -I$(LAUNCHER) -I$(GAMMADIR)/src/share/vm/prims
ARCH = $(Platform_arch)
endif

# libjvm-related things.
LIBJVM.o = $(LIBJVM.o/LINK_INTO_$(LINK_INTO))
LIBJVM.o/LINK_INTO_AOUT    =
LIBJVM.o/LINK_INTO_LIBJVM  = $(Obj_Files)

JSIG = jsig$(G_SUFFIX)
LIBJSIG = lib$(JSIG).so

SAPROC = saproc$(G_SUFFIX)
LIBSAPROC = lib$(SAPROC).so

OS_ver = $(shell uname -r)
SUM = /usr/bin/sum

# we build libjvm_db/dtrace for COMPILER1 and COMPILER2
# but not for CORE configuration

ifneq ("${TYPE}", "CORE")

JVM_DB = libjvm_db
LIBJVM_DB = libjvm$(G_SUFFIX)_db.so
JVMOFFS	= JvmOffsets
GENOFFS	= generate$(JVMOFFS)

DTRACE_SRCDIR = $(GAMMADIR)/src/os/$(Platform_os_family)/dtrace
JHELPER = jhelper
JHELPER.o = $(JHELPER).o
SRC_JHELPER.o = $(DTRACE_SRCDIR)/$(ARCH).$(JHELPER.o)
JVMOFFS.o = $(JVMOFFS).o
DTRACE_CSUMS = dtrace_csums_OK

# to remove '-g' option which causes link problems
# also '-z nodefs' is used as workaround
GENOFFS_CFLAGS = $(shell echo $(CFLAGS) | sed -e 's/ -g / /g' -e 's/ -g0 / /g';)

endif # ifneq ("${TYPE}", "CORE")

default: $(AOUT) $(LIBJSIG) $(LIBJVM_DB) checkAndBuildSA

# read a generated file defining the set of .o's and the .o .h dependencies
include $(GENERATED)/Dependencies

# read machine-specific adjustments (%%% should do this via buildATree?)
include $(GAMMADIR)/build/solaris/makefiles/$(ARCH).make

launcher.o: launcher.c $(LAUNCHER)/java.c $(LAUNCHER)/java_md.c
	$(CC) -g -c -o $@ launcher.c $(LAUNCHERFLAGS) $(CPPFLAGS)

launcher.c:
	@echo Generating $@
	$(QUIETLY) { \
	echo '#define debug launcher_debug'; \
	echo '#include "java.c"'; \
	echo '#include "java_md.c"'; \
	} > $@

$(AOUT): $(AOUT.o) $(LIBJVM)
	$(QUIETLY) \
	case "$(CFLAGS_BROWSE)" in \
	-sbfast|-xsbfast) \
	    ;; \
	*) \
	    echo Linking launcher...; \
	    $(LINK.CC) -o $@ $(AOUT.o) \
	        -L `pwd` -L $(JAVA_HOME)/jre/lib/$(Platform_gnu_dis_arch) \
	        -L $(JAVA_HOME)/lib/$(Platform_gnu_dis_arch) -l$(JVM) $(LIBS); \
	    ;; \
	esac


ifdef MAPFILE
LIBJVM_MAPFILE = mapfile

ifdef USE_GCC
LFLAGS_VM += -Xlinker -M $(LIBJVM_MAPFILE)
else
LFLAGS_VM += -M $(LIBJVM_MAPFILE)
endif

mapfile : $(MAPFILE) $(REORDERFILE)
	rm -f $@
	cat $^ > $@
endif

# making the library:
$(LIBJVM): $(LIBJVM.o) $(LIBJVM_MAPFILE) $(JVMOFFS.o) $(DTRACE_CSUMS)
	$(QUIETLY) \
	case "$(CFLAGS_BROWSE)" in \
	-sbfast|-xsbfast) \
	    ;; \
	*) \
	    echo Linking vm...; \
	    $(LINK_LIB.CC/PRE_HOOK) \
	    $(LINK_LIB.CC) $(LFLAGS_VM) -o $@ $(LIBJVM.o) $(JVMOFFS.o) $(JHELPER.o) $(LIBS); \
	    $(LINK_LIB.CC/POST_HOOK) \
	    rm -f $@.1; ln -s $@ $@.1; \
	    rm -f $(LN_LIBJVM); ln -s $@ $(LN_LIBJVM); \
	    rm -f $(LN_LIBJVM).1; ln -s $(LN_LIBJVM) $(LN_LIBJVM).1; \
	    ;; \
	esac

ifeq ("${G_SUFFIX}", "_g")
SYMFLAG = -g
FLAG_JVM_G = -DLIBJVM_G
else
SYMFLAG =
FLAG_JVM_G =
endif

ifdef LP64
ARCHFLAG = -xarch=v9
DTRACE_OPTS = -64 -D_LP64
else
ARCHFLAG =
DTRACE_OPTS =
endif

# making libjsig
JSIGSRCDIR = $(GAMMADIR)/src/os/$(Platform_os_family)/vm

$(LIBJSIG): $(JSIGSRCDIR)/jsig.c
	$(QUIETLY) echo Making signal interposition lib...; \
	$(CC) $(SYMFLAG) $(ARCHFLAG) -G -D_REENTRANT $(PICFLAG) -o $@ $(JSIGSRCDIR)/jsig.c \
	    -ldl

# making libjvm_db
INCLS = $(GENERATED)/incls

# Making 64/libjvm_db.so: v9 version of libjvm_db.so which handles 32-bit libjvm.so
ifeq ("${ARCH}", "sparc")

XLIBJVM_DB = 64/$(LIBJVM_DB)

$(XLIBJVM_DB): $(DTRACE_SRCDIR)/$(JVM_DB).c $(JVMOFFS).h
	@echo Making $@
	$(QUIETLY) mkdir -p 64/ ; \
	$(CC) $(SYMFLAG) -xarch=v9 -D$(TYPE) $(FLAG_JVM_G) -I. -I$(GENERATED) \
		-G -D_REENTRANT -KPIC -o $@ $(DTRACE_SRCDIR)/$(JVM_DB).c -lc
endif

lib$(GENOFFS).so: $(DTRACE_SRCDIR)/$(GENOFFS).cpp $(DTRACE_SRCDIR)/$(GENOFFS).h \
                  $(INCLS)/_vmStructs.cpp.incl $(LIBJVM.o)
	$(QUIETLY) $(CCC) $(CPPFLAGS) $(GENOFFS_CFLAGS) -G $(PICFLAG) \
		 -o $@ $(DTRACE_SRCDIR)/$(GENOFFS).cpp

$(GENOFFS): $(DTRACE_SRCDIR)/$(GENOFFS)Main.c lib$(GENOFFS).so
	$(QUIETLY) $(LINK.CC) -z nodefs -o $@ $(DTRACE_SRCDIR)/$(GENOFFS)Main.c \
		./lib$(GENOFFS).so

# $@.tmp is created first. It's to avoid empty $(JVMOFFS).h produced in error case.
$(JVMOFFS).h: $(GENOFFS)
	$(QUIETLY) LD_LIBRARY_PATH=. ./$(GENOFFS) -header > $@.tmp ; touch $@ ; \
	if [ `diff $@.tmp $@ > /dev/null 2>&1; echo $$?` -ne 0 ] ; \
	then rm -f $@; mv $@.tmp $@; \
	else rm -f $@.tmp; \
	fi

$(JVMOFFS)Index.h: $(GENOFFS)
	$(QUIETLY) LD_LIBRARY_PATH=. ./$(GENOFFS) -index > $@.tmp ; touch $@ ; \
	if [ `diff $@.tmp $@ > /dev/null 2>&1; echo $$?` -ne 0 ] ; \
	then rm -f $@; mv $@.tmp $@; \
	else rm -f $@.tmp; \
	fi

$(JVMOFFS).cpp: $(GENOFFS) $(JVMOFFS).h $(JVMOFFS)Index.h
	$(QUIETLY) LD_LIBRARY_PATH=. ./$(GENOFFS) -table > $@.tmp ; touch $@ ; \
	if [ `diff $@.tmp $@ > /dev/null 2>&1; echo $$?` -ne 0 ] ; \
	then rm -f $@; mv $@.tmp $@; \
	else rm -f $@.tmp; \
	fi

$(JVMOFFS.o): $(JVMOFFS).h $(JVMOFFS).cpp $(JVMOFFS).c.csum
	$(QUIETLY) $(CCC) -c -I. -o $@ $(ARCHFLAG) -D$(TYPE) \
		-DJVMOFFS_CSUM="`cat $(JVMOFFS).c.csum`" $(JVMOFFS).cpp

$(LIBJVM_DB): $(DTRACE_SRCDIR)/$(JVM_DB).c $(JVMOFFS.o) $(XLIBJVM_DB)
	@echo Making $@
	$(QUIETLY) $(CC) $(SYMFLAG) $(ARCHFLAG) -D$(TYPE) $(FLAG_JVM_G) -I. -I$(GENERATED) \
		-G -D_REENTRANT $(PICFLAG) -o $@ $(DTRACE_SRCDIR)/$(JVM_DB).c -lc

$(JVMOFFS).c.csum: $(GENOFFS) $(JVMOFFS)Index.h $(JVMOFFS).cpp
	$(QUIETLY) cat $(JVMOFFS)Index.h $(JVMOFFS).cpp | $(SUM) -r | tr -d ' ' > $@

$(JVMOFFS).d.csum: $(GENOFFS) $(JVMOFFS)Index.h $(JHELPER.o)
	$(QUIETLY) $(MCS) -p $(JHELPER.o) | \
		grep JVMOFFS_CSUM | awk '{print $$2}' > $@

$(DTRACE_CSUMS): $(JVMOFFS).c.csum $(JVMOFFS).d.csum
	$(QUIETLY) \
	if [ `diff $(JVMOFFS).c.csum $(JVMOFFS).d.csum > /dev/null 2>&1; echo $$?` -ne 0 ] ; \
	then echo "	WARNING: $(JVMOFFS) check sums don't match. Dtrace support can be broken!"; \
	     echo "	$(SRC_JHELPER.o) has to be re-built on Solaris.Next"; \
	else touch $@ ; \
	fi

# To copy $(SRC_JHELPER.o) from Dtrace source directory
# $(SRC_JHELPER.o) must be preliminary build on Solaris.Next and put under SCCS control
$(JHELPER.o): $(SRC_JHELPER.o)
	$(QUIETLY) rm -f $@; cp $(SRC_JHELPER.o) $@

ifneq ($(DTRACE),)
ifeq ("${OS_ver}", "5.10")

# making the dtrace jhelper DOF:
# $(SRC_JHELPER.o) is kept under SCCS control because it's built on Solaris.Next only
$(SRC_JHELPER.o): $(DTRACE_SRCDIR)/$(JHELPER).d $(JVMOFFS).c.csum
	@echo "DTRACE=$(DTRACE)"
	@echo Compiling $(DTRACE_SRCDIR)/$(JHELPER).d
	$(QUIETLY) \
	$(DTRACE) $(DTRACE_OPTS) -S -C -I. -DMY_JVMOFFS_CSUM=`cat $(JVMOFFS).c.csum` \
		-G -o $@ -s $(DTRACE_SRCDIR)/$(JHELPER).d > $(JHELPER).s 2>&1 ; \
	$(MCS) -a "JVMOFFS_CSUM `cat $(JVMOFFS).c.csum`" $@
endif
endif

# making libsaproc
AGENT_DIR = $(GAMMADIR)/agent
SAPROCSRCDIR = $(AGENT_DIR)/src/os/solaris/proc
SASRCFILE = $(SAPROCSRCDIR)/saproc.cpp
SAMAPFILE = $(SAPROCSRCDIR)/mapfile

# if $(AGENT_DIR) does not exist, we don't build SA
checkAndBuildSA:
	$(QUIETLY) if [ -d $(AGENT_DIR) ] ; then \
	   $(MAKE) -f vm.make $(LIBSAPROC); \
	fi

ifdef USE_GCC
SA_LFLAGS = -Xlinker -M $(SAMAPFILE)
else
SA_LFLAGS = -M $(SAMAPFILE)
endif

$(LIBSAPROC): $(SASRCFILE) $(SAMAPFILE)
	$(QUIETLY) if [ "$(BOOT_JAVA_HOME)" = "" ]; then \
	  echo "ALT_BOOTDIR, BOOTDIR or JAVA_HOME needs to be defined to build SA"; \
	  exit 1; \
	fi
	$(QUIETLY) echo Making SA debugger back-end...;                \
	$(CPP) $(SYMFLAG) $(ARCHFLAG) -G                               \
	  -I$(SAPROCSRCDIR) -I$(GENERATED) -I$(BOOT_JAVA_HOME)/include \
	  -I$(BOOT_JAVA_HOME)/include/solaris $(SA_LFLAGS)             \
          -o $@ $(SASRCFILE)  -ldl -ldemangle 
