/*
 * @(#)CNS6Adapter.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS6Adapter.h : Declaration of the CNS6Adapter that is the OJI Plug-in
//		   COM object.
//

#ifndef __CNS6ADAPTER_H_
#define __CNS6ADAPTER_H_


#include "oji_clsid.h"
#include "IPluginModule.h"

/////////////////////////////////////////////////////////////////////////////
// CNS6Adapter
class ATL_NO_VTABLE CNS6Adapter : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNS6Adapter, &CLSID_NS6Adapter>,
	public IPluginModule
{
public:
    CNS6Adapter() {}

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNS6Adapter)
	COM_INTERFACE_ENTRY_IID(IID_IPluginModule, IPluginModule)
END_COM_MAP()

public:

    // IPluginModule
    STDMETHOD_(nsresult, NSGetFactory) (const nsCID &aClass,
					nsISupports *aSupport,
                                        nsIFactory **aFactory);

    STDMETHOD_(PRBool, NSCanUnload) (void);
    STDMETHOD_(nsresult, NSRegisterSelf) (const char* path);
    STDMETHOD_(nsresult, NSUnregisterSelf) (const char* path);

};

#endif //__CNS6ADAPTER_H_

