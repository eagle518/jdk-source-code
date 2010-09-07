#
# Copyright (c) 2003, 2009, Oracle and/or its affiliates. All rights reserved.
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

# These are the commands used externally to compile and run.

!ifdef BootStrapDir
RUN_JAVA=$(BootStrapDir)\bin\java
RUN_JAVAP=$(BootStrapDir)\bin\javap
RUN_JAVAH=$(BootStrapDir)\bin\javah
RUN_JAR=$(BootStrapDir)\bin\jar
COMPILE_JAVAC=$(BootStrapDir)\bin\javac $(BOOTSTRAP_JAVAC_FLAGS)
COMPILE_RMIC=$(BootStrapDir)\bin\rmic
BOOT_JAVA_HOME=$(BootStrapDir)
!else
RUN_JAVA=java
RUN_JAVAP=javap
RUN_JAVAH=javah
RUN_JAR=jar
COMPILE_JAVAC=javac $(BOOTSTRAP_JAVAC_FLAGS)
COMPILE_RMIC=rmic
BOOT_JAVA_HOME=
!endif

# Settings for javac
BOOT_SOURCE_LANGUAGE_VERSION=6
BOOT_TARGET_CLASS_VERSION=6
JAVAC_FLAGS=-g -encoding ascii
BOOTSTRAP_JAVAC_FLAGS=$(JAVAC_FLAGS) -source $(BOOT_SOURCE_LANGUAGE_VERSION) -target $(BOOT_TARGET_CLASS_VERSION)

ProjectFile=vm.vcproj

!if "$(MSC_VER)" == "1200"

VcVersion=VC6
ProjectFile=vm.dsp

!elseif "$(MSC_VER)" == "1400"

VcVersion=VC8

!elseif "$(MSC_VER)" == "1500"

VcVersion=VC9

!else

VcVersion=VC7

!endif
