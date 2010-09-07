/*
 * @(#)NativeLock.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _NATIVE_LOCK_H_
#define _NATIVE_LOCK_H_

#include "PerfLib.h"


class PERFLIB_API NativeLock {

public:
    ////////////////////////////////////////////////////////////////////////////
    // Acquires a basic synchronization lock.
    //
    virtual void acquire(void) = 0;


    ////////////////////////////////////////////////////////////////////////////
    // Releases a basic synchronization lock.
    //
    virtual void release(void) = 0;


protected:
    ////////////////////////////////////////////////////////////////////////////
    // Need to define an explicit default constructor because of the explicit
    // block on the copy constructor.
    //
    NativeLock(void) {};


    ////////////////////////////////////////////////////////////////////////////
    // Abstract class should always have virtual destructor defined.
    //
    virtual ~NativeLock() {};


private:
    ////////////////////////////////////////////////////////////////////////////
    // Explicitly forbid the use of a copy constructor.
    //
    NativeLock(const NativeLock & src);


    ////////////////////////////////////////////////////////////////////////////
    // Explicitly forbid the use of a copy constructor.
    //
    NativeLock & operator=(const NativeLock & rhs);
};

#endif    // _NATIVE_LOCK_H_
