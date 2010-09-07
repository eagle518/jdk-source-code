/*
 * @(#)IObserverService.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IObserverService.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
//
#ifndef _IOBSERVERSERVICE_H_
#define _IOBSERVERSERVICE_H_

#include "ISupports.h"
#include "IObserver.h"

/*{70E54A36-A302-428b-A928-C4B0D756D585}*/
#define COBSERVERSERVICE_CID \
    {0x70E54A36, 0xA302, 0x428b, \
    {0xA9, 0x28, 0xC4, 0xB0, 0xD7, 0x56, 0xD5, 0x85 }}

/*{7542FB1A-F9E9-4435-8BBB-7FAD59229122}*/
#define IOBSERVERSERVICE_IID \
    {0x7542FB1A, 0xF9E9, 0x4435, \
    { 0x8b, 0xbb, 0x7f, 0xad, 0x59, 0x22, 0x91, 0x22 }}


class IObserverService : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IOBSERVERSERVICE_IID);

    JD_IMETHOD AddObserver(IObserver *anObserver, const JDUnichar *aTopic) = 0;

    JD_IMETHOD RemoveObserver(IObserver *anObserver, const JDUnichar *aTopic) = 0;

    JD_IMETHOD Notify(ISupports* aSubject, const JDUnichar *aTopic) = 0;
};

#endif //_IOBSERVERSERVICE_H_
