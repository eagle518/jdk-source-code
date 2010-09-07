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

REM Cross compile IA64 compiler2 VM
REM Usage:
REM cross_compile flavor workspace bootstrap_dir [build_id]
REM                 %1       %2             %3      %4
REM
REM Set current directory
for /F %%i in ('cd') do set CD=%%i
echo Setting up Visual C++ Compilation Environment
if "%MSVCDir%" == "" goto setdir1
goto setenv1
:setdir1
SET MSVCDir=C:\Program Files\Microsoft Visual Studio\VC98
:setenv1
SET OLDINCLUDE=%INCLUDE%
SET OLDLIB=%LIB%
SET OLDPATH=%PATH%
call "%MSVCDir%\Bin\VCVARS32"
call %2\make\windows\build %1 adlc %2 %3 %4
SET INCLUDE=%OLDINCLUDE%
SET LIB=%OLDLIB%
SET PATH=%OLDPATH%
echo Setting up 64-BIT Compilation Environment
if "%MSSdk%" == "" goto setdir2
goto setenv2
:setdir2
SET MSSdk=C:\Program Files\Microsoft SDK
:setenv2
call "%MSSdk%\SetEnv.bat" /XP64
SET ALT_ADLC_PATH=%CD%\windows_i486_compiler2\generated
call %2\make\windows\build %1 compiler2 %2 %3 %4
SET INCLUDE=%OLDINCLUDE%
SET LIB=%OLDLIB%
SET PATH=%OLDPATH%
SET OLDINCLUDE=
SET OLDLIB=
SET OLDPATH=
