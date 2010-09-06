# 
# @(#)vm.make	1.30 04/03/08 21:20:34
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# This makefile (vm.make) is included from the vm.make in the
# build directories.
# It knows how to compile and link the VM.

# It assumes the following flags are set:
# CFLAGS Platform_file, Src_Dirs, SYSDEFS, AOUT, LIB, Obj_Files, Lib_H_Files, Lib_C_Files

# And it assumes that the deps and incls have already been built.

# -- D. Ungar (5/97) from a file by Bill Bush

# build whole VM by default, I can also build foo.o or foo.i

AOUT   = gamma$(G_SUFFIX)

ARCH = $(Platform_arch)

JSIG = jsig$(G_SUFFIX)
LIBJSIG = lib$(JSIG).so

SAPROC = saproc$(G_SUFFIX)
LIBSAPROC = lib$(SAPROC).so

default: $(AOUT) $(LIBJSIG) checkAndBuildSA

GENERATED  = ../generated

# read a generated file defining the set of .o's and the .o .h dependencies

include $(GENERATED)/Dependencies
Obj_Files += $(EXTRA_OBJS)


# read machine-specific adjustments (%%% should do this via buildATree?)

include $(GAMMADIR)/build/linux/makefiles/$(Platform_arch).make

# tell make that .cpp is important

.SUFFIXES: .cpp $(SUFFIXES)


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

# Bill had this, I don't know why
# SYSDEFS += -Dvolatile=''

# Bill had this, I don't know why
# SYSDEFS += -DNOASM

BUILD_VERSION = -DHOTSPOT_BUILD_VERSION="\"$(HOTSPOT_BUILD_VERSION)\""
BUILD_VERSION$(HOTSPOT_BUILD_VERSION) = 

CPPFLAGS = ${SYSDEFS} ${INCLUDES} ${BUILD_VERSION}

# Suppress warnings (for now)
CFLAGS += -w

# do not include shared lib path in a.outs
#AOUT_FLAGS += -norunpath

COMPILE   = ${CPP} ${CPPFLAGS} ${CFLAGS}
LINK_LIB  = ${CPP} -shared -mimpure-text
LINK      = ${CPP} ${AOUT_FLAGS}


COMPILE.s = ${COMPILE} -S
COMPILE.o = ${COMPILE} -c
COMPILE.i = ${COMPILE} -E

verbose-echo = echo
verbose-echo$(MAKE_VERBOSE) = :

# Compiling

# Generate Disassembly for Inspection
%.s: %.cpp
	@echo Compiling $< ; \
	rm -f $@ ; \
	$(verbose-echo) \
	${COMPILE.s} $< ; \
	${COMPILE.s} $<;  \
	rm -f c++filt.out;	\
	cat $@ | c++filt > c++filt.out; \
	rm -f $@;	\
	mv c++filt.out $@

%.o: %.cpp
	@echo Compiling $< ; \
	rm -f $@ ; \
	$(verbose-echo) \
	${COMPILE.o} $< ; \
	${COMPILE.o} $< && \
	case "$(MFLAGS)" in *j*) echo Done with $<;; esac

%.o: %.S
	@echo Assembling-cpp $< ; \
	rm -f $@ ; \
	$(verbose-echo) \
	${COMPILE.o} $< ; \
	${COMPILE.o} $< && \
	case "$(MFLAGS)" in *j*) echo Done with $<;; esac

%.o: %.s
	@echo Assembling $< ; \
	rm -f $@ ; \
	$(verbose-echo) \
	${COMPILE.o} $< ; \
	${COMPILE.o} $< && \
	case "$(MFLAGS)" in *j*) echo Done with $<;; esac

# Intermediate files (for debugging macros)

%.i: %.cpp
	@echo Preprocessing $< to $@; \
	$(verbose-echo) \
	${COMPILE.i} $< >$@ ; \
	${COMPILE.i} $< >$@


# The whole megilla:

LIBS += -lm -ldl -lpthread

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
LAUNCHERFLAGS = -I$(LAUNCHER) -I$(GAMMADIR)/src/share/vm/prims
launcher.o: launcher.c $(LAUNCHER)/java.c $(LAUNCHER)/java_md.c
	$(CC) -g -c -o $@ launcher.c $(LAUNCHERFLAGS) $(CPPFLAGS)

# Figure out how to turn this off:
#LAUNCHERFLAGS += -DOLDJAVA


launcher.c:
	@rm -f $@ $@+
	echo '#define debug launcher_debug'          >>$@+
	echo '#include "java.c"'                     >>$@+
	echo '#include "java_md.c"'                  >>$@+
	mv $@+ $@

$(AOUT): $(AOUT.o) $(LIBJVM)
	@echo Linking launcher... ; \
	$(verbose-echo) \
	${LINK} -o $@ ${AOUT_FLAGS} $(AOUT.o) -L `pwd` -L ${JAVA_HOME}/jre/lib/$(Platform_gnu_dis_arch) -L ${JAVA_HOME}/lib/$(Platform_gnu_dis_arch) -l$(JVM) $(LIBS) ; \
	${LINK} -o $@ ${AOUT_FLAGS} $(AOUT.o) -L `pwd` -L ${JAVA_HOME}/jre/lib/$(Platform_gnu_dis_arch) -L ${JAVA_HOME}/lib/$(Platform_gnu_dis_arch) -l$(JVM) $(LIBS)

ifndef LDNOMAP
  ifdef MAPFILE
    LFLAGS_VM += -Xlinker --version-script=$(MAPFILE)
  endif
endif

# Set soname, see 5009832
LFLAGS_VM += -Xlinker -soname=$(LIBJVM)

# making the library:

LIBJVM.o = $(LIBJVM.o/LINK_INTO_$(LINK_INTO))
LIBJVM.o/LINK_INTO_AOUT    =
LIBJVM.o/LINK_INTO_LIBJVM  = $(Obj_Files)


$(LIBJVM): $(LIBJVM.o) $(MAPFILE)
	echo Linking vm... ; \
	${LINK_LIB} ${LIB_FLAGS} ${LFLAGS_VM} -z noversion -o $@ $(LIBJVM.o) $(LIBS)
ifdef STRIP
	echo Stripping vm... ; \
	$(STRIP) $(LIBJVM)
endif
#	@rm -f $@.1; ln -s $@ $@.1
#	@rm -f ${LN_LIBJVM}; ln -s $@ ${LN_LIBJVM}
#	@rm -f ${LN_LIBJVM}.1; ln -s ${LN_LIBJVM} ${LN_LIBJVM}.1

# making libjsig

JSIGSRCDIR = $(GAMMADIR)/src/os/$(Platform_os_family)/vm
ifeq ("${G_SUFFIX}", "_g")
SYMFLAG = -g
else
SYMFLAG =
endif

$(LIBJSIG): $(JSIGSRCDIR)/jsig.c
	echo Making signal interposition lib...; \
	$(CC) $(SYMFLAG) -D_GNU_SOURCE -D_REENTRANT $(LIB_FLAGS) -shared -fPIC -o $@ $(JSIGSRCDIR)/jsig.c -ldl

include $(GAMMADIR)/build/linux/makefiles/rules.make

# making libsaproc
AGENT_DIR = $(GAMMADIR)/agent
SASRCDIR = $(AGENT_DIR)/src/os/linux

SASRCFILES =  $(SASRCDIR)/salibelf.c                   \
              $(SASRCDIR)/symtab.c                     \
              $(SASRCDIR)/libproc_impl.c               \
              $(SASRCDIR)/ps_proc.c                    \
              $(SASRCDIR)/ps_core.c                    \
              $(SASRCDIR)/LinuxDebuggerLocal.c         

# if $(AGENT_DIR) does not exist, we don't build SA
# also, we don't build SA on Itanium.

checkAndBuildSA:
	if [ -d $(AGENT_DIR) -a "$(ARCH)" != "ia64" ] ; then \
	   $(MAKE) -f vm.make $(LIBSAPROC); \
	fi

ifndef LDNOMAP
  SAMAPFILE=$(SASRCDIR)/mapfile
  SA_LFLAGS = -Xlinker --version-script=$(SAMAPFILE)
endif

$(LIBSAPROC): $(SASRCFILES) $(SAMAPFILE)
	if [ "$(BOOT_JAVA_HOME)" = "" ]; then \
	   echo "ALT_BOOTDIR, BOOTDIR or JAVA_HOME needs to be defined to build SA"; \
	   exit 1; \
	fi
	echo Making SA debugger back-end...;             \
	$(CC) $(SYMFLAG) -shared -fPIC                   \
		-D$(ARCH) -D_GNU_SOURCE                  \
		-I$(GENERATED)                           \
		-I$(BOOT_JAVA_HOME)/include              \
		-I$(BOOT_JAVA_HOME)/include/linux        \
		$(SASRCFILES)                            \
                $(SA_LFLAGS) -o $@                       \
		-lthread_db

.PHONY: default
