/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)QueueRunnable.h	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Contains declaration of the Runnable class needed to process
 * the workier and spontainious pipes
 */


#ifndef QUEUERUNNABLE_H
#define QUEUERUNNABLE_H

#include "commonhdr.h"
#include "IThreadManager.h"

typedef void (* pfnQueueProcessor)(void *);

class QueueRunnable : public IRunnable {
public:
    JD_DECL_ISUPPORTS

    JD_IMETHOD Run();
	
    QueueRunnable(void * vm, pfnQueueProcessor f, int fd, JDUint32 tid, void * m, bool * pc, IThreadManager * tm);
    virtual	~QueueRunnable(void);

    void waitOnPipe(void);
    static void threadEntry(void * arg);

private:
    void * mMonitor;
    bool * mIsPipeClean;
    void * mJVM;
    pfnQueueProcessor mFunction;
    int mFD;
    JDUint32 mThreadID;
    IThreadManager* mThreadManager;
};

#endif 
