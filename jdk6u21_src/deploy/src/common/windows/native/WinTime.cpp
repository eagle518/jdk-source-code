/*
 * @(#)WinTime.cpp	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "WinTime.h"

////////////////////////////////////////////////////////////////////////////
// Gets the current system time in milliseconds.  The time is given as a
// 64-bit signed integer that represents the number of milliseconds since
// the start of the Java epoch (i.e. since Midnight, January 1st, 1970).
//
// This method duplicates the functionality the JVM provides for the
// java.lang.System.currentTimeMillis method.
//
// @return the current system time in milliseconds.
//
jlong WinTime::getCurrentTime(void) const {
    SYSTEMTIME system_time;
    jlong      file_time;
    jlong      result;

    // get the current time and convert it into a FILETIME
    ::GetSystemTime(&system_time);
    ::SystemTimeToFileTime(&system_time, (FILETIME *) &file_time);

    // convert to in milliseconds since the Java origin
    result = ((file_time - m_javaOrigin) / 10000);

    return (result);
}


////////////////////////////////////////////////////////////////////////////
// Caculates the start of the Java epoch (Midnight, January 1st, 1970).  The
// value returned by this method is a 64-bit integer, which can be cast
// to/from a Windows 32-bit API FILETIME type.  This number can then be
// subtracted from times specified in the Windows epoch to get a number
// relative to the Java epoch.
//
// @return the start of the Java epoch relative to the Windows epoch.
jlong WinTime::calculateJavaEpoch(void) {
    SYSTEMTIME java_origin;
    jlong      result = 0;

    // create a time structure with the start of the Java origin
    java_origin.wYear          = 1970;
    java_origin.wMonth         = 1;
    java_origin.wDayOfWeek     = 0; // ignored
    java_origin.wDay           = 1;
    java_origin.wHour          = 0;
    java_origin.wMinute        = 0;
    java_origin.wSecond        = 0;
    java_origin.wMilliseconds  = 0;

    // converting it into a FILETIME will give the time of the Java origin as
    // the number of 100-nanosecond intervals since January 1, 1601 (i.e. the
    // Windows origin)
    ::SystemTimeToFileTime(&java_origin, (FILETIME *) &result);

    return (result);
}
