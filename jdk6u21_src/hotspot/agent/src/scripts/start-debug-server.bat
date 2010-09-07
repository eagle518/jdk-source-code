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

if "%1" == "-help" goto usage

:JAVA_HOME
if not exist %JAVA_HOME%\bin\java.exe goto BADJAVAHOME
if not exist %JAVA_HOME\lib\sa-jdi.jar goto BADJAVAHOME

start %JAVA_HOME%\bin\java -classpath %JAVA_HOME%\lib\sa-jdi.jar sun.jvm.hotspot.jdi.SADebugServer %1 %2 
goto end

:BADJAVAHOME
echo JAVA_HOME does not point to a working J2SE 1.5 installation.

:usage
echo Usage: start-debug-server [pid]
echo        $0 <java executable> [Dr Watson dump file]
echo  Start the JDI debug server on [pid] or [Dr Watson dump file]
echo  so that it can be debugged from a remote machine.
echo  JAVA_HOME must contain the pathname of a J2SE 1.5
echo  installation.

:end
