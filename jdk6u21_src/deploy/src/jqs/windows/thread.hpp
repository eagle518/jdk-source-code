/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * This module provides interface to underlying OS functions for
 * thread management.
 */

#ifndef THREAD_HPP
#define THREAD_HPP

#include <windows.h>

/**
 * Abstract class representing OS thread.
 * To use, subclass Thread class and define run() method.
 */
class Thread {
    HANDLE handle;

public:
    /**
     * Thread body.
     */
    virtual void run (void) = 0;

    /**
     * Create a new thread object.
     */
    Thread ();


    virtual ~Thread ();

    /**
     * Start thread.
     */
    virtual bool start();

    /**
     * Wait for thread to finish
     */
    void waitToFinish ();

};

#endif

