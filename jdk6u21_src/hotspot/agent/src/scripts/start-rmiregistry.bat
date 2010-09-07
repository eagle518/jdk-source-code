@echo off

REM
REM Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
REM ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM
REM  
REM

@echo off

if "%1" == "-help" goto usage

:JAVA_HOME
if not exist %JAVA_HOME%\bin\rmiregistry goto BADJAVAHOME
if not exist %JAVA_HOME%\lib\sa-jdi.jar goto BADJAVAHOME

start %JAVA_HOME%\bin\rmiregistry -J-Xbootclasspath/p:%JAVA_HOME%\lib\sa-jdi.jar
goto end

:BADJAVAHOME
echo JAVA_HOME does not point to a working J2SE 1.5 installation.

:usage
@echo usage: start-rmiregistry
@echo  Start the rmi registry with with sa-jdi.jar on the bootclasspath
@echo  for use by the debug server.
@echo  JAVA_HOME must contain the pathname of a J2SE 1.5 installation.

:end

