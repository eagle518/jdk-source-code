/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-*/
/*
 * @(#)QueueRunnable.cpp	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Contains definition of the Runnable class needed to process
 * the worker and spontainious pipes
 */

#include "commonhdr.h"
#include <poll.h>
#include <errno.h>
#include "QueueRunnable.h"
#include "IUnixService.h"

extern IUnixService* g_unixService;

JD_IMPL_ISUPPORTS1(QueueRunnable, IRunnable)

QueueRunnable::QueueRunnable(void * vm, pfnQueueProcessor f, int fd, JDUint32 tid, void * m, bool * pc, IThreadManager * tm )
{
    JD_INIT_ISUPPORTS();

    mJVM = vm;
    mFunction = f;
    mFD = fd;
    mThreadID = tid;
    mMonitor = m;
    mIsPipeClean = pc;
    mThreadManager = tm;
    mThreadManager->AddRef();
}

QueueRunnable::~QueueRunnable()
{
    if(mThreadManager) {
        mThreadManager->Release();
    }
}

JD_IMETHODIMP QueueRunnable::Run()
{
    if (mFunction) {
        mFunction((void *) mJVM);
    }
    return JD_OK;
}

void QueueRunnable::waitOnPipe()
{
    int rv;
    struct pollfd fds[1];
    fds[0].fd = mFD;
    fds[0].events = POLLRDNORM;
 
    for(;;) {
        fds[0].revents = 0;
        rv = poll(fds,1,-1);
        if(rv == -1) {
           // Deal with error case here
           if(errno != EINTR) {
               break;
           }
        } else {
           // Make sure we are out of poll for the right reason and
           // Put us one the Event Que of the main thread
            if(fds[0].revents & POLLRDNORM) {
                if(mThreadManager) {
                    trace("QueueRunnable: Posting Event: pipe %d activity\n",mFD);
                    g_unixService->JD_EnterMonitor(mMonitor);
                    *mIsPipeClean = false;
                    mThreadManager->PostEvent(mThreadID, this, JD_TRUE);
                    while(!*mIsPipeClean) g_unixService->JD_Wait(mMonitor,
                                                                 JD_INTERVAL_NO_TIMEOUT);
                    g_unixService->JD_ExitMonitor(mMonitor);
                }
            }
        }
    }   
}

void QueueRunnable::threadEntry(void *arg) {

    QueueRunnable * qr = (QueueRunnable *) arg;

    qr->AddRef();
    qr->waitOnPipe();
    qr->Release();
}
