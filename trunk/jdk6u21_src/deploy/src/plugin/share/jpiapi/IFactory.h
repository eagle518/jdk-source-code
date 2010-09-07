/*
 * @(#)IFactory.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IFactory.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for Create XPCOM style object
//
#ifndef _IFACTORY_H_
#define _IFACTORY_H_

#include "ISupports.h"

//{2FD7BD79-92E4-11d6-B77F-00B0D0A18D51}
#define IFACTORY_IID \
    {0x2FD7BD79, 0x92E4, 0x11d6, {0xB7, 0x7F, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51} }

//ISupports interface (A replicate of nsISupports)
class IFactory : public ISupports {
public:

    JD_DEFINE_STATIC_IID_ACCESSOR(IFACTORY_IID);

     /* void createInstance (in ISupports aOuter, in nsIIDRef iid, [iid_is (iid), retval] out nsQIResult result); */
    JD_IMETHOD CreateInstance(ISupports *aOuter, const JDIID & iid, void * *result) = 0;

    /* void lockFactory (in JDBool lock); */
    JD_IMETHOD LockFactory(JDBool lock) = 0;
};
#endif //_IFACTORY_H_
