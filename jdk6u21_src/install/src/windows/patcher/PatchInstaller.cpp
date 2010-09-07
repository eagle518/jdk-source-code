/*
 * @(#)PatchInstaller.cpp	1.47 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// PatchInstaller.cpp : Implementation of WinMain
//
// By Stanley Man-Kit Ho
//

#include "stdafx.h"
#include <atlhost.h>
#include "resource.h"
#include <initguid.h>
#include "pbindwin.h"
#include "PatchUtils.h"


#define UPDATE_ERROR_OPTIONS 2
#define UPDATE_ERROR_PATCH 3
#define UPDATE_ERROR_VERSIONINFO 5
#define UPDATE_ERROR_INVALID_BASE_IMAGE 6
#define UPDATE_ERROR_SAME_VERSION_INSTALLED  20
#define UPDATE_ERROR_NEWER_VERSION_INSTALLED 21
#define UPDATE_ERROR_WRONG_VERSION_UNINSTALLED 13


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


/////////////////////////////////////////////////////////////////////////////
//
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
{
#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init(ObjectMap, hInstance, NULL);

    int nRet = 0;

    // Various mode of operation
    BOOL bSilentMode = FALSE;

    TCHAR szTargetNewVersionInfo[256];

    // Open patch trace file
    OPEN_PATCH_TRACEFILE();

    do
    {
	TCHAR szDirectory[BUFFER_SIZE];
	szDirectory[0] = NULL;

	//---------------------------------------------------
	// Parse parameters from command line
	//
	if ((__argc == 2) ||
	    (__argc == 3 && lstrcmpi(__argv[1], TEXT("-s")) == 0))
	{
	    // jupdate [-s] dir (Apply patch)
	    //
	    wsprintf(szDirectory, "%s", __argv[__argc - 1]);

	    //---------------------------------------------------
	    // Remove trailing '\' in path (#4689837)
	    //
	    int iPathLen = lstrlen(szDirectory);

	    if (szDirectory[iPathLen - 1] == '\\' || szDirectory[iPathLen - 1] == '\"')
		szDirectory[iPathLen - 1] = NULL;

	    if (__argc == 3)
		bSilentMode = TRUE;
	}
	else
	{
	    // Invalid options
	    nRet = UPDATE_ERROR_OPTIONS;
	    break;
	}


	if (bSilentMode)
	{
	    // Patch silently
	    if (ApplyPatch(szDirectory, "", SilentUpdateCallBack) == FALSE)
	    {
		nRet = UPDATE_ERROR_PATCH;
		break;
	    }
	}
    }
    while (0);


    // Show dialogs for success/failure
    //
    TCHAR szBuffer[BUFFER_SIZE], szMessage[BUFFER_SIZE], szCaption[BUFFER_SIZE];

    switch (nRet)
    {
	case 0:
	{	
	    ::LoadString(_Module.GetResourceInstance(), IDS_INSTALL_SUCCESS, szBuffer, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_SUCCEEDED, szCaption, BUFFER_SIZE);

	    wsprintf(szMessage, szBuffer, NEW_IMAGE_FULLVERSION);

	    DisplayInfo(bSilentMode, szMessage, szCaption);

	    break;    
	}
	case UPDATE_ERROR_OPTIONS:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_OPTIONS, szBuffer, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    char* lpszPath = strrchr(__argv[0], '\\');

	    if (lpszPath != NULL)
		wsprintf(szMessage, szBuffer, lpszPath + 1);
	    else
		wsprintf(szMessage, szBuffer, __argv[0]);

	    DisplayError(bSilentMode, szMessage, szCaption);

	    break;
	}

	case UPDATE_ERROR_PATCH:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_UPDATE, szMessage, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    DisplayError(bSilentMode, szMessage, szCaption);

	    break;
	}	
	case UPDATE_ERROR_VERSIONINFO:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_VERSIONINFO, szMessage, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    DisplayError(bSilentMode, szMessage, szCaption);

	    break;
	}
	case UPDATE_ERROR_INVALID_BASE_IMAGE:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_BASE_IMAGE_NOT_FOUND, szBuffer, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    wsprintf(szMessage, szBuffer, NEW_IMAGE_FULLVERSION, BASE_IMAGE_FULLVERSION);

	    DisplayError(bSilentMode, szMessage, szCaption);

	    break;
	}
	case UPDATE_ERROR_SAME_VERSION_INSTALLED:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_INSTALLED_SAME_VERSION, szBuffer, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_SUCCEEDED, szCaption, BUFFER_SIZE);

	    wsprintf(szMessage, szBuffer, NEW_IMAGE_FULLVERSION);

	    DisplayInfo(bSilentMode, szMessage, szCaption);

	    break;
	}
	case UPDATE_ERROR_NEWER_VERSION_INSTALLED:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_INSTALLED_NEWER_VERSION, szBuffer, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    wsprintf(szMessage, szBuffer, NEW_IMAGE_FULLVERSION, szTargetNewVersionInfo);

	    DisplayError(bSilentMode, szMessage, szCaption);

	    break;
	}
	case UPDATE_ERROR_WRONG_VERSION_UNINSTALLED:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_UNINSTALL_WRONG_VERSION, szBuffer, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    wsprintf(szMessage, szBuffer, NEW_IMAGE_FULLVERSION, szTargetNewVersionInfo);

	    DisplayError(bSilentMode, szMessage, szCaption);

	    break;
	}
    };


    // Close patch trace file
    CLOSE_PATCH_TRACEFILE();


    _Module.Term();
    CoUninitialize();

    if (nRet == 0)
	return 0;
    else
    	return ERROR_INSTALL_FAILURE;
}


