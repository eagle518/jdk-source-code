#
# Copyright (c) 2002, 2009, Oracle and/or its affiliates. All rights reserved.
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

ARCH := $(shell if ([ `uname -m` = "ia64" ])  ; then echo ia64 ; elif ([ `uname -m` = "x86_64" ]) ; then echo amd64; elif ([ `uname -m` = "sparc64" ]) ; then echo sparc; else echo i386 ; fi )
GCC      = gcc

JAVAH    = ${JAVA_HOME}/bin/javah

SOURCES  = salibelf.c   \
        symtab.c        \
	libproc_impl.c  \
	ps_proc.c       \
	ps_core.c       \
	LinuxDebuggerLocal.c 

INCLUDES = -I${JAVA_HOME}/include -I${JAVA_HOME}/include/linux 

OBJS     = $(SOURCES:.c=.o)

LIBS     = -lthread_db

CFLAGS   = -c -fPIC -g -D_GNU_SOURCE -D$(ARCH) $(INCLUDES)

LIBSA = $(ARCH)/libsaproc.so

all: $(LIBSA)

LinuxDebuggerLocal.o: LinuxDebuggerLocal.c
	$(JAVAH) -jni -classpath ../../../build/classes  \
		sun.jvm.hotspot.debugger.x86.X86ThreadContext \
		sun.jvm.hotspot.debugger.sparc.SPARCThreadContext \
		sun.jvm.hotspot.debugger.amd64.AMD64ThreadContext 
	$(GCC) $(CFLAGS) $<

.c.obj:
	$(GCC) $(CFLAGS)

ifndef LDNOMAP
  LFLAGS_LIBSA = -Xlinker --version-script=mapfile
endif

# If this is a --hash-style=gnu system, use --hash-style=both
#   The gnu .hash section won't work on some Linux systems like SuSE 10.
_HAS_HASH_STYLE_GNU:=$(shell $(CC) -dumpspecs | grep -- '--hash-style=gnu')
ifneq ($(_HAS_HASH_STYLE_GNU),)
  LDFLAGS_HASH_STYLE = -Wl,--hash-style=both
endif
LFLAGS_LIBSA += $(LDFLAGS_HASH_STYLE)

$(LIBSA): $(OBJS) mapfile
	if [ ! -d $(ARCH) ] ; then mkdir $(ARCH) ; fi
	$(GCC) -shared $(LFLAGS_LIBSA) -o $(LIBSA) $(OBJS) $(LIBS)

test.o: test.c
	$(GCC) -c -o test.o -g -D_GNU_SOURCE -D$(ARCH) $(INCLUDES) test.c

test: test.o
	$(GCC) -o test test.o -L$(ARCH) -lsaproc $(LIBS)

clean:
	rm -rf $(LIBSA)
	rm -rf $(OBJS)
	rmdir $(ARCH)

