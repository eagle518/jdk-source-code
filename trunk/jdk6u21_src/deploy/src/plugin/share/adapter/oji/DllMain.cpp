/*
 *  @(#)DllMain.cpp	1.1 02/11/06
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// DllMain.cpp  by X.Lu
//
///=--------------------------------------------------------------------------=
//
// DllMain.cpp : Implementation of DLL Exports for COM modules with OJI Plug-in
//	       COM object and Backward Adapter COM object.
//

#include "stdafx.h"

#include "oji_clsid.h"

#include "IPluginModule.h"
#include "CNS7Adapter.h"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_NS7Adapter, CNS7Adapter)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    ATLTRACE("DllMain\n");

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);

	// If this is loaded, always try to lock itself in the memory
	_Module.Lock();
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }

    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    ATLTRACE("DllCanUnloadNow\n");

    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    ATLTRACE("DllGetClassObject\n");

    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    ATLTRACE("DllRegisterServer\n");

    // registers object 
    return _Module.RegisterServer(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    ATLTRACE("DllUnregisterServer\n");

    return _Module.UnregisterServer();
}


