@echo off
REM
REM Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
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

REM check for .\sa.jar, if it does not exist
REM assume that we are in build configuration.

if not exist .\sa.jar goto IN_BUILD_CONF
set CLASSPATH=.\sa.jar
goto EXEC_CMD

:IN_BUILD_CONF
set CLASSPATH=..\build\classes

:EXEC_CMD
start rmiregistry -J-Xbootclasspath/p:%CLASSPATH%
