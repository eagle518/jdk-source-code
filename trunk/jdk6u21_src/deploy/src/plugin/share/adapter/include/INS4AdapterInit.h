/*
 * @(#)INS4AdapterInit.h	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// INS4AdapterInit.h  by Stanley Man-Kit Ho
//
//=---------------------------------------------------------------------------=
//
// object definition for Netscape 4.x adapter initialization implemetation in 
// Java Plug-in.
//

#ifndef __INS4ADAPTERINIT_H_
#define __INS4ADAPTERINIT_H_


// INS4AdapterInit interface
DECLARE_INTERFACE_(INS4AdapterInit, IUnknown)
{
    // IUnknown memebers
    STDMETHOD(QueryInterface) (THIS_ REFIID, void**) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // INS4AdapterInit
    STDMETHOD_(NPError, Initialize) (THIS_ NPNetscapeFuncs*) PURE;
};

// {BA9E64D0-B89C-11d2-BA19-00105A1F1DAB}
static const IID IID_INS4AdapterInit = 
{ 0xba9e64d0, 0xb89c, 0x11d2, { 0xba, 0x19, 0x0, 0x10, 0x5a, 0x1f, 0x1d, 0xab } };


#endif // __INS4ADAPTERINIT_H_
