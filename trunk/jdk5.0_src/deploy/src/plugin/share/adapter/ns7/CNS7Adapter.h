/*
 * @(#)CNS7Adapter.h	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter.h : Declaration of the CNS7Adapter that is the OJI Plug-in
//		   COM object.
//

#ifndef __CNS7Adapter_H_
#define __CNS7Adapter_H_

#include "resource.h"       // main symbols

struct IPluginModule;

////////////////////////////////////////////////////////////////////////////////
// CNS7Adapter
class ATL_NO_VTABLE CNS7Adapter : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CNS7Adapter, &CLSID_NS7Adapter>,
    public IPluginModule
{
public:
    CNS7Adapter() {}

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNS7Adapter)
	COM_INTERFACE_ENTRY_IID(IID_IPluginModule, IPluginModule)
END_COM_MAP()

public:
    // IPluginModule
    // User must implement this method 
    STDMETHOD_(nsresult, NSGetFactory) (const nsCID &aClass,
					nsISupports *aSupport,
                                        nsIFactory **aFactory);

    STDMETHOD_(PRBool, NSCanUnload) (void);
    STDMETHOD_(nsresult, NSRegisterSelf) (const char* path);
    STDMETHOD_(nsresult, NSUnregisterSelf) (const char* path);

};

#endif //__CNS7Adapter_H_
