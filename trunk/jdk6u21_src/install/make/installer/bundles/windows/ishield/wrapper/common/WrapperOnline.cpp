/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "StdAfx.h"
#include <process.h>
#include <tchar.h>
#include "resource.h"
#include <wininet.h>
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <mbstring.h>
#include <shlobj.h>
#include "UserProfile.h"
#include "WrapperUtils.h"

BOOL GetShellFolder(int nFolder, LPTSTR szPath) {
  LPITEMIDLIST itemList;

  /* The code below is taken from:
   * src/win32/native/classes/java/lang/java_props_md.c
   * It is the same code that computes the user.home property in Java
   *
   * Given user name XXX:
   * On multi-user NT, user.home gets set to c:\winnt\profiles\XXX.
   * On multi-user Win95, user.home gets set to c:\windows\profiles\XXX.
   * On single-user Win95, user.home gets set to c:\windows.
   */

  if (SUCCEEDED(SHGetSpecialFolderLocation(NULL,
                                           nFolder,
                                           &itemList))) {
    IMalloc *pMalloc;
    SHGetPathFromIDList(itemList, szPath);
    if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
      pMalloc->Free(itemList);
      pMalloc->Release();
      return TRUE;
    }
    return TRUE;
  }
  return FALSE;
}

HRESULT IsReachable(LPCTSTR lpszIsConnectedURL)
{
    TCHAR szURL[BUFFER_SIZE];
    HINTERNET hOpen, hConnect, hRequest;
    HRESULT ret = S_OK;
    DWORD dwVal;
    DWORD dwValSize = sizeof(DWORD);

    wsprintf(szURL, "%s", lpszIsConnectedURL);
    __try {

	// Open Internet Call
	TCHAR szHostName[BUFFER_SIZE], szUrlPath[BUFFER_SIZE], szExtraInfo[BUFFER_SIZE];
	GetJInstallUserAgent(szExtraInfo, BUFFER_SIZE);
	hOpen = ::InternetOpen(szExtraInfo, INTERNET_OPEN_TYPE_PRECONFIG, 
				NULL, NULL, NULL);

	if (hOpen == NULL) {
	    __leave;
	}

	// URL components
	URL_COMPONENTS url_components; 
	::ZeroMemory(&url_components, sizeof(URL_COMPONENTS));

	url_components.dwStructSize = sizeof(URL_COMPONENTS);
	url_components.lpszHostName = szHostName;
	url_components.dwHostNameLength = BUFFER_SIZE;
	url_components.nPort = NULL;
	url_components.lpszUrlPath = szUrlPath;
	url_components.dwUrlPathLength = BUFFER_SIZE;
	url_components.lpszExtraInfo = szExtraInfo;
	url_components.dwExtraInfoLength = BUFFER_SIZE;

	// Crack the URL into pieces
	::InternetCrackUrl(szURL, lstrlen(szURL), NULL, &url_components);

	// Open Internet Connection
	hConnect = ::InternetConnect(hOpen, url_components.lpszHostName, url_components.nPort,
				     "", "", INTERNET_SERVICE_HTTP, NULL, NULL);

	if (hConnect == NULL) {
	    __leave;
	}   

	// Make a HTTP HEAD request
	hRequest = ::HttpOpenRequest(hConnect, "HEAD", szUrlPath, "HTTP/1.1", "", NULL,
			INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_NO_AUTH, 0);

	if (hRequest == NULL) {
		__leave;
	}   

	if ( !::HttpSendRequest(hRequest, NULL, NULL, NULL, NULL)) {
	    DWORD dwErrorCode = GetLastError();
	    switch(dwErrorCode) {
	        case 12002: // timeout:
	        case 12007: // name not resolved:
	        case 12029: //can not connect
	        {
                    ret = E_FAIL;
	        }
                default:
                     break;
	    }
	} 

        dwVal = 0;
        if (::HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER |
                                     HTTP_QUERY_STATUS_CODE, &dwVal,
                                     &dwValSize, NULL)) {
             if (dwVal == HTTP_STATUS_PROXY_AUTH_REQ)
                 ret = ERROR_PROXY_AUTH_REQ;
	     else if (dwVal == HTTP_STATUS_BAD_REQUEST || 
		      dwVal == HTTP_STATUS_NOT_FOUND)
		 ret = ERROR_URL_NOT_FOUND;
        }

    }
    __finally
    {
	if (hRequest)
	    ::InternetCloseHandle(hRequest);
	if (hConnect)
	    ::InternetCloseHandle(hConnect);
	if (hOpen)
	    ::InternetCloseHandle(hOpen);
    }
    return ret;
}

BOOL IsThisMsi(LPCTSTR lpPath)
{  
  if(lstrlen(lpPath)>4 && 
     lstrcmpi(lpPath+ lstrlen(lpPath) - 4, ".msi")==0)
    return TRUE;
  return FALSE;
}

void GetJInstallUserAgent(LPTSTR  lpszUserAgent, DWORD dwCount)
{
    TCHAR szUserAgent[BUFFER_SIZE];

    CRegKey swKey, msKey, winKey, cvKey, isKey;

    wsprintf(lpszUserAgent, "Mozilla/4.0 (compatible; Win32; jinstall)");

    if (swKey.Open(HKEY_CURRENT_USER, "Software", KEY_READ) != ERROR_SUCCESS)
        return;
    if (msKey.Open(swKey, "MicroSoft", KEY_READ) != ERROR_SUCCESS)
        return;
    if (winKey.Open(msKey, "Windows", KEY_READ) != ERROR_SUCCESS)
        return;
    if (cvKey.Open(winKey, "CurrentVersion", KEY_READ) != ERROR_SUCCESS)
        return;
    if (isKey.Open(cvKey, "Internet Settings", KEY_READ) != ERROR_SUCCESS)
        return;

    if (isKey.QueryValue(szUserAgent, "User Agent", &dwCount) == ERROR_SUCCESS)
    {
        char *p = strrchr(szUserAgent, ')');
        if (p) {
            lstrcpyn(lpszUserAgent, szUserAgent, p-szUserAgent+1);
            lstrcat(lpszUserAgent, "; jinstall");
            lstrcat(lpszUserAgent, p);
        }
    }
}

LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL) {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL) {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}


void addSilentOptions(LPCTSTR lpszExec, LPSTR lpszOptions)
{
    TCHAR szOptions[BUFFER_SIZE];

    BOOL bSilent = (strstr(lpszOptions, "/qn") != 0);
    if (IsThisMsi(lpszExec)) {
	// only add /qn to the options
        if (!bSilent) {
	    lstrcpy(szOptions, "/qn ");
	    lstrcat(szOptions, lpszOptions);
	    lstrcpy(lpszOptions, szOptions);
	}
    }
    else {
        bSilent = (strstr(lpszOptions, "/s") != 0);
        if (!bSilent) {
	    lstrcpy(szOptions, "/s ");
	    const char* pos = strstr(lpszOptions, "/v");
	    if (pos == NULL) lstrcat(szOptions, "/v");
	    pos = strstr(lpszOptions, "\"");
	    if (pos == NULL) {
	        lstrcat(szOptions, lpszOptions);
	        lstrcat(szOptions, "\"/qn\"");
	    }
	    else {
	        TCHAR temp[BUFFER_SIZE];
	        strncpy(temp, lpszOptions, pos-lpszOptions);  
	        temp[pos-lpszOptions] = '\0';
	        lstrcat(szOptions, temp);
	        lstrcat(szOptions, "\"/qn ");
	        lstrcat(szOptions, pos+1); 
	    }
	    lstrcpy(lpszOptions, szOptions);
        }
    }
}


//=--------------------------------------------------------------------------=
//  CheckAvailDiskSpace - It takes a drive path and returns the number of bytes.
//
//  This function is used to determine the amount of free space in a 
//  particular drive.
//=--------------------------------------------------------------------------=
BOOL CheckAvailDiskSpace(LPCTSTR lpszPath, DWORD dwReqDiskSpace)
{
    ULARGE_INTEGER dwFreeBytesToCaller, dwTotalNumBytes, dwTotalFreeBytes;
    BOOL bRet = 0;
    DWORD DiskFreeSpace= 0;
    DWORD dwMegaByte = 1024 * 1024;

    dwFreeBytesToCaller.LowPart = 0;
    dwFreeBytesToCaller.HighPart = 0;
    dwTotalFreeBytes.LowPart = 0;
    dwTotalFreeBytes.HighPart = 0;

    bRet = GetDiskFreeSpaceEx( lpszPath, &dwFreeBytesToCaller, 
                                &dwTotalNumBytes, &dwTotalFreeBytes);
     if (!bRet) {
           DWORD  dwSectsPerCluster = 0L, dwBytesPerSect = 0L; 
           DWORD  dwNumOfFreeClusters = 0L, dwTotalNumOfClusters = 0L ;
            bRet = ::GetDiskFreeSpace(lpszPath, 
				&dwSectsPerCluster, &dwBytesPerSect,
                    		&dwNumOfFreeClusters, &dwTotalNumOfClusters);
           if (!bRet)  return TRUE;
           DiskFreeSpace = (dwBytesPerSect*dwSectsPerCluster*dwNumOfFreeClusters/1024);
           DiskFreeSpace = DiskFreeSpace/1024; //MB
     }
     else {
      	DiskFreeSpace = dwFreeBytesToCaller.LowPart/dwMegaByte
      	                + dwFreeBytesToCaller.HighPart*(4*1024);
     }
     return (DiskFreeSpace >= dwReqDiskSpace);
}

DWORD ExecProcess(LPCTSTR lpszExecutableName, LPTSTR lpszCommandLine)
{
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    DWORD error = ERROR_UNKNOWN;

    // Prepare the startup structure
    ::ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    ::ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
    startupInfo.cb = sizeof(STARTUPINFO);

    BOOL bOK = ::CreateProcess(lpszExecutableName, lpszCommandLine, NULL, NULL, 
			FALSE, NULL, NULL, NULL, &startupInfo, &processInfo);
    if (bOK) {
        // This function call is very important to avoid deadlock.
	AtlWaitWithMessageLoop(processInfo.hProcess);

	GetExitCodeProcess(processInfo.hProcess, &error);
	::CloseHandle(processInfo.hProcess);
    }
    else {
        error = GetLastError();
	// Either file does not exist or some other errors.
        DownloadFailure();
    }
    return error;
}
