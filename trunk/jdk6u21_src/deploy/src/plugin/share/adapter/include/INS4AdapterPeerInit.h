/*
 * @(#)INS4AdapterPeerInit.h	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// INS4AdapterPeerInit.h  by Stanley Man-Kit Ho
//
//=---------------------------------------------------------------------------=
//
// object definition for Netscape 4.x adapter peer initialization implemetation in 
// Java Plug-in.
//

#ifndef _INS4ADAPTERPEERINIT_H
#define _INS4ADAPTERPEERINIT_H

#include "ISupports.h"
// {DFEF9360-B8A5-11d2-BA19-00105A1F1DAB}
static const JDIID IID_INS4AdapterPeerInit = 
{ 0xdfef9360, 0xb8a5, 0x11d2, { 0xba, 0x19, 0x0, 0x10, 0x5a, 0x1f, 0x1d, 0xab } };

// INS4AdapterPeerInit interface
class INS4AdapterPeerInit : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IID_INS4AdapterPeerInit);
    // INS4AdapterPeerInit
    JD_IMETHOD Initialize(NPNetscapeFuncs*) = 0;
};

#endif // _INS4ADAPTERPEERINIT_H
