/*
 * @(#)NativeLocker.h	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
#ifndef _NATIVE_LOCKER_H_
#define _NATIVE_LOCKER_H_

#include "PerfLib.h"
#include "NativeLock.h"


class PERFLIB_API NativeLocker {

public:
    ////////////////////////////////////////////////////////////////////////////
    // This constructor will automatically acquire the given lock, protecting
    // against unexpected exceptions, etc. that could cause a deadlock.
    //
    NativeLocker(NativeLock * pSync)
        : m_pSync(pSync) {
        if (m_pSync != NULL) {
            m_pSync->acquire();
        }
    }


    ////////////////////////////////////////////////////////////////////////////
    // This destructor will automatically release its lock, protecting against
    // unexpected exceptions, etc. that could cause a deadlock.
    //
    ~NativeLocker() {
        if (m_pSync != NULL) {
            m_pSync->release();
            m_pSync = NULL;
        }
    }

private:
    NativeLock * m_pSync;
};

#endif    // _NATIVE_LOCKER_H_
