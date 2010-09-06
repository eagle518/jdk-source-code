# 
# @(#)Makefile	1.4 03/12/23 16:37:38
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

CWD=`pwd`

arch=$(shell uname -p)

ifdef LP64
ifeq ("${arch}", "sparc")
arch		= sparcv9
ARCHFLAG	= -xarch=v9
endif
endif

all: message $(arch) $(arch)/jvm_db_helper $(arch)/jhelper.so

$(arch):
	mkdir -p $(arch)

message:
	echo "building $(arch)"

.PRECIOUS: jvm_db_helper.c

$(arch)/jvm_db_helper: jvm_db_helper.c
	cc $(ARCHFLAG) -o $@ -g -I. $< -lproc -ldl -R.

$(arch)/jhelper.o:
	dtrace -C -G -o $@ -s jhelper.d

$(arch)/jhelper.so: $(arch)/jhelper.o
	cc -G -o $@ $<

clean:
	rm -fr $(arch)/jvm_db_helper $(arch)/jhelper.*
