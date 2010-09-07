/*
 * @(#)WrapperUtils.h	1.15 10/04/03
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <shellapi.h>
#include "UpdateUtils.h"

// If for some reason AUVERSION is not defined preprocessor (e.g. 32/64bit combined wrapper)
// Set the bundled version to 0, so that the wrappers will never uninstall existing AU
#if defined(AUVERSION)
#define BUNDLED_AUVERSION AUVERSION
#else
#define BUNDLED_AUVERSION "0"
#endif

void RunStuffPostInstall();
BOOL IsThisJDK();
LANGID DetectLocale();
BOOL IsThisValidLang(LANGID lang);
void GetShortTempPath(DWORD dwLen, LPTSTR szTempPath);
void ErrorExit(LPTSTR lpszFunction);
BOOL ExtractFileFromResource(HINSTANCE hInst, LPCTSTR lpName, LPCTSTR lpType, LPCTSTR lpszFile);
int ExecCommand(LPSTR lpszCommand);
BOOL IsWIVerValid(LPDWORD lpdwFileVerMS);
BOOL GetWIPath(LPTSTR lpszWIPath, DWORD dwSize);
void GetSingleMSIFileNames(LPTSTR lpszLocalFileName, BOOL b64bit);
void GetJInstallUserAgent(LPTSTR  lpszUserAgent, DWORD dwCount);
BOOL IsThisMsi(LPCTSTR lpPath);
HRESULT IsReachable(LPCTSTR lpszIsConnectedURL);
BOOL GetShellFolder(int nFolder, LPTSTR szPath);
LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2);
BOOL IsFileExists(LPCSTR lpszFileName);
void addSilentOptions(LPCTSTR lpszExec, LPSTR lpszOptions);
BOOL CheckAvailDiskSpace(LPCTSTR lpszPath, DWORD dwReqDiskSpace);
DWORD ExecProcess(LPCTSTR lpszExecutableName, LPTSTR lpszCommandLine);
void DownloadFailure();
BOOL IsOSWin2003();
BOOL IsSystem64bit();
BOOL ExtractSponsorDLL(HINSTANCE hInstance);
DWORD RemoveAnyOlderAU2X();
BOOL OlderAUExists();
DWORD UninstallAU2X();
BOOL GetAuVersion(LPCTSTR lpszAuVersion, DWORD dwBufferSize);

#define LANGID_ENGLISH 1033
#define LANGID_CHINESE_SIMPLIFIED 2052
#define LANGID_PORTUGUESE_BRAZILIAN 1046
#define JAPANESE_INDEX 7

#define ERROR_PROXY_AUTH_REQ HRESULT_FROM_WIN32(0x210)
#define ERROR_URL_NOT_FOUND HRESULT_FROM_WIN32(0x211)

#define BUFFER_SIZE 1024

#ifdef EXTRA_COMP_LIB_NAME
BOOL ExtraUnCompression(HINSTANCE hInstance, LPCTSTR longDirPath, LPCTSTR longCompressedFilePath);
void ReleaseExtraCompression(HINSTANCE hInstance);
#endif
