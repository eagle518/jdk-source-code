/*
 * @(#)UnixTime.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _UNIX_TIME_H_
#define _UNIX_TIME_H_

#include "SystemTime.h"
#include "PerfLib.h"


class PERFLIB_API UnixTime : public SystemTime {

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
};

#endif    // _UNIX_TIME_H_
