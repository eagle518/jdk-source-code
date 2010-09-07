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

call saenv64.bat

REM check for .\sa.jar, if it does not exist
REM assume that we are in build configuration.

if not exist .\sa.jar goto IN_BUILD_CONF
set SA_CLASSPATH=.\sa.jar
goto EXEC_CMD

:IN_BUILD_CONF
set SA_CLASSPATH=..\build\classes

:EXEC_CMD
%SA_JAVA% -classpath %SA_CLASSPATH% -Djava.rmi.server.codebase=file:/%SA_CLASSPATH% -Djava.security.policy=grantAll.policy -Djava.library.path=%SA_LIBPATH% %OPTIONS% sun.jvm.hotspot.DebugServer %1 %2 %3 %4 %5 %6 %7 %8 %9
