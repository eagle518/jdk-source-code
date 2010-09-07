/*
 * @(#)ISupports.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// ISupports.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Contains interface of basic IUnknown thing
//
#ifndef _ISUPPORTS_H_
#define _ISUPPORTS_H_

#include "JDSupportUtils.h"
#include "JDCOMUtils.h"

//{4EC64951-92D7-11d6-B77F-00B0D0A18D51}
#define ISUPPORTS_IID \
    {0x4EC64951, 0x92D7, 0x11d6, {0xB7, 0x7F, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51} }

//ISupports interface (A replicate of nsISupports)
class ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(ISUPPORTS_IID);

    // void QueryInterface(in REFIID uuid, [iid_is (uuid), retval] out QIResult result); */
    JD_IMETHOD QueryInterface(JDREFNSIID uuid, void* *result) = 0;
    // Addref
    JD_IMETHOD_(JDREFCNT) AddRef(void) = 0;
    // Release
    JD_IMETHOD_(JDREFCNT) Release(void) = 0;
};

#endif //_ISUPPORTS_H_
