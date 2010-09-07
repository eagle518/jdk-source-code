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

call saenv.bat

REM set the environment variable JCORE_PACKAGES to comman separated list of
REM packages whose classes have to be retrieved from the core file.

%SA_JAVA_CMD% -Dsun.jvm.hotspot.tools.jcore.filter=sun.jvm.hotspot.tools.jcore.PackageNameFilter -Dsun.jvm.hotspot.tools.jcore.PackageNameFilter.pkgList=%JCORE_PACKAGES% sun.jvm.hotspot.tools.jcore.ClassDump %1 %2


