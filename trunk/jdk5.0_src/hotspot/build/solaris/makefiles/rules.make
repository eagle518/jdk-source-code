# 
# @(#)rules.make	1.16 04/02/28 11:14:59
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# Common rules/macros for the vm, adlc.

# Tell make that .cpp is important
.SUFFIXES: .cpp $(SUFFIXES)

# For now.  Other makefiles use CPP as the c++ compiler, but that should really
# name the preprocessor.
ifeq	($(CCC),)
CCC		= $(CPP)
endif

DEMANGLER	= c++filt
DEMANGLE	= $(DEMANGLER) < $@ > .$@ && mv -f .$@ $@

# $(CC) is the c compiler (cc/gcc), $(CCC) is the c++ compiler (CC/g++).
C_COMPILE	= $(CC) $(CPPFLAGS) $(CFLAGS)
CC_COMPILE	= $(CCC) $(CPPFLAGS) $(CFLAGS)

AS.S		= $(AS)

COMPILE.c	= $(C_COMPILE) -c
GENASM.c	= $(C_COMPILE) -S
LINK.c		= $(CC) $(LFLAGS) $(AOUT_FLAGS) $(PROF_AOUT_FLAGS)
LINK_LIB.c	= $(CC) $(LFLAGS) -G
PREPROCESS.c	= $(C_COMPILE) -E

COMPILE.CC	= $(CC_COMPILE) -c
GENASM.CC	= $(CC_COMPILE) -S
LINK.CC		= $(CCC) $(LFLAGS) $(AOUT_FLAGS) $(PROF_AOUT_FLAGS)
LINK_NOPROF.CC	= $(CCC) $(LFLAGS) $(AOUT_FLAGS)
LINK_LIB.CC	= $(CCC) $(LFLAGS) -G
PREPROCESS.CC	= $(CC_COMPILE) -E

# Effect of REMOVE_TARGET is to delete out-of-date files during "gnumake -k".
REMOVE_TARGET   = rm -f $@

# Synonyms.
COMPILE.cpp	= $(COMPILE.CC)
GENASM.cpp	= $(GENASM.CC)
LINK.cpp	= $(LINK.CC)
LINK_LIB.cpp	= $(LINK_LIB.CC)
PREPROCESS.cpp	= $(PREPROCESS.CC)

# Note use of ALT_BOOTDIR to explicitly specify location of java and
# javac; this is the same environment variable used in the J2SE build
# process for overriding the default spec, which is BOOTDIR.
# Note also that we fall back to using JAVA_HOME if neither of these is
# specified.

ifdef ALT_BOOTDIR

RUN.JAVA  = $(ALT_BOOTDIR)/bin/java
RUN.JAVAP = $(ALT_BOOTDIR)/bin/javap
RUN.JAVAH = $(ALT_BOOTDIR)/bin/javah
RUN.JAR   = $(ALT_BOOTDIR)/bin/jar
COMPILE.JAVAC = $(ALT_BOOTDIR)/bin/javac
COMPILE.RMIC = $(ALT_BOOTDIR)/bin/rmic
BOOT_JAVA_HOME = $(ALT_BOOTDIR)
else

ifdef BOOTDIR
RUN.JAVA  = $(BOOTDIR)/bin/java
RUN.JAVAP = $(BOOTDIR)/bin/javap
RUN.JAVAH = $(BOOTDIR)/bin/javah
RUN.JAR   = $(BOOTDIR)/bin/jar
COMPILE.JAVAC = $(BOOTDIR)/bin/javac
COMPILE.RMIC  = $(BOOTDIR)/bin/rmic
BOOT_JAVA_HOME = $(BOOTDIR)
else

ifdef JAVA_HOME
RUN.JAVA  = $(JAVA_HOME)/bin/java
RUN.JAVAP = $(JAVA_HOME)/bin/javap
RUN.JAVAH = $(JAVA_HOME)/bin/javah
RUN.JAR   = $(JAVA_HOME)/bin/jar
COMPILE.JAVAC = $(JAVA_HOME)/bin/javac
COMPILE.RMIC  = $(JAVA_HOME)/bin/rmic
BOOT_JAVA_HOME = $(JAVA_HOME)
else

# take from the PATH, if ALT_BOOTDIR, BOOTDIR and JAVA_HOME are not defined
# note that this is to support hotspot build without SA. To build
# SA along with hotspot, you need to define ALT_BOOTDIR, BOOTDIR or JAVA_HOME

RUN.JAVA  = java
RUN.JAVAP = javap
RUN.JAVAH = javah
RUN.JAR   = jar
COMPILE.JAVAC = javac
COMPILE.RMIC  = rmic

endif
endif
endif

# 'gmake MAKE_VERBOSE=y' or 'gmake QUIETLY=' gives all the gory details.
QUIETLY$(MAKE_VERBOSE)	= @

# With parallel makes, print a message at the end of compilation.
ifeq	($(findstring j,$(MFLAGS)),j)
COMPILE_DONE	= && { echo Done with $<; }
endif

%.o: %.cpp
	@echo Compiling $<
	$(QUIETLY) $(REMOVE_TARGET)
	$(QUIETLY) $(COMPILE.CC) -o $@ $< $(COMPILE_DONE)

%.o: %.s
	@echo Assembling $<
	$(QUIETLY) $(REMOVE_TARGET)
	$(QUIETLY) $(AS.S) -o $@ $< $(COMPILE_DONE)

%.s: %.cpp
	@echo Generating assembly for $<
	$(QUIETLY) $(GENASM.CC) -o $@ $<
	$(QUIETLY) $(DEMANGLE) $(COMPILE_DONE)

# Intermediate files (for debugging macros)
%.i: %.cpp
	@echo Preprocessing $< to $@
	$(QUIETLY) $(PREPROCESS.CC) $< > $@ $(COMPILE_DONE)

#  Override gnumake built-in rules which do sccs get operations badly.
#  (They put the checked out code in the current directory, not in the
#  directory of the original file.)  Since this is a symptom of a teamware
#  failure, and since not all problems can be detected by gnumake due
#  to incomplete dependency checking... just complain and stop.
%:: s.%
	@echo "========================================================="
	@echo File $@
	@echo is out of date with respect to its SCCS file.
	@echo This file may be from an unresolved Teamware conflict.
	@echo This is also a symptom of a Teamware bringover/putback failure
	@echo in which SCCS files are updated but not checked out.
	@echo Check for other out of date files in your workspace.
	@echo "========================================================="
	@exit 666

%:: SCCS/s.%
	@echo "========================================================="
	@echo File $@
	@echo is out of date with respect to its SCCS file.
	@echo This file may be from an unresolved Teamware conflict.
	@echo This is also a symptom of a Teamware bringover/putback failure
	@echo in which SCCS files are updated but not checked out.
	@echo Check for other out of date files in your workspace.
	@echo "========================================================="
	@exit 666

.PHONY: default
