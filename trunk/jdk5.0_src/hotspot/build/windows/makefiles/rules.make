# @(#)rules.make	1.6 03/12/23 16:35:35
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# These are the commands used externally to compile and run.

!ifdef BootStrapDir
RUN_JAVA=$(BootStrapDir)\bin\java
RUN_JAVAP=$(BootStrapDir)\bin\javap
RUN_JAVAH=$(BootStrapDir)\bin\javah
RUN_JAR=$(BootStrapDir)\bin\jar
COMPILE_JAVAC=$(BootStrapDir)\bin\javac
COMPILE_RMIC=$(BootStrapDir)\bin\rmic
BOOT_JAVA_HOME=$(BootStrapDir)
!else
RUN_JAVA=java
RUN_JAVAP=javap
RUN_JAVAH=javah
RUN_JAR=jar
COMPILE_JAVAC=javac
COMPILE_RMIC=rmic
BOOT_JAVA_HOME=
!endif
