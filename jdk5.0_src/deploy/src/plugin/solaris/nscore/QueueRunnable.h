/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)QueueRunnable.h	1.8 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
