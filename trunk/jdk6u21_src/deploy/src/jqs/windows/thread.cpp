/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This module provides interface to underlying OS functions for
 * thread management.
 */

#include "os_utils.hpp"

#include <process.h>

#include "thread.hpp"
#include "print.hpp"


static unsigned int __stdcall _thread_body (void *param) {
    TRY {
        Thread * t = (Thread *) param;
        t -> run ();
        _endthreadex(0);

    } CATCH_SYSTEM_EXCEPTIONS {
        jqs_exit(1);
    }
    return 0;
}


/**
 * Create a new thread object.
 */
Thread :: Thread ()
    : handle(NULL)
{}


Thread :: ~Thread () {
    if (handle != NULL) {
        CloseHandle (handle);
        handle = NULL;
    }
}

/**
 * Start thread.
 */
bool Thread::start () {
    unsigned int threadID;

    handle = (HANDLE)_beginthreadex(NULL,
                                    0,
                                    _thread_body,
                                    this,
                                    0,
                                    &threadID);

    if (!handle) {
        jqs_warn ("Unable to start thread: %s\n", strerror(errno));
        return false;
    }

    return true;
}

/**
 * Wait for thread to finish
 */
void Thread::waitToFinish () {
    DWORD res = WaitForSingleObject (handle, INFINITE);
    if (res == WAIT_FAILED) {
        jqs_warn ("Waiting for thread to finish failed. Error %d\n", GetLastError());
    }
}

