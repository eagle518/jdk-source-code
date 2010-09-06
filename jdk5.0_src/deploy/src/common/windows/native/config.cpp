/*
 * @(#)config.cpp	1.11 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "stdafx.h"
#include <jni.h>
#include <windows.h>
#include <winreg.h>
#include <stdio.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>

BOOL IsPlatformWindowsNT () 
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

extern "C" {

typedef HRESULT (WINAPI *LPFNSHGetFolderPath)(HWND hwndOwner, 
	int nFolder, HANDLE hToken, DWORD dwFlags, LPTSTR pszPath);

typedef BOOL (WINAPI *LPFNSHGetSpecialFolderPath)(HWND hwndOwner, 
	LPTSTR lpszPath, int nFolder, BOOL fCreate);

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
    UINT n = GetWindowsDirectory(path, MAX_PATH);
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

    len = strlen(userHome);

    buserHome = env->NewByteArray(len);

    env->SetByteArrayRegion(buserHome, 0, len, (jbyte *)userHome);

    return buserHome;
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
    //
    // Add a Javawebstart Application entry to the Add/Remove programs utility
    //
 
    JNIEXPORT jint JNICALL Java_com_sun_deploy_config_WinConfig_addRemoveProgramsAdd(JNIEnv *env, jobject wHandler, jstring jnlpFile, jstring appName, jint sysCache)
    {

      char szJavaHome[1024], szUninstallString[1024], szSystemDir[1024];
      
      if (IsPlatformWindowsNT())
	GetSystemDirectory(szSystemDir, 1024);
      else
	GetWindowsDirectory(szSystemDir, 1024);	    
      
      // Get javaws.exe
      wsprintf(szJavaHome, "%s\\javaws.exe", szSystemDir);

      const WORD *jnlpName = (jnlpFile != NULL) ? env->GetStringChars
                        (jnlpFile, NULL) : NULL;
      char *jnlpName2 = javawsWideCharToMBCS(jnlpName);

      const WORD *pathApp = (appName != NULL) ? env->GetStringChars
                        (appName, NULL) : NULL;
      char *szUninstallKeyname = javawsWideCharToMBCS(pathApp);   

      if(sysCache==JNI_TRUE)
	wsprintf(szUninstallString, "%s -system -uninstall %s", szJavaHome, jnlpName2 );
      else
	wsprintf(szUninstallString, "%s -uninstall %s", szJavaHome, jnlpName2 );

      // Open the uninstall key
      CRegKey swKey, msKey, winKey, CVKey, UninstKey, jwsUninstKey;
      
      if (swKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (msKey.Open(swKey, "Microsoft", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (winKey.Open(msKey, "Windows", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (CVKey.Open(winKey, "CurrentVersion", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (UninstKey.Open(CVKey, "Uninstall", KEY_READ | KEY_WRITE) != ERROR_SUCCESS)
	return FALSE;
      
      if (jwsUninstKey.Open(UninstKey, szUninstallKeyname, KEY_READ) != ERROR_SUCCESS)
	{
	  if (jwsUninstKey.Create(UninstKey, szUninstallKeyname) != ERROR_SUCCESS)
	    return FALSE;
	}
      
      if (jwsUninstKey.SetValue(szUninstallKeyname, "DisplayName") != ERROR_SUCCESS)
	return FALSE;
      
      if (jwsUninstKey.SetValue(szUninstallString, "UninstallString") != ERROR_SUCCESS)
	return FALSE;

      if (jwsUninstKey.SetValue(szJavaHome, "DisplayIcon") != ERROR_SUCCESS)
	return FALSE;
      
      return TRUE;

    }
}

extern "C" {
  //
  // Remove a Javawebstart Application entry from Add/Remove programs utility
  //
  JNIEXPORT jint JNICALL  Java_com_sun_deploy_config_WinConfig_addRemoveProgramsRemove(JNIEnv *env, jobject wHandler, jstring appName)
    {

      const WORD *pathApp = (appName != NULL) ? env->GetStringChars
                        (appName, NULL) : NULL;
      char *szUninstallKeyname = javawsWideCharToMBCS(pathApp);   

      // Open the uninstall key
      CRegKey swKey, msKey, winKey, CVKey, UninstKey, jwsUninstKey;
      
      if (swKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (msKey.Open(swKey, "Microsoft", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (winKey.Open(msKey, "Windows", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (CVKey.Open(winKey, "CurrentVersion", KEY_READ) != ERROR_SUCCESS)
	return FALSE;
      
      if (UninstKey.Open(CVKey, "Uninstall", KEY_READ | KEY_WRITE) != ERROR_SUCCESS)
	return FALSE;

      if (UninstKey.RecurseDeleteKey(szUninstallKeyname) != ERROR_SUCCESS)
	return FALSE;
      
      return TRUE;

    }
}
