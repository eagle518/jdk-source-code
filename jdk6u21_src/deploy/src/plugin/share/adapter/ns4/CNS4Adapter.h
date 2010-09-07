/*
 * @(#)CNS4Adapter.h	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4Adapter.h    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4Adapter.h : Declaration of the CNS4Adapter
//
// This acts as a adapter layer to allow OJI plugins work with the 4.0/3.0 
// browser.
//

#ifndef __CNS4ADAPTER_H_
#define __CNS4ADAPTER_H_


#include "INS4Adapter.h"
#include "INS4AdapterInit.h"

class INS4AdapterPeer;

class IPluginManager;
class IPlugin;
/////////////////////////////////////////////////////////////////////////////
// CNS4Adapter
class ATL_NO_VTABLE CNS4Adapter : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNS4Adapter, &CLSID_NS4Adapter>,
	public INS4Adapter,
	public INS4AdapterInit
{
public:
    CNS4Adapter()
    {
	m_pINS4AdapterPeer = NULL;
	m_pPluginManager = NULL;
	m_pPlugin = NULL;
    }

    void FinalRelease();

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNS4Adapter)
    COM_INTERFACE_ENTRY_IID(IID_INS4Adapter, INS4Adapter)
    COM_INTERFACE_ENTRY_IID(IID_INS4AdapterInit, INS4AdapterInit)
END_COM_MAP()

// INS4Adapter
public:

    // INS4Adapter
    STDMETHOD_(NPError, NPP_Initialize) (void);
    STDMETHOD_(void, NPP_Shutdown) (void);
    STDMETHOD_(NPError, NPP_New) (NPMIMEType, NPP, uint16, 
				  int16, char**, char**, NPSavedData*);
    STDMETHOD_(NPError, NPP_Destroy) (NPP, NPSavedData**);
    STDMETHOD_(NPError, NPP_SetWindow) (NPP, NPWindow*);
    STDMETHOD_(NPError, NPP_NewStream) (NPP, NPMIMEType, 
					NPStream*, NPBool, uint16*);
    STDMETHOD_(NPError, NPP_DestroyStream) (NPP,  NPStream*,
					    NPReason);
    STDMETHOD_(int32, NPP_WriteReady) (NPP, NPStream*);
    STDMETHOD_(int32, NPP_Write) (NPP, NPStream*, int32, int32, void*);
    STDMETHOD_(void, NPP_StreamAsFile) (NPP, NPStream*, const char*);
    STDMETHOD_(void, NPP_Print) (NPP, NPPrint*);
    STDMETHOD_(int16, NPP_HandleEvent) (NPP, void*);
    STDMETHOD_(void, NPP_URLNotify) (NPP, const char*, 
				     NPReason, void*);
    STDMETHOD_(jref, NPP_GetJavaClass) (void);

    // INS4AdapterInit
    STDMETHOD_(NPError, Initialize) (NPNetscapeFuncs* pNavigatorFuncs);

private:
    INS4AdapterPeer*  m_pINS4AdapterPeer; // Interface to Netscape 4.x adapter peer
    
    IPluginManager*   m_pPluginManager;	  // PluginManager for the browser side
    IPlugin*	      m_pPlugin;	  // Plugin for the OJI side
};


#endif //__CNS4ADAPTER_H_
