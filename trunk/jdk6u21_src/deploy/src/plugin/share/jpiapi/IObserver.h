/*
 * @(#)IObserve.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IObserve.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
//
#ifndef _IOBSERVER_H_
#define _IOBSERVER_H_

#include "ISupports.h"

/*{8F7A4258-72DC-49e7-A5D4-AB34E7908F9D}*/
#define IOBSERVER_IID \
    {0x8F7A4258, 0x72DC, 0x49e7, \
    { 0xa5, 0xd4, 0xab, 0x34, 0xe7, 0x90, 0x8f, 0x9d}}


class IObserver : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IOBSERVER_IID);

    JD_IMETHOD	Observe(ISupports* aSubject, const JDUnichar *aTopic, const JDUnichar *aData) = 0;
};

#endif //_IOBSERVER_H_
