/*
 * @(#)common.h	1.8 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_BUFFER
#define MAX_BUFFER  1024
#endif

#define DEFINE_KNOWN_FOLDER(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }


// {A520A1A4-1780-4FF6-BD18-167343C5AF16}
DEFINE_KNOWN_FOLDER(FOLDERID_LocalAppDataLow, 0xA520A1A4, 0x1780, 0x4FF6, 0xBD, 0x18, 0x16, 0x73, 0x43, 0xC5, 0xAF, 0x16);

typedef GUID KNOWNFOLDERID;

#if 0
typedef KNOWNFOLDERID *REFKNOWNFOLDERID;

#endif // 0
#ifdef __cplusplus
#define REFKNOWNFOLDERID const KNOWNFOLDERID &
#else // !__cplusplus
#define REFKNOWNFOLDERID const KNOWNFOLDERID * __MIDL_CONST
#endif // __cplusplus

typedef HRESULT (WINAPI *LPFNSHGetFolderPathEx)(REFKNOWNFOLDERID rfid,
    DWORD dwFlags, HANDLE hToken, LPWSTR pszPath, int cchPath);


// {08B0E5C0-4FCB-11CF-AAA5-00401C608501}
static const CLSID MSJAVA_APPLET_CLSID_TYPE =
{ 0x08B0E5C0, 0x4FCB, 0x11CF, { 0xAA, 0xA5, 0x0, 0x40, 0x1C, 0x60, 0x85, 0x01 } };


BOOL IsPlatformWindowsNT();
BOOL IsPlatformWindowsVista();
BOOL IsPlatformWindowsXPorLater();
BOOL GetAppDataLocalLowPath(char * lpszUserInfo);
BOOL GetLoadedModuleDirectory(LPCTSTR moduleName,
                              BOOL    shortName,
                              LPTSTR  directoryContainingModule,
                              DWORD   directoryContainingModuleLength);

/* Queries the registry for the JavaHome path for the given Java version.
   Returns TRUE if success, FALSE otherwise. */
BOOL GetPluginJavaHomePath(LPCTSTR version,
                           BOOL    shortName,
                           LPTSTR  javaHomePath,
                           DWORD   javaHomePathLen);

/* Checks if a file is being locked. */
BOOL IsFileLocked(LPCTSTR lpszFile);

/* Checks if the registry is set to use the new Java Plug-in.*/
BOOL IsUsingNewJavaPlugin(LPCTSTR lpszVersion);

/* Obtains the value of the Web Browser Applet Control "TreatAs" registry key. */
BOOL GetTreatAsValue(char* szTreatAs, DWORD size);

#ifdef __cplusplus
}
#endif
