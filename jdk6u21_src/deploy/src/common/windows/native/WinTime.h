/*
 * @(#)WinTime.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _WIN_TIME_H_
#define _WIN_TIME_H_

#include <windows.h>
#include "SystemTime.h"
#include "PerfLib.h"


class PERFLIB_API WinTime : public SystemTime {

public:
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
    virtual jlong getCurrentTime(void) const;

    WinTime(void)
        : m_javaOrigin(calculateJavaEpoch()) {
        // everything done in init
    }

private:
    ////////////////////////////////////////////////////////////////////////////
    // Caculates the start of the Java epoch (Midnight, January 1st, 1970).  The
    // value returned by this method is a 64-bit integer, which can be cast
    // to/from a Windows 32-bit API FILETIME type.  This number can then be
    // subtracted from times specified in the Windows epoch to get a number
    // relative to the Java epoch.
    //
    // @return the start of the Java epoch relative to the Windows epoch.
    static jlong calculateJavaEpoch(void);

    jlong m_javaOrigin;
};

#endif    // _WIN_TIME_H_
