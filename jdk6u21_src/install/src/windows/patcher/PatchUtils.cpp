/*
 * @(#)PatchUtils.cpp	1.57 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// PatchUtils.cpp : Implementation of Patching utilities.
//
// By Stanley Man-Kit Ho

#include "stdafx.h"
#include "resource.h"
#include <stdio.h>
#include <ctype.h>
#include "PatchUtils.h"
#include "patchwin.h"


typedef LPVOID (CALLBACK* PATCHCALLBACK)(UINT, LPVOID);
typedef UINT (CALLBACK* LPFNRTPatchApply32)(LPSTR, PATCHCALLBACK, BOOL);


// Critical section for protect accessing trace
// file from multiple threads if patch tracing 
// is enabled
static CComAutoCriticalSection csPatchTrace;

// Trace file for patch tracing output
static FILE* traceFile = NULL;

void OPEN_PATCH_TRACEFILE()
{
    char buf[BUFFER_SIZE];

    // Use critical section to protect accessing  
    // the trace file from multiple threads
    __try
    {
	csPatchTrace.Lock();
	
	GetSystemDirectory(buf, BUFFER_SIZE);
	lstrcat(buf, "\\");
	lstrcat(buf, "jupdate-"NEW_IMAGE_FULLVERSION".log");
	traceFile = fopen(buf, "w");
    }
    __finally
    {
	csPatchTrace.Unlock();
    }
}


void CLOSE_PATCH_TRACEFILE()
{
    // Use critical section to protect accessing  
    // the trace file from multiple threads
    __try
    {
	csPatchTrace.Lock();

	if (traceFile)
	    fclose(traceFile);
    }
    __finally
    {
	csPatchTrace.Unlock();
    }
}


void PATCH_TRACE(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);

    int nBuf;
    char szBuffer[2048];

    nBuf = _vsnprintf(szBuffer, sizeof(szBuffer), lpszFormat, args);
    //ATLASSERT(nBuf < sizeof(szBuffer)); //Output truncated as it was > sizeof(szBuffer)

    OutputDebugStringA(szBuffer);
    va_end(args);

    // Use critical section to protect accessing
    // the trace file from multiple threads
    __try
    {
	csPatchTrace.Lock();

	// Output to trace file
	if (traceFile)
	    fprintf(traceFile, szBuffer);
    }
    __finally
    {
	csPatchTrace.Unlock();
    }
}



//---------------------------------------------------------------
// Dispatch messages in the Windows Event Queue
//
BOOL DispatchMessageInQueue()
{
    MSG msg;

    // There is one or more window message available. Dispatch them
    while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
    {
	TranslateMessage(&msg);
	DispatchMessage(&msg);
    }

    return TRUE;
}

//---------------------------------------------------------------
// Move directory from source to destination
//
// @param lpszSourceDir Path to source directory
// @param lpszDestDir Path to destination directory
// @return TRUE if operation succeeds
//
BOOL MoveDirectory(LPCTSTR lpszSourceDir, LPCTSTR lpszDestDir)
{
    PATCH_TRACE("Move %s to %s\n", lpszSourceDir, lpszDestDir);

    BOOL bRet = ::MoveFile(lpszSourceDir, lpszDestDir);

    if (bRet == FALSE)
	PATCH_TRACE("FAIL: Move from %s to %s\n", lpszSourceDir, lpszDestDir);

    return bRet;	
}


//---------------------------------------------------------------
// Extract file from resources
//
// @param lpName Resource name
// @param lpType Resource type
// @param lpszFile Destination file
// @return TRUE if operation succeeds
//
BOOL ExtractFileFromResource(LPCTSTR lpName, LPCTSTR lpType, LPCTSTR lpszFile)
{
    // Look up resources
    HRSRC hrSrc = ::FindResource(_Module.GetResourceInstance(), lpName, lpType);

    if (hrSrc == NULL)
	return FALSE;

    // Determine size of resources
    DWORD dwSize = SizeofResource(_Module.GetResourceInstance(), hrSrc);

    // Load and lock resources
    HGLOBAL hRes = ::LoadResource(_Module.GetResourceInstance(), hrSrc);
    LPVOID lpVoid = ::LockResource(hRes);

    // Create file
    HANDLE hFile = ::CreateFile(lpszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    // Write the resources into the file
    DWORD dwNumberOfBytesWritten = 0;
    ::WriteFile(hFile, lpVoid, dwSize, &dwNumberOfBytesWritten, NULL);

    // Close file
    ::CloseHandle(hFile);

    return TRUE;
}


//---------------------------------------------------------------
// Apply patch to a directory
//
// @param lpszDirectory Path to apply the patch
// @param lpszOptions Options passed to patch engine
// @param lpfnCallBack Callback for status
// @return TRUE if operation succeeds
//
BOOL ApplyPatch(LPCTSTR lpszDirectory, LPCTSTR lpszOptions,
		LPVOID (CALLBACK *lpfnCallBack)(UINT  Id, LPVOID lpParm))
{
    PATCH_TRACE("Patching %s\n", lpszDirectory);

    char lpszDirectoryShort[1024];
    TCHAR lpszDirectoryLong[1024];

    wsprintf(lpszDirectoryLong, lpszDirectory);

    ::GetShortPathName(lpszDirectory, lpszDirectoryShort, MAX_PATH);

    // Notify users - backup begin
    if (lpfnCallBack)
	lpfnCallBack(WM_BACKUP_BEGIN, NULL);

    // Patching begin
    //
    TCHAR szTempPatchFile[BUFFER_SIZE];
    wsprintf(szTempPatchFile, TEXT("%s\\..\\patch-%s.rtp"), lpszDirectoryShort, NEW_IMAGE_FULLVERSION);

    TCHAR szTempPatchEngine[BUFFER_SIZE];
    wsprintf(szTempPatchEngine, TEXT("%s\\..\\patchw32-%s.dll"), lpszDirectoryShort, NEW_IMAGE_FULLVERSION);

    // Handle of patch engine
    HMODULE hPatchEngine = NULL;

    __try
    {
	// Extract patch file from resources
	if (ExtractFileFromResource(MAKEINTRESOURCE(IDP_PATCH_FILE), "PATCH", szTempPatchFile) == FALSE)
	    return FALSE;

	// Extract patch engine from resources
	if (ExtractFileFromResource(MAKEINTRESOURCE(IDP_PATCH_ENGINE), "PATCH_ENGINE", szTempPatchEngine) == FALSE)
	    return FALSE;
    
	// Notify users - backup finish
	if (lpfnCallBack)
	    lpfnCallBack(WM_BACKUP_END, NULL);


	// Load Patch Engine Library
	hPatchEngine = ::LoadLibrary(szTempPatchEngine);

	// Look up patch function
	LPFNRTPatchApply32 lpfnRTPatchApply32 = (LPFNRTPatchApply32) ::GetProcAddress(hPatchEngine, "RTPatchApply32@12");


	// Apply patch to the temp directory

	TCHAR szCommandLine[BUFFER_SIZE];
	szCommandLine[0] = NULL;
	lstrcat(szCommandLine, "\"");
//	lstrcat(szCommandLine, szTempDir);
	lstrcat(szCommandLine, lpszDirectoryShort);
	lstrcat(szCommandLine, "\" \"");
	lstrcat(szCommandLine, szTempPatchFile);
	lstrcat(szCommandLine, "\"");

	DWORD retCode = lpfnRTPatchApply32(szCommandLine, lpfnCallBack, TRUE);

	// Patch doesn't succeed, rollback
	if (retCode != 0)
	    return FALSE;
    }
    __finally
    {
	if (hPatchEngine)
	    ::FreeLibrary(hPatchEngine);

	// Remove patch file
	::DeleteFile(szTempPatchFile);

	// Remove patch engine
	::DeleteFile(szTempPatchEngine);
    }

    MoveDirectory(lpszDirectoryShort, lpszDirectoryLong);

    return TRUE;
}


//--------------------------------------------------------------------------
// Called when patching is in progress in silent mode
//
// @param id Message ID
// @param param Parameter
// @return LVOID
//
LPVOID WINAPI SilentUpdateCallBack(UINT id, LPVOID param)
{ 
    const char* szStatusText = (const char*) param;
    char szBuffer[BUFFER_SIZE], szMessage[BUFFER_SIZE], szCaption[BUFFER_SIZE];
    static char szFileEntry[BUFFER_SIZE];
    static char szWarningHeader[BUFFER_SIZE];

    switch( id )
    {
	case 1:	  // Warning message header
	{
	    wsprintf(szWarningHeader, "%s", szStatusText);
	    break;		
	}

	case 3:   // Error message header
	case 5:	  // % complete
	case 8:   // Current file patch complete
	case 9:   // Progress message
	case 0xa: // Help message
	case 0xb: // Patch file comment
	case 0xc: // Set copyright string

	    // these are all text output...
	    break;

	case 2:
        {
	    // warning 
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_WARNING, szCaption, BUFFER_SIZE);

	    PATCH_TRACE(szStatusText);
	    PATCH_TRACE("\n");

	    // Popup warning dialog for most warnings, except
	    // wpt0015: Old File does not exist
	    // wpt0016: New File already exists
	    // wpt0024: New File already exists
	    // wpt0037: Error opening PATCH.ERR
	    //
	    if (strstr(szWarningHeader, "wpt0015") == NULL 
		&& strstr(szWarningHeader, "wpt0016") == NULL
		&& strstr(szWarningHeader, "wpt0024") == NULL
		&& strstr(szWarningHeader, "wpt0037") == NULL)
	    {
		wsprintf(szBuffer, "%s: %s", szFileEntry, szStatusText);

		::MessageBox(NULL, szBuffer, szCaption, MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	    }

	    break;
	}

	case 4:
        {
	    // error
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    ::MessageBox(NULL,  szStatusText, szCaption, MB_ICONSTOP | MB_OK);

	    PATCH_TRACE(szStatusText);
	    PATCH_TRACE("\n");
	    break;		
	}
	case 7:
	// File patch begin
	{
	    wsprintf(szFileEntry, "%s", szStatusText);

	    ::LoadString(_Module.GetResourceInstance(), IDS_STATUS_PROCESSING, szBuffer, BUFFER_SIZE);
	    wsprintf(szMessage, szBuffer, szStatusText);

	    PATCH_TRACE(szMessage);
	    PATCH_TRACE("\n");
	    break;
	}

	case 0xd: // Patch file dialog
	case 0xe: // Invalid patch file alert
        case 0xf: // Password dialog
	case 0x10: // Invalid password alert
	case 0x11: // Next disk dialog
	case 0x12: // Invalid disk alert
	case 0x14: // Location dialog
        {
	    // this one shouldn't happen (only occurs if the patch file
	    //   file bound to the executable is invalid in some way).
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ABORT, szCaption, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_HANDLING, szBuffer, BUFFER_SIZE);
	    wsprintf(szMessage, szBuffer, id);

    	    PATCH_TRACE("Error: Location Dialog\n");

	    ::MessageBox(NULL, szMessage, szCaption, MB_ICONEXCLAMATION | MB_OK );

	    return NULL;
	}

	case 0x13:
	    // system location confirmation (always returns "Y" in this skeleton)
	    return "Y";
      
	case 0x16:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_STATUS_SEARCHING, szMessage, BUFFER_SIZE);
	    PATCH_TRACE(szMessage);
	    PATCH_TRACE("\n");
	    break;
	}
       
	case 0x15:
	default:
	    break;
    } 
  
    // do a few more messages while we're here...
  
    return "";
}

//---------------------------------------------------------------
// Retrieve a line from file stream
//
// @param f File stream
// @param lpszBuffer Buffer
//
void getline(FILE* f, char* lpszBuffer, int size, char c)
{
    for (int i=0; i < size; i++)
    {
	char ch = fgetc(f);
	if (ch == c || ch == EOF)
	    break;
	else
	    lpszBuffer[i] = ch;
    }

    lpszBuffer[i] = '\0'; 
}


//---------------------------------------------------------------
// Extract value from a string
//
//
void extractValue(const char* str, const char* begin, const char* end, char* lpszBuffer)
{
    const char* lpszBegin = strstr(str, begin);
    const char* lpszEnd = strstr(str, end);

    if (lpszBegin != NULL && lpszEnd != NULL)
    {
	const char* lpszIter = lpszBegin + strlen(begin);

	int i=0;

	for (i=0; lpszIter != lpszEnd; i++)
	{
	    lpszBuffer[i] = lpszIter[0];
	    lpszIter++;
	}
	
	lpszBuffer[i] = '\0';
    }
}

//---------------------------------------------------------------
// Retrieve patch info
//
// @param lpszDirectory Path to patched directory
// @param lpszVersionInfo Version Info
// @return TRUE if operation succeeds
//
BOOL RetrievePatchInfo(LPCTSTR lpszDirectory, LPTSTR lpszVersionInfo)
{
    TCHAR szPatchInfoFile[BUFFER_SIZE];
    TCHAR szMajorVersion[256], szMinorVersion[256], szMicroVersion[256], szUpdateVersion[256];
    TCHAR szMilestone[256], szBuildNumber[256];

    szMajorVersion[0] = NULL;
    szMinorVersion[0] = NULL;
    szMicroVersion[0] = NULL;
    szUpdateVersion[0] = NULL;
    szMilestone[0] = NULL;
    szBuildNumber[0] = NULL;

    wsprintf(szPatchInfoFile, "%s\\%s", lpszDirectory, VERSION_INFO_FILE);

    FILE* patchInfoFile = fopen(szPatchInfoFile, "r");

    if (patchInfoFile)
    {
	char szBuffer[1024];

	while (!feof(patchInfoFile))
	{
	    getline(patchInfoFile, szBuffer, 1024, '\n');

	    // Skip all comment
	    if (szBuffer[0] == '#')   
	    {
		// Goto next line
		continue;
	    }
	    else if (szBuffer[0] == '<' && szBuffer[1] == '?')
	    {
		// Goto next line
		continue;
	    }

	    extractValue(szBuffer, "<major>", "</major>", szMajorVersion);
	    extractValue(szBuffer, "<minor>", "</minor>", szMinorVersion);
	    extractValue(szBuffer, "<micro>", "</micro>", szMicroVersion);
	    extractValue(szBuffer, "<update>", "</update>", szUpdateVersion);
	    extractValue(szBuffer, "<milestone>", "</milestone>", szMilestone);
	    extractValue(szBuffer, "<build-number>", "</build-number>", szBuildNumber);
	}

	fclose(patchInfoFile);

	if (szUpdateVersion[0] != NULL)
	    wsprintf(lpszVersionInfo, "%s.%s.%s_%s", szMajorVersion, szMinorVersion, 
						    szMicroVersion, szUpdateVersion);
	else
	    wsprintf(lpszVersionInfo, "%s.%s.%s", szMajorVersion, szMinorVersion, szMicroVersion);

	// Append milestone
	if (szMilestone[0] != NULL)
	{
	    lstrcat(lpszVersionInfo, "-");
	    lstrcat(lpszVersionInfo, szMilestone);
	}

	// Append build number
	if (szBuildNumber[0] != NULL)
	{
	    lstrcat(lpszVersionInfo, "-b");
	    lstrcat(lpszVersionInfo, szBuildNumber);
	}

	return TRUE;
    }
    else
    {
	// If we can't find the version file, we determine the version 
	// through "java -fullversion"
	//
	TCHAR szCommandLine[BUFFER_SIZE];

	// Run command using fully quoted strings for java.exe command
	wsprintf(szCommandLine, "\"%s\\bin\\java.exe\" -fullversion", lpszDirectory);

	HANDLE hread[3], hwrite[3];
	SECURITY_ATTRIBUTES sa;

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = 0;
	sa.bInheritHandle = TRUE;

	memset(hread, 0, sizeof(hread));
	memset(hwrite, 0, sizeof(hwrite));

	// Create pipe to read stdin/stdout/stderr from another process
	//
	if (!(CreatePipe(&hread[0], &hwrite[0], &sa, 1024) &&
	      CreatePipe(&hread[1], &hwrite[1], &sa, 1024) &&
	      CreatePipe(&hread[2], &hwrite[2], &sa, 1024))) 
	{
	    CloseHandle(hread[0]);
	    CloseHandle(hread[1]);
	    CloseHandle(hread[2]);
	    CloseHandle(hwrite[0]);
	    CloseHandle(hwrite[1]);
	    CloseHandle(hwrite[2]);
	}

	
	STARTUPINFO si;

	::ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput  = hread[0];
	si.hStdOutput = hwrite[1];
	si.hStdError  = hwrite[2];

	SetHandleInformation(hwrite[0], HANDLE_FLAG_INHERIT, FALSE);
	SetHandleInformation(hread[1],  HANDLE_FLAG_INHERIT, FALSE);
	SetHandleInformation(hread[2],  HANDLE_FLAG_INHERIT, FALSE);


	PROCESS_INFORMATION pi;
	::ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	// Run "java -fullversion"
	//
	BOOL ret = ::CreateProcess(NULL, szCommandLine, NULL, NULL, 
				   TRUE, CREATE_NO_WINDOW | DETACHED_PROCESS,
				   NULL, NULL, &si, &pi);

	if (ret) 
	{
	    char szOutput[256];

	    DWORD dwByteRead = 0;

	    // Read output
	    ReadFile(hread[2], szOutput, 256, &dwByteRead, NULL);

	    // Extract version info
	    //
	    const char* versionInfo = strstr(szOutput, "\"");
	    const char* endInfo = strstr(versionInfo + 1, "\"");

	    strncpy(lpszVersionInfo, versionInfo + 1, endInfo - versionInfo - 1);
	    lpszVersionInfo[endInfo - versionInfo - 1] = '\0';
	}

	CloseHandle(hread[0]);
	CloseHandle(hread[1]);
	CloseHandle(hread[2]);
	CloseHandle(hwrite[0]);
	CloseHandle(hwrite[1]);
	CloseHandle(hwrite[2]);

	// wait for the 'java -fullversion' to be done. This will avoid
	// ApplyPatch() failing, since the source dir is still locked/in use.
	// maximum wait 2 minutes, don't block for too long, incase
	// there is a problem
	
	WaitForSingleObject(pi.hProcess, 120000);
	
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	
	return ret;
    }
}

// Display information
void DisplayInfo(BOOL bSilentMode, LPCTSTR lpszMessage, LPCTSTR lpszCaption)
{
    PATCH_TRACE("INFO: %s\n", lpszMessage);

    if (bSilentMode == FALSE)
	::MessageBox(NULL, lpszMessage, lpszCaption, MB_OK | MB_ICONINFORMATION);
}

// Display error
void DisplayError(BOOL bSilentMode, LPCTSTR lpszMessage, LPCTSTR lpszCaption)
{
    PATCH_TRACE("FAIL: %s\n", lpszMessage);

    // Will display error dialog no matter if it is in silent mode.
    ::MessageBox(NULL, lpszMessage, lpszCaption, MB_OK | MB_ICONSTOP);
}
