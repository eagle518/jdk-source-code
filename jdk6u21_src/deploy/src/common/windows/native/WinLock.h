/*
 * @(#)WinLock.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _WIN_LOCK_H_
#define _WIN_LOCK_H_

#include <windows.h>
#include "NativeLock.h"
#include "PerfLib.h"


class PERFLIB_API WinLock : public NativeLock {

public:
    ////////////////////////////////////////////////////////////////////////////
    // Acquires a basic synchronization lock.
    //
    virtual void acquire(void) {
        EnterCriticalSection(&m_sync);
    }


    ////////////////////////////////////////////////////////////////////////////
    // Releases a basic synchronization lock.
    //
    virtual void release(void) {
        LeaveCriticalSection(&m_sync);
    }


    ////////////////////////////////////////////////////////////////////////////
    // Constructs a WinLock.
    //
    WinLock::WinLock(void)
            : m_sync()
    {
        InitializeCriticalSection(&m_sync);
    }


    ////////////////////////////////////////////////////////////////////////////
    // Destroys a WinLock.
    //
    WinLock::~WinLock() {
        DeleteCriticalSection(&m_sync);
    }

private:
    CRITICAL_SECTION m_sync;
};

#endif    // _WIN_LOCK_H_
