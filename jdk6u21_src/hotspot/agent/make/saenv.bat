@echo off
REM
REM Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
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

REM This is common environment settings for all SA
REM windows batch scripts

REM set jre\bin and jre\bin\client (or server) in PATH
REM WINDBG_HOME must point to the Windows Debugging Tools
REM installation directory

if "%SA_JAVA%" == "" goto no_sa_java

goto sa_java_set

:no_sa_java
set SA_JAVA=java

:sa_java_set

set SA_CLASSPATH=..\build\classes;..\src\share\lib\js.jar;sa.jar;lib\js.jar

set SA_LIBPATH=..\src\os\win32\windbg\i386;.\win32\i386

set OPTIONS=-Dsun.jvm.hotspot.debugger.useWindbgDebugger 
set OPTIONS=-Dsun.jvm.hotspot.debugger.windbg.imagePath="%PATH%" %OPTIONS%
set OPTIONS=-Dsun.jvm.hotspot.debugger.windbg.sdkHome="%WINDBG_HOME%" %OPTIONS%

if "%SA_DISABLE_VERS_CHK%" == "" goto vers_chk
set OPTIONS="-Dsun.jvm.hotspot.runtime.VM.disableVersionCheck %OPTIONS%"

:vers_chk
set SA_JAVA_CMD=%SA_JAVA% -showversion -cp %SA_CLASSPATH% -Djava.library.path=%SA_LIBPATH% %OPTIONS% 
