/*
 *  @(#)config.cpp	1.42 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "stdafx.h"
#include "common.h"
#include <jni.h>
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <shlobj.h>
#include <userenv.h>

#include <sys/types.h>
#include <sys/stat.h>

static const char UNINSTALL_KEY[] =
    "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

static const char DESKTOP_METRICS_KEY[] =
    "Control Panel\\Desktop\\WindowMetrics";

static const char ICON_SIZE_KEY[] =
    "Shell Icon Size";

char* javawsWideCharToMBCS(const unsigned short* uString);

extern "C" {

typedef HRESULT (WINAPI *LPFNSHGetFolderPath)(HWND hwndOwner,
	int nFolder, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

typedef BOOL (WINAPI *LPFNSHGetSpecialFolderPath)(HWND hwndOwner,
	LPTSTR lpszPath, int nFolder, BOOL fCreate);

typedef BOOL (WINAPI *LPFNGetAllUsersProfileDirectory)(
        LPTSTR lpszPath, LPDWORD lpdwSize);

typedef BOOL (WINAPI *LPFNPostJFXPing)(LPCTSTR szInstallMethod, 
        LPCTSTR szPingName, LPCTSTR szCurrentJavaFXVersion,
        LPCTSTR szRequestJavaFXVersion, LPCTSTR szCurrentJavaVersion,
        DWORD returnCode, LPCTSTR szErrorFile);

    JNIEXPORT jboolean JNICALL
    Java_com_sun_deploy_config_WinConfig_sendJFXPingImpl(JNIEnv *env,
            jobject sender, jstring regUtilsPath,
            jstring installMethod, jstring pingName,
            jstring currentJavaFXVersion, jstring requestJavaFXVersion,
            jstring currentJavaVersion, jint returnCode, jstring errorFile) {

        const char* szRegUtilsPath = env->GetStringUTFChars(regUtilsPath,
                NULL);
        const char* szInstallMethod = env->GetStringUTFChars(installMethod,
                NULL);
        const char* szPingName = env->GetStringUTFChars(pingName, NULL);
        const char* szCurrentJavaFXVersion;
        if (currentJavaFXVersion != NULL) {
            szCurrentJavaFXVersion = env->GetStringUTFChars(currentJavaFXVersion, NULL);
        } else {
            szCurrentJavaFXVersion = "";
        }
        const char* szRequestJavaFXVersion;
        if (requestJavaFXVersion != NULL) {
            szRequestJavaFXVersion = env->GetStringUTFChars(requestJavaFXVersion, NULL);
        } else {
            szRequestJavaFXVersion = "";
        }
        const char* szCurrentJavaVersion = env->GetStringUTFChars(
                currentJavaVersion, NULL);
        const char* szErrorFile = env->GetStringUTFChars(errorFile, NULL);

        HMODULE hModule = NULL;
        LPFNPostJFXPing lpfnPostJFXPing;
        BOOL ret = FALSE;
        __try {
            hModule = LoadLibrary(szRegUtilsPath);
            if (hModule != NULL) {
                lpfnPostJFXPing = (LPFNPostJFXPing)
                        GetProcAddress(hModule, "PostJFXPing");

                if (lpfnPostJFXPing != NULL) {
                    ret = lpfnPostJFXPing(szInstallMethod, szPingName,
                            szCurrentJavaFXVersion, szRequestJavaFXVersion,
                            szCurrentJavaVersion, returnCode, szErrorFile);
                }
            }
        } __finally {
            if (hModule != NULL) {
                FreeLibrary(hModule);
            }
        }
 
        env->ReleaseStringUTFChars(regUtilsPath, szRegUtilsPath);
        env->ReleaseStringUTFChars(installMethod, szInstallMethod);
        env->ReleaseStringUTFChars(pingName, szPingName);
        env->ReleaseStringUTFChars(currentJavaFXVersion, szCurrentJavaFXVersion);
        env->ReleaseStringUTFChars(requestJavaFXVersion, szRequestJavaFXVersion);
        env->ReleaseStringUTFChars(currentJavaVersion, szCurrentJavaVersion);
        env->ReleaseStringUTFChars(errorFile, szErrorFile);

        return (ret == TRUE) ? JNI_TRUE : JNI_FALSE;
    }

/**
 * Returns the windows system directory
 */
JNIEXPORT jstring JNICALL Java_com_sun_deploy_config_WinConfig_getSystemExecutableHomeImpl
  (JNIEnv *env, jclass)
{
    char path[MAX_PATH];
    UINT n = 0;
    if (IsPlatformWindowsNT()) {
      n = GetSystemDirectory(path, MAX_PATH);
    } else {
      n = GetWindowsDirectory(path, MAX_PATH);
    }

    if ((n == 0) || (n >= MAX_PATH)) {
        return NULL;
    }
    else {
        return env->NewStringUTF(path);
    }
}

/**
 * Returns the value returned by the Win32 GetWindowsDirectory() function
 * or NULL.  On NT this is typically "C:\WINNT".
 */
JNIEXPORT jstring JNICALL Java_com_sun_deploy_config_WinConfig_getPlatformSystemHomeImpl
  (JNIEnv *env, jclass)
{
    char path[MAX_PATH];
    UINT n = GetSystemWindowsDirectory(path, MAX_PATH);
    if ((n == 0) || (n >= MAX_PATH)) {
        return NULL;
    }
    else {
        return env->NewStringUTF(path);
    }
}

/**
 * Returns the value returned by the Win32 SHGetFolderPath() function
 * or NULL.  On NT this is typically
 * "C:\WINNT\Profiles\<username>\Application Data".
 */
JNIEXPORT jbyteArray JNICALL Java_com_sun_deploy_config_WinConfig_getPlatformUserHomeImpl
  (JNIEnv *env, jclass)
{
    HMODULE hModule = NULL;
    static char userHome[MAX_PATH];
    LPTSTR lpszUserInfo;
    LPFNSHGetFolderPath lpfnSHGetFolderPath;
    LPFNSHGetSpecialFolderPath lpfnSHGetSpecialFolderPath;
    jbyteArray buserHome;
    int len = 0;

    lpszUserInfo = userHome;

    if (IsPlatformWindowsVista()) {

        GetAppDataLocalLowPath(lpszUserInfo); 
       
    } else { 

        __try {
            hModule = LoadLibrary("shfolder.dll");
            // use SHGetFolderPath if avaliable
            if (hModule != NULL) {
                // SHGetFolderPath can work everywhere SHFOLDER is installed.
                lpfnSHGetFolderPath = (LPFNSHGetFolderPath)
				GetProcAddress(hModule, "SHGetFolderPathA");

                if (lpfnSHGetFolderPath != NULL) {
                    lpfnSHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, lpszUserInfo);
                }
            } else {
                hModule = LoadLibrary("shell32.dll");
                // SHGetSpecialFolderPath may also work
                lpfnSHGetSpecialFolderPath = (LPFNSHGetSpecialFolderPath)
                    GetProcAddress(hModule, "SHGetSpecialFolderPathA");

                if (lpfnSHGetSpecialFolderPath != NULL) {
                    lpfnSHGetSpecialFolderPath(NULL, lpszUserInfo,
					   CSIDL_APPDATA, TRUE);
                }
            }
        } __finally {
            if (hModule != NULL)
            FreeLibrary(hModule);
        }
    }
    len = strlen(userHome);

    buserHome = env->NewByteArray(len);

    env->SetByteArrayRegion(buserHome, 0, len, (jbyte *)userHome);

    return buserHome;
}

typedef void (*LPFNRedirectAllStaticVersionKeys)();

JNIEXPORT void JNICALL Java_com_sun_deploy_config_WinConfig_notifyJREInstalled
  (JNIEnv *env, jclass, jstring jreBinPath)
{
    const char* szJreBinPath = env->GetStringUTFChars(jreBinPath, (jboolean*)0);
    char szSSVLib[1024];
    HMODULE hModule = NULL;

    // compose ssv.dll full name
    memset(szSSVLib, 0, 1024);
    strcpy(szSSVLib, szJreBinPath);
    strcat(szSSVLib, "ssv.dll");

    __try {
        // load the ssv.dll
        hModule = LoadLibrary(szSSVLib);

        if (hModule != NULL) {
	    // get pointer to function RedirectAllStaticVersionKeys;
	    LPFNRedirectAllStaticVersionKeys lpfnRedirectAllStaticVersionKeys = (LPFNRedirectAllStaticVersionKeys) 
				GetProcAddress(hModule, "RedirectAllStaticVersionKeys");
	    if (lpfnRedirectAllStaticVersionKeys != NULL) {
	        (lpfnRedirectAllStaticVersionKeys)();
	    }
        }
    } __finally {
	if (hModule != NULL)
	    FreeLibrary(hModule);
    }

    if (szJreBinPath)
	env->ReleaseStringUTFChars(jreBinPath, szJreBinPath);
}

JNIEXPORT jint JNICALL
    Java_com_sun_deploy_config_WinConfig_isNativeModalDialogUp(
        JNIEnv *env, jclass)
{
    // get top level window
    HWND hWnd = GetForegroundWindow();
    if (hWnd == NULL)
        return JNI_FALSE;

    // get window style
    LONG lExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
    LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);

    // check modality
    if (!((lExStyle & WS_EX_DLGMODALFRAME) == WS_EX_DLGMODALFRAME) &&
        !((lStyle & DS_MODALFRAME) == DS_MODALFRAME) ) 
            return JNI_FALSE;

    // get pid corresponding to the top window
    DWORD dWndPid;
    DWORD dTid = GetWindowThreadProcessId(hWnd, &dWndPid);

    // return true if the current process id is the same as the window process id
    return (GetCurrentProcessId() == dWndPid) ? JNI_TRUE : JNI_FALSE;

}

} /* extern "C" */

/**
 * following code was from shortcutInstaller.cpp:
 *
 */

// This exposes some simple functions for adding shortcuts on the desktop
// and start menu.
// In Windows shortcuts are manipulated through the COM interfaces
// IShellLink and IPersistFile. In the end, they stored as binary files.
// The locations for these files can be found in the registry under:
// [HKEY_CURRENT_USER]\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders
// and the value for key 'Desktop' giving the path to desktop shortcuts,
// and the value for key 'Start Menu' giving the path to the users start menu
// shortcuts.
// It should be noted that IPersistFile does not expose methods for
// removing the link, instead the usual functions for remove a file are
// used.

char* javawsWideCharToMBCS(const unsigned short* uString) {
  char * mbcs = (char*)malloc(sizeof(char)*MAX_PATH);
  ZeroMemory(mbcs, sizeof(char)*MAX_PATH);
  WideCharToMultiByte(CP_ACP, 0, uString, -1, mbcs, MAX_PATH, NULL, NULL);
  return mbcs;
}

// type indicates the type of shortcut and should be either:
//   0 for a desktop shortcut
//   1 for a 'start menu' shortcut
// Returns 0 on success, else failure

int installShortcut(const WORD *shortcutPath, const unsigned short *shortcutNameU,
                    const unsigned short *descriptionU, const unsigned short *pathU,
                    const unsigned short *argumentsU, const unsigned short *workingDirectoryU,
                    const unsigned short *iconPathU) {
    char *shortcutName = javawsWideCharToMBCS(shortcutNameU);
    char *description = javawsWideCharToMBCS(descriptionU);
    char *path = javawsWideCharToMBCS(pathU);
    char *arguments = javawsWideCharToMBCS(argumentsU);
    char *workingDirectory = javawsWideCharToMBCS(workingDirectoryU);
    char *iconPath = javawsWideCharToMBCS(iconPathU);

    // Initialize COM, stash the result to know if we need to call
    // CoUnintialize
    HRESULT comStart = CoInitialize(NULL);

    HRESULT tempResult;
    IShellLink *shell;

    int retValue = 0;

    // Find IShellLink interface.
    tempResult = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IShellLink, (void **)&shell);

    if (SUCCEEDED(tempResult)) {
        IPersistFile* persistFile;

        // Query IShellLink for the IPersistFile interface for
        // saving the shell link in persistent storage.
        tempResult = shell->QueryInterface(IID_IPersistFile,
                                           (void **)&persistFile);
        if (SUCCEEDED(tempResult)) {

            // Set the path to the shell link target.
            tempResult = shell->SetPath(path);

            if (!SUCCEEDED(tempResult)) {
                // Couldn't set the path
                retValue = -2;
            }

            // Set the description of the shell link.
            // fix for 4499382
            // make sure description length is less than MAX_PATH
            // else truncate the string before setting description
            if (retValue == 0 && description != NULL &&
                strlen(description) < MAX_PATH &&
                !SUCCEEDED(shell->SetDescription(description))) {
              retValue = -3;
            } else {
              char *desc = (char*)malloc(sizeof(char)*MAX_PATH);
              desc = strncpy(desc, description, MAX_PATH - 1);
              if (!SUCCEEDED(shell->SetDescription(desc))) {
                retValue = -3;
              }
            }

            // Set the arguments
            if (retValue == 0 && arguments != NULL &&
                !(SUCCEEDED(shell->SetArguments(arguments)))) {
                retValue = -4;
            }

            // Working directory
            if (retValue == 0 && workingDirectory != NULL &&
                !(SUCCEEDED(shell->SetWorkingDirectory(workingDirectory)))) {
                retValue = -5;
            }

            // Sets the icon location, default to an icon index of 0.
            if (retValue == 0 && iconPath != NULL &&
                !(SUCCEEDED(shell->SetIconLocation(iconPath, 0)))) {
                retValue = -6;
            }
            // PENDING: if iconPath == null, should install a link to
            // the default icon!

            // Defaults to a normal window.
            if (retValue == 0) {
                shell->SetShowCmd(SW_NORMAL);

                // Save the link via the IPersistFile::Save method.
                if (!SUCCEEDED(persistFile->Save(shortcutPath, TRUE))) {
                    retValue = -7;
                }
            }
            // Release pointer to IPersistFile.
            persistFile->Release();
        }
        else {
            // No persist file
            retValue = -8;
        }
        // Release pointer to IShellLink.
        shell->Release();
    }
    else {
        // No shell!
        retValue = -9;
    }
    if (comStart == S_OK) {
        CoUninitialize();
    }
    free(shortcutName);
    free(description);
    free(path);
    free(arguments);
    free(workingDirectory);
    free(iconPath);
    return retValue;
}

extern "C" {
    //
    // Install
    //

    JNIEXPORT jint JNICALL Java_com_sun_deploy_config_WinConfig_installShortcut(
		JNIEnv *env, jobject wHandler, jstring pathS, jstring nameS,
		jstring descriptionS, jstring appPathS, jstring argsS,
		jstring directoryS, jstring iconPathS) {
        const WORD *path = (pathS != NULL) ? env->GetStringChars
                        (pathS, NULL) : NULL;
        const unsigned short *name = (nameS != NULL) ? env->GetStringChars
                        (nameS, NULL) : NULL;
        const unsigned short *description = (descriptionS != NULL) ?
                      env->GetStringChars(descriptionS, 0) : NULL;
        const unsigned short *appPath = (appPathS != NULL) ? env->GetStringChars
                          (appPathS, 0) : NULL;
        const unsigned short *args = (argsS != NULL) ? env->GetStringChars
                       (argsS, 0) : NULL;
        const unsigned short *directory = (directoryS != NULL) ? env->GetStringChars
                            (directoryS, 0) : NULL;
        const unsigned short *iconPath = (iconPathS != NULL) ? env->GetStringChars
                           (iconPathS, 0) : NULL;

        jint iShortcut = -1;

        // Find the valid name
        if (name != NULL && path != NULL) {
            iShortcut = installShortcut(path, name, description,
                                        appPath, args, directory, iconPath);

            // Should log error somehow.
        }

        if (path != NULL) {
            env->ReleaseStringChars(pathS, path);
        }
        if (name != NULL) {
            env->ReleaseStringChars(nameS, name);
        }
        if (description != NULL) {
            env->ReleaseStringChars(descriptionS, description);
        }
        if (appPath != NULL) {
            env->ReleaseStringChars(appPathS, appPath);
        }
        if (args != NULL) {
            env->ReleaseStringChars(argsS, args);
        }
        if (directory != NULL) {
            env->ReleaseStringChars(directoryS, directory);
        }
        if (iconPath != NULL) {
            env->ReleaseStringChars(iconPathS, iconPath);
        }
        return iShortcut;
    }
}




/**
 *
 * Following code is from browserSupport.cpp
 *
 *
 *
 *
 */

#define TEMP_FILE_NAME "jawshtml.html"

/* Reference to browserExecutable */
static char* browserExecutable = NULL;

void inititalizeBrowser() {
  int begin, end;
  char dummy[MAX_PATH];
  char resultPath[MAX_PATH];
  char tempPath[MAX_PATH];
  HINSTANCE retval;

  /* Already inititalized? */
  if (browserExecutable != NULL) return;

  /* First create a known temp. HTML file */
  if (FAILED(GetTempPath(MAX_PATH, tempPath))) return;
  _tcscat(tempPath, TEMP_FILE_NAME);
  FILE* fp = fopen(tempPath, "w");
  fputs("<HTML></HTML>", fp);
  fclose(fp);

  dummy[0] = resultPath[0] = '\0';
  retval = FindExecutable(tempPath, dummy, resultPath);

  /* Trim result */
  if ((int)retval > 32) {
     begin = 0;
     end = strlen(resultPath);
     while(resultPath[begin] != 0 && resultPath[begin] == ' ') begin++;
     while(end > begin && resultPath[end-1] == ' ') end--;
     if (end > begin) {
       resultPath[end] = '\0';
       browserExecutable = strdup(resultPath + begin);
     }
  }

  /* Remove tempoary file */
  remove(tempPath);
}


typedef WINADVAPI BOOL (WINAPI *LPFNAllocateAndInitializeSid)(
    PSID_IDENTIFIER_AUTHORITY pIdentifierAuthority,
    BYTE nSubAuthorityCount,
    DWORD nSubAuthority0,
    DWORD nSubAuthority1,
    DWORD nSubAuthority2,
    DWORD nSubAuthority3,
    DWORD nSubAuthority4,
    DWORD nSubAuthority5,
    DWORD nSubAuthority6,
    DWORD nSubAuthority7,
    PSID *pSid
    );

typedef WINADVAPI BOOL (WINAPI *LPFNFreeSid)(
    PSID pSid
    );

typedef WINADVAPI BOOL (WINAPI *LPFNOpenProcessToken)(
    HANDLE ProcessHandle,
    DWORD DesiredAccess,
    PHANDLE TokenHandle
    );

typedef WINADVAPI BOOL (WINAPI *LPFNGetTokenInformation)(
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength
    );

typedef WINADVAPI BOOL (WINAPI *LPFNEqualSid)(
    PSID pSid1,
    PSID pSid2
    );


extern "C" {
    JNIEXPORT jint JNICALL
	Java_com_sun_deploy_config_WinConfig_canDownloadJRE(
            JNIEnv *env, jclass)
    {
         OSVERSIONINFOEX osvi;
	 BOOL bOsVersionInfoEx;

	 // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	 // If that fails, try using the OSVERSIONINFO structure.

	 ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	 osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	 if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	 {
	   // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
	   osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	   if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
	     return JNI_FALSE;;
	 }

	 // return true if we are not running on win2k, XP or NT
	 if (!((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
	       (osvi.dwMajorVersion >= 4)))
	 {
	   return JNI_TRUE;
	 }

   

        HMODULE hModule = ::LoadLibrary("advapi32.dll");

        LPFNAllocateAndInitializeSid lpfnAllocateAndInitializeSid = 
		(LPFNAllocateAndInitializeSid)
                ::GetProcAddress(hModule, "AllocateAndInitializeSid");

        LPFNOpenProcessToken lpfnOpenProcessToken = (LPFNOpenProcessToken)
		::GetProcAddress(hModule, "OpenProcessToken");
    
        LPFNGetTokenInformation lpfnGetTokenInformation = 
		(LPFNGetTokenInformation)
                ::GetProcAddress(hModule, "GetTokenInformation");
    
        LPFNEqualSid lpfnEqualSid = (LPFNEqualSid)
		::GetProcAddress(hModule, "EqualSid");

        LPFNFreeSid lpfnFreeSid = (LPFNFreeSid)
		::GetProcAddress(hModule, "FreeSid");

        BOOL bIsAdmin = FALSE;
        SID_IDENTIFIER_AUTHORITY ntauth = SECURITY_NT_AUTHORITY;
        void* psidAdmin = 0;
        lpfnAllocateAndInitializeSid( &ntauth, 2, SECURITY_BUILTIN_DOMAIN_RID, 
		DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &psidAdmin );
   
        HANDLE htok = 0;
        lpfnOpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &htok );
        DWORD cb = 0;
        lpfnGetTokenInformation( htok, TokenGroups, 0, 0, &cb );
        TOKEN_GROUPS* ptg = (TOKEN_GROUPS*) malloc(cb);
        lpfnGetTokenInformation( htok, TokenGroups, ptg, cb, &cb );

        for ( DWORD i = 0; i < ptg->GroupCount; ++i ) {
	   if ( lpfnEqualSid( psidAdmin, ptg->Groups[i].Sid ) ) {
		 break;
	    }
	}

        bIsAdmin = i != ptg->GroupCount;
        free( ptg );
        ::CloseHandle( htok );
        lpfnFreeSid( psidAdmin );

	return (bIsAdmin) ? JNI_TRUE : JNI_FALSE;
    }

} // End extern "C"


extern "C" {
    JNIEXPORT jint JNICALL
        Java_com_sun_deploy_config_WinConfig_showDocument(
            JNIEnv *env, jclass, jstring url)
    {
        char dummy[MAX_PATH];
        HINSTANCE retval;
        dummy[0] = '\0';

        const char *urlStr = env->GetStringUTFChars(url, NULL);

        inititalizeBrowser();
        if (browserExecutable == NULL) return JNI_FALSE;

        retval = ShellExecute(
                 NULL, "open", browserExecutable, urlStr, dummy, SW_SHOWNORMAL);

        return ((int)retval > 32) ? JNI_TRUE : JNI_FALSE;
    }

} // End extern "C"

extern "C" {
    JNIEXPORT jstring JNICALL
        Java_com_sun_deploy_config_WinConfig_getBrowserPath(JNIEnv *env,
            jclass)
    {
        inititalizeBrowser();
        if (browserExecutable == NULL) return JNI_FALSE;
        return env->NewStringUTF(browserExecutable);
    }

} // End extern "C"

extern "C" {
    JNIEXPORT jint JNICALL 
        Java_com_sun_deploy_config_WinConfig_getDesktopIconSize( 
            JNIEnv *env, jclass) {
        HKEY key;
        DWORD retval = -1;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, 
                             DESKTOP_METRICS_KEY,
                             NULL,
                             KEY_READ, 
                             &key)) {

            char sizeString[8];
            DWORD dwValueSize = 8;
            DWORD dwType = REG_SZ;
            if (RegQueryValueEx(key, ICON_SIZE_KEY, NULL,
                &dwType, (LPBYTE)sizeString, &dwValueSize) == 
                ERROR_SUCCESS) {
                if (dwValueSize < 8) sizeString[dwValueSize] = '\0';
                retval = atoi(sizeString);
            }  else {
                retval = -1;
            }
            RegCloseKey(key);
        } 
        return (jint)retval;
    }
}

extern "C" {
    //
    // Add a Javawebstart Application entry to the Add/Remove programs utility
    //
    JNIEXPORT void JNICALL
        Java_com_sun_deploy_config_WinConfig_addRemoveProgramsAdd(JNIEnv  * env,
                                                                  jobject   wHandler,
                                                                  jstring   jnlpURL,
                                                                  jstring   title,
                                                                  jstring   icon,
                                                                  jstring   vendor,
                                                                  jstring   description,
                                                                  jstring   homepage,
                                                                  jboolean  sysCache) {
        // make sure the minimum required parameters are here
        if ((jnlpURL != NULL) && (title != NULL)) {
            // get the path to the default JavaWS
            char szSystemDir[1024];
            char szJavaWSHome[1024];

            if (IsPlatformWindowsNT() == TRUE) {
                GetSystemDirectory(szSystemDir, 1024);
            }
            else {
                GetWindowsDirectory(szSystemDir, 1024);
            }

            wsprintf(szJavaWSHome, "%s\\javaws.exe", szSystemDir);

            // convert the url and title to native strings
            const jchar * pJnlpUrl      = env->GetStringChars(jnlpURL, NULL);
            const jchar * pTitle        = env->GetStringChars(title, NULL);
            const char  * szJnlpUrl     = javawsWideCharToMBCS(pJnlpUrl);
            const char  * szDisplayName = javawsWideCharToMBCS(pTitle);
            const char  * szDisplayIcon = szJavaWSHome;

            if (icon != NULL) {
                const jchar * pIcon = env->GetStringChars(icon, NULL);

                szDisplayIcon = javawsWideCharToMBCS(pIcon);
            }
            // no else required; use default icon for JavaWS

            // open the application specific key
            CRegKey parentKey;

            if (parentKey.Create((sysCache == JNI_TRUE) ?
		    HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, 
		    UNINSTALL_KEY) == ERROR_SUCCESS) {
                CRegKey appKey;

                if (appKey.Create(parentKey, szDisplayName) == ERROR_SUCCESS) {
                    char         szUninstallString[1024];
                    const char * szFormat = "%s -uninstall -prompt %s";

                    if(sysCache == JNI_TRUE) {
                        szFormat = "%s -system -uninstall -prompt %s";
                    }
                    // no else required; not in system cache
                    wsprintf(szUninstallString,
                             szFormat,
                             szJavaWSHome, szJnlpUrl);

                    LONG nStatus = ERROR_SUCCESS;

                    // Set the required keys so the app is displayed by the
                    // control panel.
                    nStatus |= appKey.SetValue(szUninstallString, "UninstallString");
                    nStatus |= appKey.SetValue(szDisplayName, "DisplayName");
                    nStatus |= appKey.SetValue(szDisplayIcon, "DisplayIcon");

                    // The next 2 keys control the butons that are displayed in
                    // the control panel when the app is selected
                    nStatus |= appKey.SetValue(1, "NoModify");
                    nStatus |= appKey.SetValue(1, "NoRepair");

                    if (vendor != NULL) {
                        // add the vendor (i.e. Publisher)
                        const jchar * pVendor     =
                            env->GetStringChars(vendor, NULL);
                        const char  * szPublisher = NULL;

                        szPublisher = javawsWideCharToMBCS(pVendor);

                        nStatus |= appKey.SetValue(szPublisher, "Publisher");
                    }
                    else {
                        // Delete any previously set Publisher value
                        appKey.DeleteValue("Publisher");
                    }

                    if (description != NULL) {
                        // add the description (i.e. Comments)
                        const jchar * pDesc     = env->GetStringChars(description, NULL);
                        const char  * szComment = NULL;

                        szComment = javawsWideCharToMBCS(pDesc);

                        nStatus |= appKey.SetValue(szComment, "Comments");
                    }
                    else {
                        // Delete any previously set Comments value
                        appKey.DeleteValue("Comments");
                    }

                    if (homepage != NULL) {
                        // add the homepage (i.e. URLInfoAbout)
                        const jchar * pHomepage      = env->GetStringChars(homepage, NULL);
                        const char  * szURLAboutInfo = NULL;

                        szURLAboutInfo = javawsWideCharToMBCS(pHomepage);

                        nStatus |= appKey.SetValue(szURLAboutInfo, "URLInfoAbout");
                    }
                    else {
                        // Delete any previously set URLInfoAbout value
                        appKey.DeleteValue("URLInfoAbout");
                    }

                    appKey.Close();

                    // if anything failed, delete the key and punt
                    if (nStatus != ERROR_SUCCESS) {
                        parentKey.RecurseDeleteKey(szDisplayName);
                    }
                    // no else required; success

                    parentKey.Close();
                }
                // no else required; failed to access registry
            }
            // no else required; failed to access registry
        }
        // no else required; missing required parameters
    }
} // End extern "C"

extern "C" {
    //
    // Remove a Javawebstart Application entry from Add/Remove programs utility
    //
    JNIEXPORT void JNICALL
        Java_com_sun_deploy_config_WinConfig_addRemoveProgramsRemove(
			JNIEnv * env, jobject  wHandler, 
			jstring  title, jboolean  sysCache) {
        // make sure the minimum required parameters are here
        if (title != NULL) {
            // convert the title String object to a native string
            const jchar * pTitle        = env->GetStringChars(title, NULL);
            const char  * szDisplayName = javawsWideCharToMBCS(pTitle);

            // open the application specific key
            CRegKey parentKey;

            if (parentKey.Open((sysCache == JNI_TRUE) ?
                    HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER, 
		    UNINSTALL_KEY, KEY_READ | KEY_WRITE) == ERROR_SUCCESS) {
                CRegKey appKey;

                // Note: It's easier to create it, than it is to care if it
                //       already exists.  This code wouldn't be getting called
                //       if the JNLP app wasn't being removed, so who cares
                //       if something else has already deleted this key.
                if (appKey.Create(parentKey, szDisplayName) == ERROR_SUCCESS) {
                    appKey.Close();
                    parentKey.RecurseDeleteKey(szDisplayName);
                }
                // no else required; either can't open, or can't create the key

                parentKey.Close();
            }
            // no else required; failed to access registry
        }
        // no else required; missing required parameters
    }

    JNIEXPORT jint JNICALL
        Java_com_sun_deploy_config_WinConfig_isPlatformWindowsVista (
	    JNIEnv * env, jobject  wHandler ) {

        if (IsPlatformWindowsVista()) {
            return JNI_TRUE;
        }
        return JNI_FALSE;
    }

    JNIEXPORT jint JNICALL
        Java_com_sun_deploy_config_WinConfig_isBrowserFireFox (
            JNIEnv * env, jobject  wHandler ) {

        WCHAR buffer[MAX_PATH];
        LPSTR lpBuffer = (LPSTR)buffer;
        char *pch;

        // Get the name of current process
        if ( GetModuleFileName(NULL, lpBuffer, MAX_PATH) != 0) {;
           pch = strtok(lpBuffer, "\\");
           while (pch != NULL) {
              if ( stricmp(pch, "firefox.exe") == 0 ) {
                 return JNI_TRUE;
              }
              pch = strtok(NULL, "\\");
           }
        }

        return JNI_FALSE;
    }

    JNIEXPORT jstring JNICALL 
	Java_com_sun_deploy_config_WinConfig_getBrowserHomePathImpl(JNIEnv *env, jclass) {  
	WCHAR mozDir[MAX_PATH];
	LPSTR lpMozDir = (LPSTR)mozDir;
	char *p;
	DWORD dwSize;
	
	// Get the path to the running process
	dwSize = GetModuleFileName(NULL, lpMozDir, sizeof(mozDir));
	
	if ((dwSize == 0) || (dwSize >= MAX_PATH)) {
	    return NULL;
	} else {
	    p = strrchr(lpMozDir, '\\');
	    if (p) {
		*p = '\0';
	    }
	    return env->NewStringUTF(lpMozDir);
	}
    }

    JNIEXPORT jint JNICALL 
        Java_com_sun_deploy_config_WinConfig_getPlatformMaxCommandLineLength (
	    JNIEnv * env, jobject  wHandler ) {

      if(IsPlatformWindowsXPorLater()) {
        return (jint)8191;
      }
      return (jint)2047;
    }

    JNIEXPORT jlong JNICALL 
        Java_com_sun_deploy_config_WinConfig_getSysTickCount (
        JNIEnv *env, jobject objHandle) {

          return (jlong) GetTickCount();
    }



    JNIEXPORT jlong JNICALL 
        Java_com_sun_deploy_config_UnixConfig_getPlatformPID (
        JNIEnv *env, jobject objHandle) {

      if(IsPlatformWindowsNT()) {
          return (jlong) GetCurrentProcessId();
      } else {
          return -1; // not supported 
      }
    }

} // End extern "C"
