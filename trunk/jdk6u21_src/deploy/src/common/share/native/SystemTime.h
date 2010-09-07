/*
 * @(#)SystemTime.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _SYSTEM_TIME_H_
#define _SYSTEM_TIME_H_

#include "jni.h"
#include "PerfLib.h"


class PERFLIB_API SystemTime {

public:

    ////////////////////////////////////////////////////////////////////////////
    // Gets the current system time in milliseconds.  The time is given as a
    // 64-bit signed integer that represents the number of milliseconds since
    // the start of the Java epoch (i.e. since Midnight, January 1st, 1970).
    //
    // Override this method to provide a platform specific implementation.  The
    // subclass should implement the method in a maner that is equivalent to the
    // functionality the JVM provides for the java.lang.System.currentTimeMillis
    // method.
    //
    // @return the current system time in milliseconds.
    //
    virtual jlong getCurrentTime(void) const = 0;


    ////////////////////////////////////////////////////////////////////////////
    // Abstract class should always have virtual destructor defined.
    //
    virtual ~SystemTime() {};
};

#endif    // _SYSTEM_TIME_H_
