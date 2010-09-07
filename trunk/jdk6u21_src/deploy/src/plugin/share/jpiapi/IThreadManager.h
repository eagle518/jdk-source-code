/*
 * @(#)IThreadManager.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//  IThreadManager.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Contains interface for threading service from browser called by Plug-in
//
#ifndef _ITHREADMANAGER_H_
#define _ITHREADMANAGER_H_

#include "ISupports.h"
class IRunnable;
//{EFD74BDF-99B7-11d6-9A76-00B0D0A18D51}
#define ITHREADMANAGER_IID \
    {0xEFD74BDF, 0x99B7, 0x11d6, {0x9A, 0x76, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

//ISupports interface (A replicate of nsISupports)
class IThreadManager : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(ITHREADMANAGER_IID);

    JD_IMETHOD
    GetCurrentThread(JDUint32 *threadID) = 0;

    JD_IMETHOD
    PostEvent(JDUint32 threadID, IRunnable* runnable, JDBool async) = 0;
};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// IRunnable
// This interface represents the invocation of a new thread.

#define IRUNNABLE_IID								\
{ /* {EFD74BE0-99B7-11d6-9A76-00B0D0A18D51} */					\
    0xEFD74BE0,									\
    0x99B7,									\
    0x11d6,									\
    {0x9A, 0x76, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}				\
}

class IRunnable : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IRUNNABLE_IID)

    /**
     * Defines an entry point for a newly created thread.
     */
    JD_IMETHOD
    Run() = 0;
};

#endif /* IThreadManager_h___ */
