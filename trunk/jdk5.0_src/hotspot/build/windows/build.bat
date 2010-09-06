@echo off
REM @(#)build.bat	1.27 03/12/23 16:35:32
REM 
REM Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
REM SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
REM 


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

if "%3" == ""          goto usage
if not "%6" == ""      goto usage
if "%1" == "core"      goto test2
if "%1" == "compiler1" goto test2
if "%1" == "compiler2" goto test2
if "%1" == "docs"      goto test2
if "%1" == "adlc"      goto build_adlc

goto usage

:test2
REM check_j2se_version
REM jvmti.make requires J2SE 1.4.x or newer.
REM If not found then fail fast.
REM
%3\bin\java -fullversion 2>&1 | grep "1\.[45]"
if %errorlevel% == 0 goto build
echo.
echo J2SE version found at %3\bin\java:
%3\bin\java -version
echo.
echo An XSLT processor (J2SE 1.4.x or newer) is required to
echo bootstrap this build
echo.

goto usage

:build
REM Add a DEVELOP=1 to the line below to make RELEASE builds instead of PRODUCT builds
REM if you want to build SA, pass %5 to be the homedir of Microsoft Debugging Tools installation.
if not "%5" == "" goto BuildSA
nmake -f %2/build/windows/build.make Variant=%1 WorkSpace=%2 BootStrapDir=%3 BuildID=%4 BuildUser="%USERNAME%" 
goto end
:BuildSA
echo "building SA with HotSpot.."
nmake -f %2/build/windows/build.make Variant=%1 WorkSpace=%2 BootStrapDir=%3 BuildID=%4 BuildUser="%USERNAME%" WindbgHome=%5
goto end

:build_adlc
nmake -f %2/build/windows/build.make Variant=compiler2 WorkSpace=%2 BootStrapDir=%3 BuildID=%4 BuildUser="%USERNAME%" ADLC_ONLY=1
goto end

:usage
echo Usage: build version workspace bootstrap_dir [build_id] [windbg_home]
echo.
echo where version is "docs", "core", "compiler1" or "compiler2", workspace is
echo source directory without trailing slash, bootstrap_dir is a full path to
echo a JDK in which bin/java and bin/javac are present and working, and
echo build_id is an optional build identifier displayed by java -version

:end
