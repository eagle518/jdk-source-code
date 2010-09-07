/*
 *  @(#)common.cpp	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <atlbase.h>
#include "common.h"

/* Returns TRUE if current running platform is Windows NT, FALSE otherwise */
BOOL IsPlatformWindowsNT()
{
    OSVERSIONINFO  osvi;

    // Initialize the OSVERSIONINFO structure.
    ZeroMemory( &osvi, sizeof( osvi ) );
    osvi.dwOSVersionInfoSize = sizeof( osvi );

    GetVersionEx( &osvi );  // Assume this function succeeds.

    // Split code paths for NT and Win9x
    if( osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
    {
        // Windows 95, 98, ME
        return FALSE;
    }
    else if( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        // Windows NT, 2000, XP
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* Returns TRUE if current running platform is Windows XP or later, FALSE otherwise */
BOOL IsPlatformWindowsXPorLater()
{
    static BOOL initialized = FALSE;
    static BOOL isXP = FALSE;
    OSVERSIONINFO  osvi;

    if (initialized) {
        return isXP;
    }


    // Initialize the OSVERSIONINFO structure.
    ZeroMemory( &osvi, sizeof( osvi ) );
    osvi.dwOSVersionInfoSize = sizeof( osvi );

    GetVersionEx( &osvi );  // Assume this function succeeds.

    if( osvi.dwPlatformId > VER_PLATFORM_WIN32_NT )
    {
        // maybe a windows BSD ? (ROTFL)
        isXP = TRUE;
    }
    else if( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
    {
        if( osvi.dwMajorVersion > 5 ) {
            // >= Vista
            isXP = TRUE;
        } else if( osvi.dwMajorVersion == 5 ) {
            if( osvi.dwMinorVersion >= 1 ) {
                // >= XP
                isXP = TRUE;
            } // else Win2000
        }
    }
    initialized = TRUE;

    return isXP;
}

/* Returns TRUE if current running platform is Windows Vista, FALSE otherwise */
BOOL IsPlatformWindowsVista() {
    static BOOL initialized = FALSE;
    static BOOL isVista = FALSE;
    OSVERSIONINFO  osvi;

    if (initialized) {
        return isVista;
    }

    // Initialize the OSVERSIONINFO structure.
    ZeroMemory( &osvi, sizeof( osvi ) );
    osvi.dwOSVersionInfoSize = sizeof( osvi );

    GetVersionEx( &osvi );  // Assume this function succeeds.

    if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && 
        osvi.dwMajorVersion == 6 ) {
	isVista = TRUE;
    } else {
        isVista = FALSE;
    }

    initialized = TRUE;

    return isVista;
}

BOOL GetAppDataLocalLowPath(char * lpszUserInfo) {
    HMODULE hModule = LoadLibrary("shell32.dll");
    LPWSTR path;
    BOOL   ret = FALSE;

    WCHAR  pathBuf[MAX_PATH + 1];

    if (hModule == NULL) {
        return ret;
    }
    // SHGetFolderPathEx
    LPFNSHGetFolderPathEx lpfnSHGetFolderPathEx = (LPFNSHGetFolderPathEx)
        GetProcAddress(hModule, "SHGetFolderPathEx");
	
    if (lpfnSHGetFolderPathEx != NULL) {
        path = pathBuf;
        HRESULT result = lpfnSHGetFolderPathEx(FOLDERID_LocalAppDataLow, 0, 
						 NULL, path, MAX_PATH);
        if (result == S_OK) {          
            WideCharToMultiByte(CP_ACP, 0, path, -1, lpszUserInfo, MAX_PATH, 
                NULL, NULL);
            ret = TRUE;
        }
    }
  
    FreeLibrary(hModule);
    return ret;
}


BOOL GetLoadedModuleDirectory(LPCTSTR moduleName,
                              BOOL shortName,
                              LPTSTR  directoryContainingModule,
                              DWORD   directoryContainingModuleLength)
{
    // Attempts to find the given module in the address space of this
    // process and return the directory containing the module. A NULL
    // moduleName means to look up the directory containing the
    // application rather than any given loaded module. There is no
    // trailing backslash ('\') on the returned path. If the named
    // module was found in the address space of this process, returns
    // TRUE, otherwise returns FALSE. directoryContainingModule may be
    // mutated regardless.
    HMODULE module = NULL;
    TCHAR fileDrive[MAX_BUFFER] = {0};
    TCHAR fileDir[MAX_BUFFER] = {0};
    TCHAR fileName[MAX_BUFFER] = {0};
    TCHAR fileExt[MAX_BUFFER] = {0};
    DWORD numChars;
    int dirLen;

    if (moduleName != NULL) {
        module = GetModuleHandle(moduleName);
        if (module == NULL)
            return FALSE;
    }

    numChars = GetModuleFileName(module,
                                 directoryContainingModule,
                                 directoryContainingModuleLength);
    if (numChars == 0)
        return FALSE;
    
    if (shortName) {
        // We need this path to be in short form; we can't support spaces in the boot class path
        if (GetShortPathName(directoryContainingModule,
                             directoryContainingModule,
                             directoryContainingModuleLength) == 0) {
            return FALSE;
        }
    } else {
        // We prefer to use long path names for the registry
        if (GetLongPathName(directoryContainingModule,
                            directoryContainingModule,
                            directoryContainingModuleLength) == 0) {
            return FALSE;
        }
    }
    _tsplitpath(directoryContainingModule, fileDrive, fileDir, fileName, fileExt);
    // The fileDir has a trailing slash which we don't want
    dirLen = _tcslen(fileDir);
    if (dirLen > 1 && (fileDir[dirLen - 1] == _T('\\') || fileDir[dirLen - 1] == _T('/'))) {
        fileDir[dirLen - 1] = _T('\0');
    }
    _tcscpy(directoryContainingModule, fileDrive);
    _tcscat(directoryContainingModule, fileDir);
    return TRUE;
}

// check if a file is being locked
BOOL IsFileLocked(LPCTSTR lpszFile) {
    BOOL bRet = FALSE;

    HANDLE hFile = CreateFile(lpszFile, GENERIC_WRITE, 0, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_SHARING_VIOLATION) bRet = TRUE;
    } else CloseHandle(hFile);

    return bRet;
}

// check registry if new Java Plug-in is currently selected
BOOL IsUsingNewJavaPlugin(LPCTSTR lpszVersion)
{
    CRegKey swKey, jsKey, pluginKey, pvKey;
    DWORD dwIndex = 0;
    char szKeyName[MAX_BUFFER];
    BOOL bResult = FALSE;

    if (swKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE", KEY_READ) != ERROR_SUCCESS)
        return FALSE;
    if (jsKey.Open(swKey, "JavaSoft", KEY_READ) != ERROR_SUCCESS)
        return FALSE;
    if (pluginKey.Open(jsKey, "Java Plug-in", KEY_READ) != ERROR_SUCCESS)
        return FALSE;
    ::ZeroMemory(szKeyName, MAX_BUFFER);

    while (::RegEnumKey(HKEY(pluginKey), dwIndex++, szKeyName, MAX_BUFFER) == ERROR_SUCCESS){
        // check if we're inspecting the current version of Java Plug-in registry
        // MIMEVERSION is defined in PluginVersion.h
        if (!strcmp(szKeyName, lpszVersion)) {
            if (pvKey.Open(pluginKey, szKeyName, KEY_READ) != ERROR_SUCCESS){
                break;
            }

            DWORD dwSetting = 0;
            if( pvKey.QueryValue(dwSetting, "UseNewJavaPlugin") == ERROR_SUCCESS
){
                bResult = (dwSetting == 1) ? TRUE : FALSE;
            }
            break;
        }
        ::ZeroMemory(szKeyName, MAX_BUFFER);
    }

    return bResult;
}

BOOL GetPluginJavaHomePath(LPCTSTR version,
                           BOOL    shortName,
                           LPTSTR  javaHomePath,
                           DWORD   javaHomePathLen) {
    CRegKey swKey, jsKey, pluginKey, pvKey;
    DWORD dwIndex = 0;
    BOOL bResult = FALSE;
    DWORD tmpLen = javaHomePathLen;

    if (swKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE", KEY_READ) != ERROR_SUCCESS)
        return FALSE;
    if (jsKey.Open(swKey, "JavaSoft", KEY_READ) != ERROR_SUCCESS)
        return FALSE;
    if (pluginKey.Open(jsKey, "Java Plug-in", KEY_READ) != ERROR_SUCCESS)
        return FALSE;
    if (pvKey.Open(pluginKey, version, KEY_READ) != ERROR_SUCCESS)
        return FALSE;
    if (pvKey.QueryValue(javaHomePath, _T("JavaHome"), &tmpLen) != ERROR_SUCCESS)
        return FALSE;
    if (shortName) {
        // We need this path to be in short form; we can't support spaces in the boot class path
        if (GetShortPathName(javaHomePath,
                             javaHomePath,
                             javaHomePathLen) == 0) {
            return FALSE;
        }
    } else {
        // We prefer to use long path names for the registry
        if (GetLongPathName(javaHomePath,
                            javaHomePath,
                            javaHomePathLen) == 0) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL GetTreatAsValue(char* szTreatAs, DWORD size)
{
    USES_CONVERSION;

    CRegKey regKey, clsidKey, treatAsKey;

    // HKEY_CLASSES_ROOT\\CLSID\\{08B0E5C0-4FCB-11CF-AAA5-00401C608501}\\TreatAs
    if (regKey.Open(HKEY_CLASSES_ROOT, "CLSID", KEY_READ) != ERROR_SUCCESS)
        return FALSE;

    LPOLESTR lpszOleCLSID = NULL;

    if (FAILED(StringFromCLSID(MSJAVA_APPLET_CLSID_TYPE, &lpszOleCLSID)))
        return FALSE;

    const char* lpszMSJavaAppletCLSID = OLE2A(lpszOleCLSID);

    ::CoTaskMemFree(lpszOleCLSID);
    lpszOleCLSID = NULL;

    if (clsidKey.Open(regKey, lpszMSJavaAppletCLSID, KEY_READ) != ERROR_SUCCESS) {
        return FALSE;
    }

    if (treatAsKey.Open(clsidKey, "TreatAs", KEY_READ) != ERROR_SUCCESS){
        return FALSE;
    }  

    // Query value
    if (treatAsKey.QueryValue(szTreatAs, NULL, &size) != ERROR_SUCCESS) {
        return FALSE;
    }

    return TRUE;
}
