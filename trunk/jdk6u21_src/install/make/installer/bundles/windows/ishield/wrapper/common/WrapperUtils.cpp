/*
 * @(#)WrapperUtils.cpp	1.18 10/04/03
 * 
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
#include "UserProfile.h"
#include "WrapperUtils.h"
#include "CommonResource.h"

#define REG_WI_LOCATION_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer"
#define REG_JAVASOFTAU_KEY "SOFTWARE\\JavaSoft\\Auto Update"

//This is a place holder function for when we need all of the wrappers
//to run something at the end of an install.  It gets called by online,
//offline JRE wrappers and the JDK wrapper
void RunStuffPostInstall(){

}

BOOL IsThisJDK()
{
   if (lstrcmpi(BUNDLE, "jdk") == 0)
     return TRUE;
   return FALSE;
}

/* We only support 11 languages in InstallShield installer 
 * other than 0x804 and 0x416(exact match),
 * all others are matched by primary langID
 * and default is English(1033).
*/
typedef struct LCIDtoIShieldLCID {
    LCID    winID;
    LANGID   IShieldLCID;
} LCIDtoIShieldLCID;

static LCIDtoIShieldLCID IShieldLCIDMap[] = {
    0x804, 2052,   // 0x804 - Chinese Simplified
    0x04,  1028,   // 0x404 - Chinese Traditional
    0x07,  1031,   // 0x407 - German
    0x09,  1033,   // 0x409 - English
    0x0a,  1034,   // 0x40a - Spanish Traditional
    0x0c,  1036,   // 0x40c - French Standard
    0x10,  1040,   // 0x410 - Italian Standard
    0x11,  1041,   // 0x411 - Japanese
    0x12,  1042,   // 0x412 - Korean
    0x416, 1046,   // 0x416 - Brazilian Portuguese
    0x1d,  1053   // 0x41d - Swedish
};

LANGID DetectLocale() 
{
    LANGID langid, ishieldid;
    int i;
    int len = sizeof(IShieldLCIDMap) / sizeof(LCIDtoIShieldLCID);

    ishieldid = LANGID_ENGLISH; // English is the default
    langid = GetUserDefaultLangID();

    // first compare to Chinese Simplified
    if (langid == LANGID_CHINESE_SIMPLIFIED)
        return LANGID_CHINESE_SIMPLIFIED;

    // second compare to Brazilian Portuguese
    // Return LANGID_PORTUGUESE_BRAZILIAN before langid = langid & 0xff
    // If using & 0xff, the return value will be for both pt_BR and pt_PT
    // but we only want to return pt_BR for the installer
    // SDK is not localized into pt_BR, if it's JDK, still need to return LANGID_ENGLISH
    if ((!IsThisJDK()) && (langid == LANGID_PORTUGUESE_BRAZILIAN)) {
        return LANGID_PORTUGUESE_BRAZILIAN;
    }

    langid = langid & 0xff;
    if (IsThisJDK()) {
        //SDK has only English & Japanese MST files, in addition to
        //Chinese Simplified
        if (langid == IShieldLCIDMap[JAPANESE_INDEX].winID)
	{
	  ishieldid = IShieldLCIDMap[JAPANESE_INDEX].IShieldLCID;
	}
    }
    else {
        for (i=0; i<len; i++) {
            if (langid == IShieldLCIDMap[i].winID) {
                ishieldid = IShieldLCIDMap[i].IShieldLCID;
                break;
            }
	}
    }
    return ishieldid;
}

BOOL IsThisValidLang(LANGID lang)
{
    if (IsThisJDK()) {
	//SDK has only English, Chinese Simplified & Japanese MST files
	if ((lang == LANGID_ENGLISH)
	    || (lang == IShieldLCIDMap[JAPANESE_INDEX].IShieldLCID)
	    || (lang == LANGID_CHINESE_SIMPLIFIED))
	    return TRUE;
	else
	    return FALSE;
    }
    else {
	int len = sizeof(IShieldLCIDMap) / sizeof(LCIDtoIShieldLCID);
	for (int i=0; i<len; i++) {
            if (lang == IShieldLCIDMap[i].IShieldLCID)
		return TRUE;
	}
        return FALSE;
    }
}


//=--------------------------------------------------------------------------=
//  IsFileExists -- Check if file exists
//=--------------------------------------------------------------------------=
BOOL IsFileExists(LPCSTR lpszFileName)
{
    WIN32_FIND_DATA FindFileData;

    HANDLE hFind = FindFirstFile(lpszFileName, &FindFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    ::FindClose(hFind);
    return TRUE;
}

void GetShortTempPath(DWORD dwLen, LPTSTR szTempPath)
{
    TCHAR szLongPath[BUFFER_SIZE];

    GetTempPath(BUFFER_SIZE, szLongPath);
    if (GetShortPathName(szLongPath, szTempPath, dwLen) == 0)
        lstrcpy(szTempPath, szLongPath);
}

BOOL IsOSWin2003()
{
  OSVERSIONINFO  osvi;

  // Initialize the OSVERSIONINFO structure.
  ZeroMemory( &osvi, sizeof( osvi ) );
  osvi.dwOSVersionInfoSize = sizeof( osvi );

  GetVersionEx( &osvi );  // Assume this function succeeds.

  if (( osvi.dwMajorVersion == 5)
      && (osvi.dwMinorVersion == 2 )) //2003
    return(TRUE);
  else
    return(FALSE);
}

BOOL IsWIVerValid(LPDWORD lpdwFileVerMS)
{
    BOOL ret = FALSE;
    char buf2[BUFFER_SIZE];
    char buf[BUFFER_SIZE];

    GetSystemDirectory(buf, BUFFER_SIZE);
    lstrcat(buf, "\\msi.dll");

    if (GetFileVersionInfo(buf, 0, BUFFER_SIZE, (void *) buf2)) {
        UINT size;
        void *ptr;
        // minimum required version is 2.0.2600.0
        if (VerQueryValue((void *) buf2, "\\" , &ptr, &size)) {
           VS_FIXEDFILEINFO *vsptr = (VS_FIXEDFILEINFO *) ptr;
           if ((vsptr->dwFileVersionMS > 0x20000) || 
                ((vsptr->dwFileVersionMS == 0x20000) &&
		 (vsptr->dwFileVersionLS >= 0xa280000))) {
               ret = TRUE;
               if (lpdwFileVerMS != NULL)
                   *lpdwFileVerMS =  vsptr->dwFileVersionMS;
           }
        }
    }
    return ret;
}

BOOL GetWIPath(LPTSTR lpszWIPath, DWORD dwSize)
{
    HKEY hKey;
    DWORD dwType;
    BOOL bRet = FALSE;
    BOOL bWin2003 = IsOSWin2003();

    if(bWin2003) {
	GetSystemDirectory(lpszWIPath, dwSize);
	bRet = TRUE;
    }
    else {
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_WI_LOCATION_KEY, 0, KEY_READ, 
			 &hKey) == ERROR_SUCCESS) {
	  if (RegQueryValueEx(hKey, "InstallerLocation", NULL, &dwType,
			      (LPBYTE)lpszWIPath, &dwSize) == ERROR_SUCCESS) {
	      bRet=TRUE;
	  }
	  RegCloseKey(hKey);
	}
    }
    if (bRet) lstrcat(lpszWIPath, "\\msiexec.exe");
    return bRet;
}


#ifdef EXTRA_COMP_LIB_NAME

// Stringifier macros to get the library name

#define JI_STRING(x) #x
#define JI_GETSTRING(x) JI_STRING(x)

/*
 * Write a resource out to a file and return TRUE for success, FALSE for
 * failure. Parameter hInst is ordinarily a handle to the executing program,
 * id is the resource id and lpType is the resource type.
 * The filePath parm must point to a Windows "short pathname."
 */

BOOL WriteFileFromResource(HINSTANCE hInst, unsigned int id, LPCTSTR lpType, 
    LPCTSTR filePath)
{
    // Look up resources
    HRSRC hrSrc = ::FindResource(hInst, (LPCSTR) id, lpType);

    if (hrSrc == NULL) {
        return FALSE;
    }

    // Determine size of resources
    DWORD dwSize = SizeofResource(hInst, hrSrc);

    // Load the resource
    HGLOBAL hRes = ::LoadResource(hInst, hrSrc);
    if (hRes == NULL) {
        return false;
    }
    
    // lock it
    LPVOID lpVoid = ::LockResource(hRes);
    if (lpVoid == NULL) {
        return false;
    }

    // Create specified file
    HANDLE hFile = ::CreateFile(filePath, GENERIC_WRITE, 0, NULL, 
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    // Write the resource into the file
    DWORD dwTotalNumberOfBytesWritten = 0;

    while (dwTotalNumberOfBytesWritten < dwSize) {
        DWORD dwNumberOfBytesWritten = 0;

        if (!::WriteFile(hFile, lpVoid, min(8192, 
            dwSize-dwTotalNumberOfBytesWritten), 
            &dwNumberOfBytesWritten, NULL)) {

            // FIXME: check for "disk full" and give the user a clue and a
            // chance to fix this. Needs changes out in the Java code.
            // This pertains to the current bundle security check code too,
            // as well as the jbroker fixes.

            // Write failed: close the handle, delete the file and bail out
            ::CloseHandle(hFile);
            ::DeleteFile(filePath);
            return FALSE;
        }

        lpVoid = (LPVOID)(LPBYTE(lpVoid) + dwNumberOfBytesWritten); 
        dwTotalNumberOfBytesWritten += dwNumberOfBytesWritten;
    } 

    // Close file
    if (!::CloseHandle(hFile)) {

        // Close failed: delete the file and bail out
        ::DeleteFile(filePath);
        return FALSE;
    }

    return TRUE;
}

// Extract the extra compression library into a .dll file and link to it if 
// this hasn't already been done. Then invoke the decoder to uncompress 
// the file specified by parm filePath into a temp file in directory dirPath.
// Then delete the original file and rename the temp to filePath.
// 
// Return TRUE for "OK", FALSE for "failed".
// TODO: share with kernel.cpp some how

typedef int (*EXTRACOMPPTRTYPE) (int, const char**);
static volatile EXTRACOMPPTRTYPE mptr = NULL;
HMODULE extraCompLibHandle = NULL;

BOOL ExtraUnCompression(HINSTANCE hInstance, LPCTSTR longDirPath, LPCTSTR longCompressedFilePath)
{
    TCHAR dirPath[MAX_PATH+1] = {NULL};
    TCHAR compressedFilePath[MAX_PATH+1] = {NULL};

    DWORD shortLengthDirPath = GetShortPathName(longDirPath, dirPath, MAX_PATH);
    DWORD shortLengthCompressedFilePath = GetShortPathName(longCompressedFilePath, compressedFilePath, MAX_PATH);

    if (mptr == NULL) {
        // Load the library and develop a pointer to the decoder routine
        TCHAR extraCompPath[MAX_PATH*3] = {NULL};
        wsprintf(extraCompPath, "%s\\%s", dirPath, 
                 JI_GETSTRING(EXTRA_COMP_LIB_NAME));
        if (WriteFileFromResource(hInstance, IDP_EXTRA_COMP_LIB, 
                                  "JAVA_INSTALLER_UTIL", extraCompPath)) { 

            extraCompLibHandle = LoadLibrary(extraCompPath);
            if (extraCompLibHandle == NULL) {
                return FALSE;
            }

            // find the decoder procedure

            mptr = (EXTRACOMPPTRTYPE) GetProcAddress(extraCompLibHandle, 
                                                     "ExtraCompressionMain");

            if (mptr == NULL) {
                return FALSE;
            }

            // everything fine: ready to decode now and with any other
            // invocation

        } else {
            return FALSE;
        }
    }

    // Now set up a temporary path to the uncompressed file
    TCHAR tmpFilePath[MAX_PATH+1] = {NULL};
    wsprintf(tmpFilePath, "%s\\msi.tmp", dirPath);
    // Make sure it doesn't already exist
    ::DeleteFile(tmpFilePath);

    // Create the arguments for the decoder
    // Decoder options must go *between* the "d" argument and the "filePath"
    // arguments.
    // TODO: smarten this up as in kernel.cpp. Consider factoring out
    // common code.
    const char *args[] = {
        "", 
        "d",
        // any option switches for command should go here
	"-q",
        // end of extra option switches
        compressedFilePath,
        tmpFilePath};

    int argc = sizeof(args) / sizeof(const char*);
    // Decode the file
    if ((*mptr)(argc, args) != 0) {
        // Decode failed. Try to delete the tmp file in case it
        // got created and then return "failed."
        ::DeleteFile(tmpFilePath);
        return FALSE;
    }

    if (::DeleteFile(compressedFilePath) == 0) {
        // Delete of original file failed. Try to get rid of the
        // temp file and then fail.
        ::DeleteFile(tmpFilePath);
        return FALSE;
    } else {
        // Now give the uncompressed temp file the original file path
        if (::MoveFile(tmpFilePath, compressedFilePath) == 0) {
            // Rename failed. Try to remove both files.
            ::DeleteFile(compressedFilePath);
            ::DeleteFile(tmpFilePath);
            // TODO: get official status code
            return FALSE;
        }
        return TRUE;
    }
}

/**
 * Unload extra compression library (e.g. so the .msi installer can move it).
 */

void ReleaseExtraCompression(HINSTANCE hInst) {
    BOOL fFreeResult = FALSE;
    // If the handle is valid, try to free the library
    if (extraCompLibHandle != NULL) {  
        // Free the DLL module.
        fFreeResult = FreeLibrary(extraCompLibHandle);
    }  
    mptr = NULL;
}

#endif // EXTRA_COMP_LIB_NAME

void ErrorExit(LPTSTR lpszFunction) 
{ 
  LPTSTR lpMsgBuf;
  LPTSTR lpDisplayBuf;
  DWORD dw = GetLastError(); 

  FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

  lpDisplayBuf = (LPTSTR) LocalAlloc(LMEM_ZEROINIT, 
			    strlen(lpMsgBuf)+strlen(lpszFunction)+40); 
  wsprintf(lpDisplayBuf, "%s failed with error %d: %s", 
	   lpszFunction, dw, lpMsgBuf); 
  MessageBox(NULL, (LPTSTR) lpDisplayBuf, "Error", MB_OK); 

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
  ExitProcess(dw); 
}


BOOL ExtractFileFromResource(HINSTANCE hInst, LPCTSTR lpName, LPCTSTR lpType, LPCTSTR lpszFile)
{
    // Look up resources
    HRSRC hrSrc = ::FindResource(hInst, lpName, lpType);

    if (hrSrc == NULL)
	return FALSE;

    // Determine size of resources
    DWORD dwSize = SizeofResource(hInst, hrSrc);

    // Load and lock resources
    HGLOBAL hRes = ::LoadResource(hInst, hrSrc);
    LPVOID lpVoid = ::LockResource(hRes);

   // Create file
    HANDLE hFile = ::CreateFile(lpszFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if(hFile == INVALID_HANDLE_VALUE) {
	ErrorExit("Installer : Wrapper.CreateFile ");
    }

    // Write the resources into the file
    DWORD dwTotalNumberOfBytesWritten = 0;

    while (dwTotalNumberOfBytesWritten < dwSize) {
        DWORD dwNumberOfBytesWritten = 0;

        if (!::WriteFile(hFile, lpVoid, min(8192, dwSize-dwTotalNumberOfBytesWritten), &dwNumberOfBytesWritten, NULL)) {
	  ErrorExit("Installer : Wrapper.WriteFile (In small chunks) ");
	}

	// should we use a larger buffer for better IO performance?
        //::WriteFile(hFile, lpVoid, 65536, &dwNumberOfBytesWritten, NULL);

        //lpVoid += dwNumberOfBytesWritten;
        lpVoid = (LPVOID)(LPBYTE(lpVoid) + dwNumberOfBytesWritten); 
        dwTotalNumberOfBytesWritten += dwNumberOfBytesWritten;
    } 

    // Close file
    if (!::CloseHandle(hFile)) {
      ErrorExit("Installer : Wrapper.CloseHandle ");
    }

    return TRUE;
}

//
// Returns TRUE if the system is 64-bit.
//
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

BOOL IsSystem64bit()
{
    SYSTEM_INFO si;
    PGNSI pGNSI;

    ZeroMemory(&si, sizeof(SYSTEM_INFO));

    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = (PGNSI) GetProcAddress(
                            GetModuleHandle(TEXT("kernel32.dll")),
                            "GetNativeSystemInfo");
    if(NULL != pGNSI) {
        pGNSI(&si);
    } else {
        GetSystemInfo(&si);
    }

    return(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64);
}


// Extract the sponsor library into a .dll file
// 
// Return TRUE for "OK", FALSE for "failed".

BOOL ExtractSponsorDLL(HINSTANCE hInstance) {

    TCHAR szAppDataSunPath[MAX_PATH] = {0};
    TCHAR szDllFilename[MAX_PATH] = {0};

    GetUserShellFolder(szAppDataSunPath); 

    wsprintf(szAppDataSunPath, "%s\\Sun\\", szAppDataSunPath);
    CreateDirectory(szAppDataSunPath, NULL);
    wsprintf(szAppDataSunPath, "%sJava\\", szAppDataSunPath);
    CreateDirectory(szAppDataSunPath, NULL);
    wsprintf(szAppDataSunPath, "%s%s%s", szAppDataSunPath, BUNDLE, VERSION);
    CreateDirectory(szAppDataSunPath, NULL);

    wsprintf(szDllFilename, "%s\\gtapi.dll", szAppDataSunPath);
 
    if (ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_SPONSOR1DLL), "SPONSOR_DLL", szDllFilename)) {
        return TRUE;
    }
  
    return FALSE;
  
}

// Entry point for all wrappers (online, offline, JDK) to
// check for and handle outdated versions of Auto Update
DWORD RemoveAnyOlderAU2X()
{
    
    // Detect older AU installations, and remove them
    if (OlderAUExists()){
        
        // UninstallAU2X will return error if any problems are encountered
        // removing outdated versions of AU
        return UninstallAU2X();
        
    }
    
    // AU either does not exist, or is => the bundled version
    // So just return ERROR_SUCCESS
    return ERROR_SUCCESS;
    
}

// Puts the AUVersion value from the registry into the  
// LPCTSTR lpszAuVersion paramter.  
// Returns TRUE if the value was found and read successfully, 
//  FALSE otherwise.
BOOL GetAuVersion(LPCTSTR lpszAuVersion, DWORD dwBufferSize) {
    BOOL retVal = FALSE;
    // Get the installed version of AU
    // if the HKLM\\Software\\Auto Update\\AUVersion key 
    // value is NULL, AU 2.0+ is not installed, so return FALSE
    HKEY hJavaSoftAUKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                     REG_JAVASOFTAU_KEY, 
                     0, 
                     KEY_READ, 
                     &hJavaSoftAUKey) == ERROR_SUCCESS){
        
        
        // Get the installed AU Version via the windows registry
        DWORD dwType;
        DWORD dwSize = dwBufferSize;
        if (RegQueryValueEx(hJavaSoftAUKey,
                            "AUVersion", 
                            NULL, 
                            &dwType,
                            (LPBYTE) lpszAuVersion, 
                            &dwSize) == ERROR_SUCCESS) {
                                retVal = TRUE;
                            }
    }
    
    RegCloseKey(hJavaSoftAUKey);
    return retVal;
    
}

// Returns TRUE is there is an older version of AU
// Installed on the host. Compares the installed version
// with the bundled version, which is set at build time
BOOL OlderAUExists()
{
    TCHAR szInstalledAUVersion[BUFFER_SIZE] = {NULL};
    DWORD dwSize = BUFFER_SIZE;
            
    if( GetAuVersion(szInstalledAUVersion, dwSize) ) {
        // Return TRUE (older AU version installed) if the
        // Installed version is < the bundled version
        // Note: the lstrcmp comparison works with any revision
        // format, so even if AU version is x.x.x.x format in the 
        // future, this will not need to be updated
        if (lstrcmp(szInstalledAUVersion, BUNDLED_AUVERSION) < 0){
            return TRUE;
        }
    }
            
    // No older AU detected, return FALSE
    return FALSE;
    
}

// Calls msiexec to uninstall the AU MSI silently
// returns the value returned by ExecProcess()
// Note: this function will only install AU versions 2.0+
DWORD UninstallAU2X()
{
    
    DWORD dwError = ERROR_SUCCESS;
    
    // Get path to msiexec.exe   
    TCHAR szMSIExec[BUFFER_SIZE] = {NULL};
    GetWIPath(szMSIExec, sizeof(szMSIExec));
    
    // Initialize startup data
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInfo;
    ::ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    ::ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
    startupInfo.cb = sizeof(STARTUPINFO);
    
    // Uninstall the AU GUID silently
    if (::CreateProcess(szMSIExec, 
                        " /x {4A03706F-666A-4037-7777-5F2748764D10} /qn", 
                        NULL, 
                        NULL, 
                        FALSE, 
                        NULL, 
                        NULL, 
                        NULL, 
                        &startupInfo, 
                        &processInfo)){
        
        // This function call is very important to avoid deadlock.
        AtlWaitWithMessageLoop(processInfo.hProcess);
        GetExitCodeProcess(processInfo.hProcess, &dwError);
        ::CloseHandle(processInfo.hProcess);
        
    } else {
        
        dwError = GetLastError();
    
    }
        
    return dwError;
        
}


