# @(#)rules.make	1.8 03/12/23 16:35:20
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

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
