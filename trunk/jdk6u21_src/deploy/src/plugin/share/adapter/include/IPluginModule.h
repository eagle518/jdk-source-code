/*
 * @(#)IPluginModule.h	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IPluginModule.h  by Stanley Man-Kit Ho
//
//=---------------------------------------------------------------------------=
//
// object definition for OJI plug-in entry implemetation in Java Plug-in.
//
#ifndef _IPLUGINMODULE_H
#define _IPLUGINMODULE_H

#include "nsIFactory.h"

// IPluginModule interface
DECLARE_INTERFACE_(IPluginModule, IUnknown)
{
    // IUnknown memebers
    STDMETHOD(QueryInterface) (THIS_ REFIID, void**) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // IPluginModule
    STDMETHOD_(nsresult, NSGetFactory) (THIS_ const nsCID &aClass,
					nsISupports *aSupport,
                                        nsIFactory **aFactory) PURE;
    STDMETHOD_(PRBool, NSCanUnload) (THIS) PURE;
    STDMETHOD_(nsresult, NSRegisterSelf) (THIS_ const char* path) PURE;
    STDMETHOD_(nsresult, NSUnregisterSelf) (THIS_ const char* path) PURE;
};

// {5614DF31-B8BE-11d2-BA19-00105A1F1DAB}
static const IID IID_IPluginModule = 
{ 0x5614df31, 0xb8be, 0x11d2, { 0xba, 0x19, 0x0, 0x10, 0x5a, 0x1f, 0x1d, 0xab } };


#endif // _IPLUGINMODULE_H
