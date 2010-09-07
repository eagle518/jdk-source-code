/*
 * @(#)UnixTime.cpp	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <sys/time.h>
#include "UnixTime.h"

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
jlong UnixTime::getCurrentTime(void) const {
    timeval t;
    jlong   result;

    gettimeofday(&t, NULL);

    // convert to in milliseconds and return
    result = ((jlong(t.tv_sec) * 1000) + (jlong(t.tv_usec) / 1000));

    return (result);
}
