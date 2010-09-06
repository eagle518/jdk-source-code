@echo off
REM @(#)create.bat	1.11 03/12/23 16:35:33
REM 
REM Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
REM SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
REM 

REM This is the interactive build setup script (as opposed to the batch
REM build execution script). It creates $HotSpotBuildSpace if necessary,
REM copies the appropriate files out of $HotSpotWorkSpace into it, and
REM builds and runs MakeDeps in it. This has the side-effect of creating
REM the vm.dsp file in the buildspace, which is then used in Visual C++.
REM 
REM The generated project file depends upon the include databases. If
REM those are changed then MakeDeps is rerun.

REM
REM Since we don't have uname and we could be cross-compiling,
REM Use the compiler to determine which ARCH we are building
REM 
cl 2>&1 | grep "IA-64" >NUL
if %errorlevel% == 0 goto isia64
cl 2>&1 | grep "AMD64" >NUL
if %errorlevel% == 0 goto amd64
set ARCH=i486
goto end
:amd64
set ARCH=amd64
goto end
:isia64
set ARCH=ia64
:end

setlocal

if "%1" == "core"      goto start
if "%1" == "compiler1" goto start
if "%1" == "compiler2" goto start
goto usage

:start

if "%2" == ""          goto envvar

:cmdline
if not "%5" == ""      goto usage

set HotSpotWorkSpace=%2
set HotSpotBuildSpace=%3
set HotSpotBinDest=%4

goto check

:envvar

echo Using environment variables.

:check

REM Test all variables to see whether the directories they
REM reference exist

if exist %HotSpotWorkSpace% goto test1

echo Error: directory pointed to by HotSpotWorkSpace
echo does not exist, or the variable is not set.
echo.
goto usage

:test1
if exist %HotSpotBuildSpace% goto test2
echo Error: directory pointed to by HotSpotBuildSpace
echo does not exist, or the variable is not set.
echo.
goto usage

:test2
if exist %HotSpotBinDest% goto makedir
echo Error: directory pointed to by HotSpotBinDest
echo does not exist, or the variable is not set.
echo.
goto usage

:makedir
echo NOTE: Using the following settings:
echo   HotSpotWorkSpace=%HotSpotWorkSpace%
echo   HotSpotBuildSpace=%HotSpotBuildSpace%
echo   HotSpotBinDest=%HotSpotBinDest%

if EXIST %HotSpotBuildSpace% goto copyfiles
mkdir %HotSpotBuildSpace%

REM This is now safe to do.
:copyfiles
copy %HotSpotWorkSpace%\build\windows\projectfiles\%1\* %HotSpotBuildSpace%
REM force regneration of vm.dsp
if exist %HotSpotBuildSpace%\vm.dsp del %HotSpotBuildSpace%\vm.dsp

echo # Generated file!                                                 >    %HotSpotBuildSpace%\local.make
echo # Changing a variable below and then deleting vm.dsp will cause  >>    %HotSpotBuildSpace%\local.make
echo # vm.dsp to be regenerated with the new values.  Changing the    >>    %HotSpotBuildSpace%\local.make
echo # version requires rerunning create.bat.                         >>    %HotSpotBuildSpace%\local.make
echo.                                      >>    %HotSpotBuildSpace%\local.make
echo HOTSPOTWORKSPACE=%HotSpotWorkSpace%   >>    %HotSpotBuildSpace%\local.make
echo HOTSPOTBUILDSPACE=%HotSpotBuildSpace% >>    %HotSpotBuildSpace%\local.make
echo HOTSPOTBINDEST=%HotSpotBinDest%       >>    %HotSpotBuildSpace%\local.make

REM Build MakeDeps in the buildspace

pushd %HotSpotBuildSpace%
nmake
popd

goto end

:usage
echo Usage: create version { HotSpotWorkSpace HotSpotBuildSpace HotSpotBinDest }
echo.
echo where version is "core", "compiler1" or "compiler2".
echo.
echo This is the interactive build setup script (as opposed to the batch
echo build execution script). It creates HotSpotBuildSpace if necessary,
echo copies the appropriate files out of HotSpotWorkSpace into it, and
echo builds and runs MakeDeps in it. This has the side-effect of creating
echo the vm.dsp file in the build space, which is then used in Visual C++.
echo.
echo The generated project file depends upon the include databases. If
echo those are changed then MakeDeps is rerun.
echo.
echo NOTE that it is now NOT safe to modify any of the files in the build
echo space, since they may be overwritten whenever this script is run or
echo nmake is run in that directory.

:end

endlocal
