/*
 * @(#)JavaWebStart.cpp	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// JavaWebStart.cpp : Implementation of DLL Exports.


// Note: Proxy/Stub Information
//      To build a separate proxy/stub DLL, 
//      run nmake -f JavaWebStartps.mk in the project directory.

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "wsdetect.h"

#include "wsdetect_i.c"
#include "isInstalled.h"


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_isInstalled, CisInstalled)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, &LIBID_JAVAWEBSTARTLib);
        DisableThreadLibraryCalls(hInstance);
    }
	else if (dwReason == DLL_PROCESS_DETACH) {
        _Module.Term();
		// Clean up Winsocks library
		if (do_WSACleanup) {
			do_WSACleanup();
		}
	}
    return TRUE;    // ok
}

/////////////////////////////////////////////////////////////////////////////
// Used to determine whether the DLL can be unloaded by OLE

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// Returns a class factory to create an object of the requested type

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

/////////////////////////////////////////////////////////////////////////////
// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _Module.RegisterServer(TRUE);

    //To fix #4847320
    TCHAR szkey[_MAX_PATH];
    szkey[0] = '\0';
    LPOLESTR oleCLSID;
    USES_CONVERSION;
    StringFromCLSID(CLSID_isInstalled, &oleCLSID);
    wsprintf(szkey, "CLSID\\%s\\InprocServer32", OLE2A(oleCLSID));
    CoTaskMemFree(oleCLSID);

    CRegKey clsidKey;
    if (clsidKey.Open(HKEY_CLASSES_ROOT, szkey, KEY_ALL_ACCESS) != ERROR_SUCCESS)
	return SELFREG_E_CLASS;

    TCHAR buffer[_MAX_PATH];
    GetModuleFileName(_Module.m_hInst, buffer, _MAX_PATH);
    clsidKey.SetValue(buffer);

    return hr;
}

/////////////////////////////////////////////////////////////////////////////
// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


