/*
 * @(#)main.cpp	1.3 03/01/23
 *
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
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
#include "shlobj.h"

#include "UpdateUtils.h"
#include "UpdateConf.hpp"
#include "UserProfile.h"
#include "XMLParser.h"
#include "Locale.h"
#include "WrapperUtils.h"
#include "PluginVersion.h"

#define BUFFER_SIZE 1024
#define ERROR_PROXY_AUTH_REQ HRESULT_FROM_WIN32(0x210)
#define REG_WI_LOCATION_KEY "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Installer"
#define REG_JDK_MAIN_KEY "SOFTWARE\\JavaSoft\\Java Development Kit\\" VERSION
#define REG_JRE_MAIN_KEY "SOFTWARE\\JavaSoft\\Java Runtime Environment\\" VERSION
//define this in header file


#define  JDK_CABEXTRACT_WAIT_EVENT "SunJDKCabExtractWaitEvent"

//Call this in wrapper.cpp main function.
//If return value is NULL, then it failed to create event.
//You might want to handle the error and exit
HANDLE GetExtractWaitEvent()
{
  return(::CreateEvent(NULL, TRUE, FALSE, JDK_CABEXTRACT_WAIT_EVENT));
}

//Call this from extraction function, after done with the extraction
BOOL SignalExtractWaitEvent()
{
  HANDLE handle = GetExtractWaitEvent();
  if (handle == NULL) return FALSE;
    SetEvent(handle);
  CloseHandle(handle);
  return TRUE;
}

int ExecCommand(LPSTR lpszCommand) 
{
	PROCESS_INFORMATION pf;
	STARTUPINFO sf;
	DWORD dwExitCode = 1;

	::ZeroMemory(&pf, sizeof(pf));
	::ZeroMemory(&sf, sizeof(sf));

	if(::CreateProcess(NULL, lpszCommand, NULL, NULL, FALSE, 0, NULL, NULL, &sf, &pf)) {
		::WaitForSingleObject(pf.hProcess, INFINITE);
		GetExitCodeProcess(pf.hProcess, &dwExitCode);
		::CloseHandle(pf.hProcess);
	}

	return dwExitCode;
}

void GetMSICABFileNames(LPTSTR lpszLocalFileName, LPTSTR lpszCabName, BOOL b64bit)
{
    GetUserShellFolder(lpszLocalFileName);
    wsprintf(lpszLocalFileName, "%s\\Sun\\", lpszLocalFileName);
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%sJava\\", lpszLocalFileName);
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%s%s%s%s", lpszLocalFileName, BUNDLE, VERSION, (b64bit) ? "_x64" : "");
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%s\\%s.cab", lpszLocalFileName, lpszCabName);
}

//
// Find JDK installation directory
//
BOOL FindJDKHome(LPTSTR lpszJdkHome, DWORD dwCount) {
    
    CRegKey jreRegKey;

    if (jreRegKey.Open(HKEY_LOCAL_MACHINE, REG_JDK_MAIN_KEY, KEY_READ) != ERROR_SUCCESS) {
        return FALSE;
    }

    if (jreRegKey.QueryValue(lpszJdkHome, "JavaHome", &dwCount) != ERROR_SUCCESS) {
        return FALSE;
    }      

    return TRUE;
}


void GetCobundleFileName(LPTSTR lpszLocalFileName, LPTSTR lpszCobundleName, BOOL b64bit) {
    GetUserShellFolder(lpszLocalFileName);
    wsprintf(lpszLocalFileName, "%s\\Sun\\Java\\%s%s%s\\%s", lpszLocalFileName, 
	     BUNDLE, VERSION, (b64bit) ? "_x64" : "", lpszCobundleName);
}

DWORD WINAPI ExtractMSICABFiles(void *data)
{

  HINSTANCE hInstance = (HINSTANCE) data;

  TCHAR szCabName[MAX_PATH], szCabNamePath[MAX_PATH];

    BOOL b64bit = FALSE;

#ifdef ARCH
    if (lstrcmpi(ARCH, "amd64") == 0) {
        b64bit = TRUE;
    }
#endif

  wsprintf(szCabName, "sd%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);
  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB6), 
				"JAVA_CAB6", szCabNamePath);

  wsprintf(szCabName, "ss%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);
  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB9), 
				"JAVA_CAB9", szCabNamePath);

  wsprintf(szCabName, "st%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);

  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB10), 
				"JAVA_CAB10", szCabNamePath);

  wsprintf(szCabName, "sj%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);
  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB7), 
				"JAVA_CAB7", szCabNamePath);

  wsprintf(szCabName, "sp%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);
  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB8), 
				"JAVA_CAB8", szCabNamePath);

  wsprintf(szCabName, "sz%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);
  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB11), 
				"JAVA_CAB11", szCabNamePath);

  wsprintf(szCabName, "sb%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);
  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB12), 
				"JAVA_CAB12", szCabNamePath);

  wsprintf(szCabName, "sr%s%s%s%s0", PLUGIN_MAJOR_VERSION, PLUGIN_MINOR_VERSION, PLUGIN_MICRO_VERSION, PLUGIN_UPDATE_VERSION);
  GetMSICABFileNames(szCabNamePath, szCabName, b64bit);
  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERCAB13), 
				"JAVA_CAB13", szCabNamePath);

#ifdef COBUNDLE

  GetCobundleFileName(szCabNamePath, "javafx-sdk.msi", b64bit);

  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_COBUNDLE_MSI), 
				"JAVA_COBUNDLE_MSI", szCabNamePath);
  GetCobundleFileName(szCabNamePath, "disk1.cab", b64bit);

  ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_COBUNDLE_CAB), 
				"JAVA_COBUNDLE_CAB", szCabNamePath);
#endif

  SignalExtractWaitEvent();

  return TRUE;
}


//
// Find if the same version of the jre is installed
//
BOOL IsJREInstalled() {
    
    CRegKey jreRegKey;

    if (jreRegKey.Open(HKEY_LOCAL_MACHINE, REG_JRE_MAIN_KEY, KEY_READ) != ERROR_SUCCESS) {
        return FALSE;
    } 

    return TRUE;
}

TCHAR szWIPath[MAX_PATH];
LANGID gLangid;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
	             LPTSTR     lpCmdLine,
                     int       nCmdShow)
{
    lpCmdLine = GetCommandLine(); 
    gLangid = DetectLocale();
    TCHAR command[BUFFER_SIZE];
    TCHAR szExecutableName[MAX_PATH], szExecutableNameCommand[BUFFER_SIZE];
    command[0] = NULL;
    int iReturn = 1;
    BOOL bSilentInstall = FALSE;
    BOOL b64bit = FALSE;
    TCHAR szPubJreDir[BUFFER_SIZE] = {NULL};    
    TCHAR szJavaDBDir[BUFFER_SIZE] = {NULL};    

    for (int i=1; i< __argc; i++) {

        if (strstr(__argv[i], "/lang=") != NULL) {
	    LANGID lang = (LANGID) atoi(strstr(__argv[i], "=")+1);
  	    if (IsThisValidLang(lang)) gLangid = lang;

        }
        //msi cmdline options are appended to xml file <options>, if usr online
	//take out /v" & then last ending "
        else if(strncmp(__argv[i], "/v\"", 3)==0) {
            lstrcat(command, " ");
            strncat(command, __argv[i]+3, lstrlen(__argv[i])-4);
            lstrcat(command, " ");
        }
        else if(strncmp(__argv[i], "/v", 2)==0) {
            lstrcat(command, " ");
            lstrcat(command, __argv[i]+2);
            lstrcat(command, " ");
        }
	//silent option, add /qn
        else if(lstrcmp(__argv[i], "/s")==0) {
	    bSilentInstall = TRUE;
	}
	//Public Jre installdir for silent install
        else if (strstr(__argv[i], "/INSTALLDIRPUBJRE=") != NULL) {
	    lstrcpy(szPubJreDir, (strstr(__argv[i], "=")+1) );
	}
	//javadb installdir for silent install
        else if (strstr(__argv[i], "/INSTALLDIRJAVADB=") != NULL) {
	    lstrcpy(szJavaDBDir, (strstr(__argv[i], "=")+1) );
	}
        //The argument is either /x
        else {
            lstrcat(command, __argv[i]);
            lstrcat(command, " ");
        }
    }

#ifdef ARCH
     if (lstrcmpi(ARCH, "amd64") == 0) {
	    b64bit = TRUE;
        }
#endif

    TCHAR szMessage[1024]; 
    if (!IsWIVerValid(NULL)){ 
      ::LoadString(hInstance, IDS_ERROR_WI20, szMessage, 1024);
      ::MessageBox(NULL, szMessage, "", MB_OK|MB_ICONERROR);
      return -2;
    }

    GetSingleMSIFileNames(szExecutableName, b64bit);
    if (ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLERMSI), 
				"JAVA_INSTALLER", szExecutableName)) {
 
	if (!GetWIPath(szWIPath, sizeof(szWIPath)))
	    return -3;
	if (bSilentInstall) {
	    if (strstr(command, "/qn") == 0) lstrcat(command, " /qn");
	}
	
	if (gLangid == LANGID_ENGLISH)
	  wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\" %s WRAPPER=1", 
		   szWIPath, szExecutableName, command);
	else
	  wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\" TRANSFORMS=:%d %s WRAPPER=1", 
		   szWIPath, szExecutableName, gLangid, command);
	
	DWORD dwThreadId =  NULL;
	HANDLE handle = GetExtractWaitEvent();
	//create a thread to extract the msi cab files, while the msi launches
	::CreateThread(NULL, 0,
		       (LPTHREAD_START_ROUTINE)ExtractMSICABFiles ,
		       (LPVOID)hInstance, 0, &dwThreadId);

	iReturn = ExecCommand(szExecutableNameCommand);
	//Check if it's the reinstall case. If so, then relaunch the same cmd
	TCHAR buf[512];
	lstrcpy(buf, "");
	GetJavaSoftKey(REG_INSTALLSTATS_STATUS, buf, sizeof(buf));
	if (lstrcmp(buf, "reinstall") == 0) {
            SetInstallStatsKey(NULL);
	    iReturn = ExecCommand(szExecutableNameCommand);
	}

#ifdef COBUNDLE
	TCHAR szCobundleNamePath[MAX_PATH];
	GetCobundleFileName(szCobundleNamePath, "", b64bit);
	//Now launch the cobundle installer

	wsprintf(szExecutableNameCommand, "\"%s\" /i \"%sjavafx-sdk.msi\" ", szWIPath, szCobundleNamePath);

	ExecCommand(szExecutableNameCommand);
#endif

        if (bSilentInstall) {
	    TCHAR szJdkHome[BUFFER_SIZE] = {NULL};

            //Find JDK installer directory
            if (FindJDKHome(szJdkHome, BUFFER_SIZE)) {
	        TCHAR szJreMsiCommand[BUFFER_SIZE] = {NULL};
	        TCHAR szJavaDBMsiCommand[BUFFER_SIZE] = {NULL};
	        TCHAR szJreMsiName[BUFFER_SIZE] = {NULL};
	        TCHAR szJreTrans[BUFFER_SIZE] = {NULL};
	        TCHAR szJavaDBMsiName[BUFFER_SIZE] = {NULL};
	        TCHAR szFilename[BUFFER_SIZE] = {NULL};

                //install public jre silently
	        wsprintf(szJreMsiName, "%s\\jre.msi", szJdkHome);

                if (IsFileExists(szJreMsiName) == TRUE) {
		    if (gLangid == LANGID_ENGLISH) {
                        wsprintf(szJreMsiCommand, "\"%s\" /i \"%s\" /qn ADDLOCAL=ALL SDKSILENT=1", szWIPath, szJreMsiName);
		    }
		    else {
                        wsprintf(szJreTrans, "%s\\jre%d.MST", szJdkHome, gLangid);
	                wsprintf(szJreMsiCommand, "\"%s\" /i \"%s\" TRANSFORMS=\"%s\" /qn ADDLOCAL=ALL SDKSILENT=1", szWIPath, szJreMsiName, szJreTrans);
		    }
	                

                    if (lstrlen(szPubJreDir) != 0) {
                        lstrcat(szJreMsiCommand, " INSTALLDIR=");
                        lstrcat(szJreMsiCommand, szPubJreDir);
		    }                       
  	            ExecCommand(szJreMsiCommand);
                }

                //install javadb silently
   	        wsprintf(szJavaDBMsiName, "%s\\javadb.msi", szJdkHome);

                if (IsFileExists(szJavaDBMsiName) == TRUE) {
                  
	            wsprintf(szJavaDBMsiCommand, "\"%s\" /i \"%s\" /qn", szWIPath, szJavaDBMsiName);
                    if (lstrlen(szJavaDBDir) != 0) {
                        lstrcat(szJavaDBMsiCommand, " INSTALLDIR=");
                        lstrcat(szJavaDBMsiCommand, szJavaDBDir);
		    }                                         
 	            ExecCommand(szJavaDBMsiCommand);
	        }


               //Remove the jre and javadb msi, mst files in silent install. This is needed since the files are removed 
               //with the CustomAction in UI sequence in UI mode. 
	       //But, that CustomAction can't be replaced since the user may install with the msi directly.
 	       ::DeleteFile(szJreMsiName);
	       ::DeleteFile(szJavaDBMsiName);

  	       wsprintf(szFilename, "%s\\%s", szJdkHome, "jre2052.MST");
	       ::DeleteFile(szFilename);

	       wsprintf(szFilename, "%s\\%s", szJdkHome, "jre1041.MST");
	       ::DeleteFile(szFilename);

	    }

	}

    }
  
    RunStuffPostInstall(); 

    //only install AU if jre is installed successfully.  
  if ((iReturn == 0) && (IsJREInstalled())){
    if(!b64bit){
        //Install Auto Update 
        //Extract au.msi
        TCHAR szAppDataPath[MAX_PATH] = {0};
        TCHAR szUserShellFolder[MAX_PATH] = {0};
        TCHAR szCabName[MAX_PATH] = {0};
        TCHAR szExecutableName[MAX_PATH] = {0};
        TCHAR szPath[MAX_PATH] = {0};
        
        GetWIPath(szPath, sizeof(szPath));
        GetUserShellFolder(szUserShellFolder); 
        wsprintf(szAppDataPath,"%s\\Sun\\Java\\AU\\",szUserShellFolder);
        CreateDirectory(szAppDataPath, NULL);
        wsprintf(szExecutableName, "%sau.msi", szAppDataPath);
        wsprintf(szCabName, "%sau.cab", szAppDataPath);
        
        ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_AUINSTALLERCAB),   "JAVA_INSTALLER", szCabName);
        if(ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_AUINSTALLER),   "JAVA_INSTALLER", szExecutableName)){
            
             // Before installing the bundled AU msi, uninstall any older version first
             RemoveAnyOlderAU2X();
            
#ifdef J4B            
             wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\"DISABLE=1 ALLUSERS=1 /qn",szPath, szExecutableName);
#else          
             wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\" ALLUSERS=1 /qn",szPath, szExecutableName);
#endif     
             ExecCommand(szExecutableNameCommand);

             //Register jre to auto-update
             char CommonDir[MAX_PATH] = {0};//store the location of AppData directory

             if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, CommonDir))){
	            wsprintf(szExecutableNameCommand, "%s\\Java\\Java Update\\jaureg.exe -r jre %s", CommonDir, FULLVERSION);
                    ExecCommand(szExecutableNameCommand);
	        }

        }
    } // end of if(!b64 bit)
  }
    return iReturn;
}


