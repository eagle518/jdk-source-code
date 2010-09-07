/*
 * @(#)jinstall.cpp	1.110 08/06/17 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// JInstall.cpp by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
// Contains WinMain of the JInstall
//
#define STRICT


// disable warning C4786: symbol greater than 255 character, // okay to ignore 
#pragma warning(disable: 4786) 

#include "stdafx.h"
#include <atlhost.h>
#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <objbase.h>
#include <comdef.h>	   // COM smart pointer
#include <urlmon.h>
#include <winbase.h>
#include <shlobj.h>             // for SHBrowseForFolder
#include "resource.h"
#include "ErrorHelpDialog.h"
#include "DownloadDialog.h"
#include "JInstall.h" 
#include "PluginVersion.h"
#include "WrapperUtils.h"
#include "UpdateConf.hpp"
#include "DownloadFile.h"
#include "UserProfile.h"
#include "jinstalllang.h"
#include "InstallerFiles.h"
#include "WelcomeDialog.h"      // for Back from ChangeFolder
#include "ChangeFolder.h"
#include "SetupProgressDialog.h"


#define INSTALLER_VER FULLVERSION

#if defined(_MSC_VER) && (_MSC_VER >= 1310)
#include <WinTrust.h>
#endif

#ifdef PATCH_LIST
typedef struct patch_info {
    char * szName;		// differentiates patches for same version but different builds
    char * szVersion;
    char * szCkSum;
} PATCH_INFO;
#include "PatchStruct.h"
#endif
#include "PatchInPlace.hpp"


//we need this value to cache the msi & mst files in the localappdata folder
//in case users need to install on demand features later
#ifndef CSIDL_LOCAL_APPDATA
#define CSIDL_LOCAL_APPDATA             0x001c        // <user name>\Local Settings\Applicaiton Data (non roaming)
#endif // CSIDL_LOCAL_APPDATA

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

#define SPONSOR_CMDLINE_OPTION "SPWEB="

BOOL bPostStatus = FALSE;
BOOL bSponsorsEnabled = TRUE;  //used for checking if sponsor offer is enabled
BOOL bStatic = FALSE;  //installer argument for "Static Install"
BOOL bKernel = FALSE;  //installer argument for "Kernel Install"
BOOL bOverrideOptions = TRUE; //installer argument to override options passed from auto-update

LANGID gLangid;
TCHAR szMSTCachedDir[1024];
TCHAR szMethod[20];
TCHAR szCountryOverride[3] = {NULL};
TCHAR szPostStatusURL[256];
TCHAR szCountryServletURL[256];
TCHAR szCountry[3];
TCHAR szPreferenceOrder[1024];
TCHAR szVector[20];
TCHAR szVInstallAuxName[BUFFER_SIZE] = {NULL};
TCHAR szVPreloadURL[BUFFER_SIZE] = {NULL};
TCHAR szVRunURL[BUFFER_SIZE]= {NULL};
TCHAR szVMinOS[BUFFER_SIZE]= {NULL};
TCHAR szVMinPSpeed[BUFFER_SIZE]= {NULL};
TCHAR szVMinMemory[BUFFER_SIZE]= {NULL};
TCHAR szVLocale[128]= {NULL};
TCHAR szVGeos[128]= {NULL};

void RemoveSP1Dll(){
        TCHAR sz_gtb_dll[MAX_PATH] = {NULL};
        TCHAR szUserShellFolder[MAX_PATH] = {NULL};
        GetUserShellFolder(szUserShellFolder);
        wsprintf(sz_gtb_dll, "%s\\Sun\\Java\\%s%s\\%s", szUserShellFolder, BUNDLE, VERSION, "gtapi.dll");
        // Delete gtapi.dll
        RemoveFile(sz_gtb_dll);
}

BOOL CALLBACK EnumLanguageCB(LGRPID lgid, LPTSTR, LPTSTR, DWORD, LONG_PTR lPar) {
  BOOL ret = FALSE;
  *(LONG *)lPar = 1;

  for (int i = 0; i < (sizeof(DEFAULT_LANGGRPS) /
                   sizeof(DEFAULT_LANGGRPS[0])); i++) {
    if (lgid == DEFAULT_LANGGRPS[i]) {
      // the language is European
      *(LONG *)lPar = 0;
      ret = TRUE; // continue enumeration if more lgids to check
    }
  }
  return ret;
}

// - On Windows 98/ME/NT (which only allow one language group to be installed)
// Is the system default language a European language? For this purpose,
// GetSystemDefaultLangID can be used, and any language listed under
// ID 1-6 (Western European and Unites States, Central Europe, Baltic, Greek,
// Cyrillic, and Turkic) on
// http://www.microsoft.com/globaldev/win2k/setup/localsupport.asp should
// be treated as European, even though some really aren't. If the
// system default language doesn't fall into this group, the complete
// international JRE needs to be installed.
//
// - On Windows 2000/XP (which allow multiple language groups to be installed)
// Are the above set the only language groups installed? For this purpose,
// EnumSystemLanguageGroups can be used. If any language group other than the
// above set is installed, the complete international JRE needs to be
// installed.

BOOL InstallIntlJRE() {

  OSVERSIONINFO os = { sizeof(os) };
  GetVersionEx(&os);

  if ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5) {
    // it's 2000 or XP
    LPFNEnumSystemLanguageGroups lpfnEnumSystemLanguageGroups = NULL;
    HMODULE hMod = NULL;

    if ((hMod = LoadLibrary("kernel32.dll")) != NULL) {
      if ((lpfnEnumSystemLanguageGroups = (LPFNEnumSystemLanguageGroups)
	   GetProcAddress(hMod, "EnumSystemLanguageGroupsA")) != NULL) {
	LONG bFoundOtherLang = 0;
	if (lpfnEnumSystemLanguageGroups(EnumLanguageCB, LGRPID_INSTALLED,
					 (LONG_PTR)&bFoundOtherLang)) {
	  if (bFoundOtherLang == 1){	  
	    return TRUE;
	  }
	  else{
	    return FALSE;
	  }
	}
      }
      FreeLibrary(hMod);
    }
    // if any of the above functions failed, be conservative and return TRUE
    return TRUE;
  } else {
    // it's 98, ME, or NT
    LANGID syslgid = GetSystemDefaultLangID();
    for (int i = 0; i < (sizeof(DEFAULT_LANGIDS) /
		     sizeof(DEFAULT_LANGIDS[0])); i++) {
      if (PRIMARYLANGID(syslgid) == PRIMARYLANGID(DEFAULT_LANGIDS[i]) &&
	  (SUBLANGID(DEFAULT_LANGIDS[i]) == SUBLANG_NEUTRAL ||
	   SUBLANGID(syslgid) == SUBLANGID(DEFAULT_LANGIDS[i]))) {
	// the system default lang is in the JRE default set:
	// no need to install international components by default.
	return FALSE;
      }
    }
    return TRUE;
  }
  return TRUE;
}


//Check if the OS is  win xp or later 
BOOL IsWinXpOrLater()
{

    OSVERSIONINFO os = { sizeof(os) };
    GetVersionEx(&os);

    if ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && os.dwMajorVersion >= 5 ) {
        if ( os.dwMajorVersion == 5 && os.dwMinorVersion == 0) {
            return FALSE;
        } else {
            return TRUE;
	}  
    } else {
      return FALSE;
    }
  
    return TRUE;
}


BOOL IsEnoughDiskSpace()
{
    char szTempPath[1024];
    char szDrive[10];

    GetUserShellFolder(szTempPath);
    _splitpath(szTempPath, szDrive, NULL, NULL, NULL);
    strcat(szDrive, "\\");

/* Don't need space in temp directory anymore.
  If needed just uncomment this code

    if (!CheckAvailDiskSpace(szDrive, REQUIRED_TEMP_DISKSPACE))
        return FALSE;

    // check if there is enough diskspace in windows temp directory
    GetShortTempPath(1024, szTempPath);
    _splitpath(szTempPath, szDrive, NULL, NULL, NULL);
*/

    return CheckAvailDiskSpace(szDrive, REQUIRED_TEMP_DISKSPACE);
}

void GetSingleMSIFileNames(LPTSTR lpszLocalFileName, LPTSTR lpszLocalDir, LANGID langMST, LPTSTR szDownloadVersion, BOOL b64bit)
{
    GetUserShellFolder(lpszLocalFileName); 
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%s\\Sun\\", lpszLocalFileName);
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalFileName, "%sJava\\", lpszLocalFileName);
    CreateDirectory(lpszLocalFileName, NULL);
    if (b64bit) {
      wsprintf(lpszLocalFileName, "%s%s%s_x64", lpszLocalFileName, BUNDLE, VERSION);
    }
    else {
      wsprintf(lpszLocalFileName, "%s%s%s", lpszLocalFileName, BUNDLE, VERSION);
    }
    CreateDirectory(lpszLocalFileName, NULL);
    wsprintf(lpszLocalDir, "%s", lpszLocalFileName);      
    wsprintf(lpszLocalFileName, "%s\\%s.msi", lpszLocalFileName,  szDownloadVersion);
}


BOOL IsFileDownloaded(LPCTSTR szCachedFile, LPSTR szMessage)
{
    //TODO: Should these error messages be localized? If so, move to rc files

    if (IsFileExists(szCachedFile) == FALSE) {
        if (szMessage != NULL) {
            wsprintf(szMessage, "File %s does not exist.", szCachedFile);
        }
        return FALSE;
    }

#ifdef IGNORE_CHECKSUM
    return TRUE;        // for debugging developer builds
#endif

    //Verify Checksum of the downloaded file
    int j = lstrlen(szCachedFile)-1;
    for (; j> 0; j--) { 
        if ((szCachedFile[j] == '\\') || (szCachedFile[j] == '/')) {
            j++;
            break;
        }
    }
    for (int i=0; i < (sizeof(filelist)/(2*sizeof(char*))); i++) {
        if (lstrcmp(&szCachedFile[j], filelist[2*i]) == 0) {
            if (verifySHA1(szCachedFile, filelist[2*i+1])) {
                return TRUE;
            } else {
                break;
            }
        }
    }

    if (i == (sizeof(filelist)/(2*sizeof(char*)))) {
        return TRUE;
    }

    if (szMessage != NULL) {
        wsprintf(szMessage, "Downloaded File %s is corrupt.", szCachedFile);
    }
    //delete the corrupted file
    ::DeleteFile(szCachedFile);
    return FALSE;
} 

BOOL IsFilesDownloaded(LPCTSTR szSinglemsiCachedFile, LPCTSTR szMSTCachedFile, 
                           LPSTR szMessage)
{
    static BOOL bExist = FALSE;

    if (bExist) {
        return TRUE;
    }
    if ((lstrlen(szSinglemsiCachedFile) > 1)
          && !IsFileDownloaded(szSinglemsiCachedFile, szMessage)) {
        return FALSE;
    }
    if ((lstrlen(szMSTCachedFile) > 1)
          && !IsFileDownloaded(szMSTCachedFile, szMessage)) {
        return FALSE;
    }
    bExist = TRUE; 
    return bExist;
}

void SetInstallStatInfo()
{	
    DWORD dwCount = sizeof(szCountry);

     SetInstallStatsKey(NULL);
     if (bPostStatus) {
         if( !GetJavaUpdateKey(NULL, REG_JUPDATE_COUNTRY, szCountry, &dwCount)){ 
	     GetCountry(szCountryServletURL, szCountry);
         }
         //workaround for an msi issue
         SetJavaUpdateStringKey(NULL, REG_JUPDATE_METHOD, szMethod);
         SetJavaUpdateStringKey(NULL, REG_JUPDATE_POSTSTATUSURL, szPostStatusURL);
     }
}

#ifdef EXTRA_COMP_LIB_NAME
BOOL UncompressDownloadedFile(HINSTANCE hInstance, LPCTSTR szCachedDir, LPCTSTR szCachedFile, LPSTR szMessage)
{

    if (ExtraUnCompression(hInstance, szCachedDir, szCachedFile)) {
        return TRUE;
    }

    if (szMessage != NULL) {
        wsprintf(szMessage, "Uncompression of downloaded file failed.");
    }

    //delete the invalid file
    ::DeleteFile(szCachedFile);
    return FALSE;
} 


BOOL UncompressDownloadedFiles(HINSTANCE hInstance, LPCTSTR szSinglemsiCachedFile, LPCTSTR szMSTCachedFile, 
                       LPCTSTR szCachedDir, LPSTR szMessage)
{
    static BOOL bValid = FALSE;

    if (bValid) {
        return TRUE;
    }
    bValid = TRUE;//Assuming it will work
    if ( (lstrlen(szSinglemsiCachedFile) > 1)
          && !UncompressDownloadedFile(hInstance, szCachedDir, szSinglemsiCachedFile, szMessage) ) {
        bValid=FALSE;
    }
    if ( (lstrlen(szMSTCachedFile) > 1)
          && !UncompressDownloadedFile(hInstance, szCachedDir, szMSTCachedFile, szMessage) ) {
        bValid=FALSE;
    }
    // Unload the library.
    ReleaseExtraCompression(hInstance);
    // Prepare for lzma.dll deletion
    TCHAR sz_LZMA_dll[MAX_PATH] = {NULL};
    TCHAR szUserShellFolder[MAX_PATH] = {NULL};
    BOOL b64bit = FALSE;
#ifdef ARCH
    if (lstrcmpi(ARCH, "amd64") == 0) {
        b64bit = TRUE;
    }
#endif
    GetUserShellFolder(szUserShellFolder); 
    if (b64bit) {
        wsprintf(sz_LZMA_dll, "%s\\Sun\\Java\\%s%s_x64\\%s", szUserShellFolder, BUNDLE, VERSION,"lzma.dll");  
    }
    else {
        wsprintf(sz_LZMA_dll, "%s\\Sun\\Java\\%s%s\\%s", szUserShellFolder, BUNDLE, VERSION,"lzma.dll");
    }
    // Delete lzma.dll
    RemoveFile(sz_LZMA_dll);
    return bValid;
}
#endif

//=--------------------------------------------------------------------------=
//  WinMain -- main procedure for JInstall.
//=--------------------------------------------------------------------------=

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE /*hPrevInstance*/, LPTSTR lpCmdLine, int /*nShowCmd*/)
{

    lpCmdLine = GetCommandLine(); //this line necessary for _ATL_MIN_CRT

    gLangid = DetectLocale();

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init(ObjectMap, hInstance, NULL);

    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    BOOL bRunOnce = FALSE;
    BOOL bKeepAppDataFiles = FALSE;
    BOOL bPrintUsage = FALSE;
    BOOL bSilentInstall = FALSE;
    BOOL bDownloadOnly = FALSE;
    BOOL bUninstallBeta = FALSE;
    BOOL bRunVector = FALSE;
    BOOL bPreviousInstallDir = FALSE;
    
    TCHAR szConfigURL[1024] = {NULL};
    TCHAR szInstallerURL[1024] = {NULL};
    TCHAR szInstallerOptions[512] = {NULL};
    TCHAR szOnlyFirstInstallerCommandOptions[256] = {NULL};
    TCHAR szSinglemsiCachedFile[1024] = {NULL};
    TCHAR szMSTCachedFile[1024] = {NULL};
    TCHAR szMessage[1024] = {NULL};
    TCHAR szTitle[256] = {NULL};
    TCHAR szIsConnectedURL[1024] = {NULL};
    DWORD AuVersionSize = 128;
    TCHAR szProteusOldVersion[128]= {NULL};
    //Default Au return code to AU_NOT_INSTALLED value
    int nAuRet = AU_NOT_INSTALLED;

    szPreferenceOrder[0] = NULL;
    szVector[0] = NULL;
    szMSTCachedDir[0] = NULL;
    szCountryServletURL[0] = NULL;
    szCountry[0] = NULL;
    BOOL b64bit = FALSE;

#ifdef ARCH
     if (lstrcmpi(ARCH, "amd64") == 0) {
	    b64bit = TRUE;
        }
#endif


#ifdef JKERNEL
    lstrcpy(szMethod, INSTALLMETHOD_JKERNEL);
#elif (XPI || XPIRV)
    lstrcpy(szMethod, INSTALLMETHOD_GENERIC_XPI);
#elif (JXPI || JXPIRV)
    lstrcpy(szMethod, INSTALLMETHOD_XPI);
#elif (JCAB || JCABRV)
    lstrcpy(szMethod, INSTALLMETHOD_CAB);
#elif (JCHROME || JCHROMERV)
    lstrcpy(szMethod, INSTALLMETHOD_JCHROME);
#else
    // by default method is manual & other
    lstrcpy(szMethod, INSTALLMETHOD_OTHER);
#endif

    USES_CONVERSION;

    //Get the Old Version if there is one
    TCHAR oldVersion[128] = {NULL};
    if (!GetLatestJreKey(oldVersion, TRUE)) {
        //GetLatestJreKey doesn't change oldVersion if it doesn't exist, so no need to set to NULL again, just logit and move on (SendHeadRequest will set to XX)
        logit("No Old JRE Version found\n");
    }
    
    //Get the old AU version if there is one
    if( ! GetAuVersion(szProteusOldVersion, AuVersionSize) ) {
        //if no old version was found, set old version to XX
        lstrcpy(szProteusOldVersion, "XX");
        logit("No Old AU Version found\n");
    }


    //reset the installstats registry key
    SetInstallStatsKey(NULL);
    lstrcpy(szInstallerOptions, " ");

    //Since Sponsor software is offered, use default xml for all JRE installs
    //use the same xml for jdk also, to take out dependency on
    //GetFileServletURL & make msi-url flexible through xml file.

    if (!GetJavaSoftKey(REG_INSTALLER_XMLURL, szConfigURL, BUFFER_SIZE)) {
        wsprintf(szConfigURL, "%s", DEFAULT_XML);
    }

    lstrcpy(szIsConnectedURL, szConfigURL);


    do
    {
        //PARSE THE COMMAND LINE ARGUMENTS
	for (int i=1; i < __argc; i++) {
	    if (lstrcmpi(__argv[i], "/s") == 0) {
		bSilentInstall = TRUE;
	    }
	    else if (strstr(__argv[i], "/installurl=") != NULL) {
		// Check if the URL is valid
	    	LPWSTR  lpURLStr = T2OLE(strstr(__argv[i], "=")+1);
	    	bPrintUsage = (::IsValidURL(NULL, lpURLStr, NULL) != S_OK);
    		// Obtain XML File Name
		lstrcpy(szConfigURL, (strstr(__argv[i], "=")+1) );
		lstrcpy(szIsConnectedURL, szConfigURL); 
	    }
	    else if (strstr(__argv[i], "/installmethod=") != NULL) {
		lstrcpyn(szMethod, (strstr(__argv[i], "=")+1), sizeof(szMethod)-1);
	    } else if (strstr(__argv[i], "COUNTRYOVERRIDE=") != NULL) {
		lstrcpyn(szCountryOverride, (strstr(__argv[i], "=")+1), sizeof(szCountryOverride));
                SetJavaUpdateStringKey(NULL, REG_JUPDATE_COUNTRY, szCountryOverride);
	    } else if (strstr(__argv[i], "/lang=") != NULL) {
		LANGID lang = (LANGID) atoi(strstr(__argv[i], "=")+1);
		if (IsThisValidLang(lang)) 
                    gLangid = lang; 
	    }
	    else if (lstrcmpi(__argv[i], "/h") == 0) {
		bPrintUsage = TRUE;
	    }
	    else if (lstrcmpi(__argv[i], "SPONSORS=0") == 0) {
		bSponsorsEnabled = FALSE;
	    }
        else if (lstrcmpi(__argv[i], "RUNONCE=1") == 0) {
            
            // Set flag and PING value when iftw running via "RunOnce"
            bRunOnce = TRUE;
            lstrcpy(szMethod, INSTALLMETHOD_RUNONCE);
            
        }
	    else if(lstrcmp(__argv[i], "/v")==0) {
		//The argument is /v, so read quotes before and after next arg
		lstrcat(szInstallerOptions, "/v\"");
		lstrcat(szInstallerOptions, __argv[++i]);
		lstrcat(szInstallerOptions, "\" ");
	    }
	    else if(lstrcmp(__argv[i], "STATIC=1")==0) {
	        bStatic = TRUE;
	    }
            else if(lstrcmp(__argv[i], "KERNEL=1")==0) {
                bKernel = TRUE;
            }
	    else if (lstrcmpi(__argv[i], "-download") == 0) {
                //Download bundles in silent mode, but don't install
                //then create a file to indicate that download is complete
		bDownloadOnly = TRUE;
		bSilentInstall = TRUE;
	    }
        else if (lstrcmpi(__argv[i], "-no_override") == 0) {
                //Don't override the options passed in from auto-update
                bOverrideOptions = FALSE;
        }
        else if (lstrcmpi(__argv[i], "REINSTALL_NO_PROMPT=1") == 0) {
            bUninstallBeta = TRUE;
        }
	    else {
		// could be /s or /x, pass them to msi installer
		lstrcat(szInstallerOptions, __argv[i]);
		lstrcat(szInstallerOptions, " ");
	    }
	}
	//END - PARSE COMMANDLINE ARGUMENTS

        // If we're installing 6u10beta -> 6u10fcs
        // uninstall any pre-fcs build first
        if (bUninstallBeta && !bDownloadOnly){
            UninstallSixUTenBeta();
        }

	//PRINT USAGE INFO
	if (bPrintUsage)
	{
	    if (!bSilentInstall)
	    {
		::LoadString(hInstance, IDS_USAGEERROR, szTitle, 1024);
		wsprintf(szMessage, szTitle, __argv[0]);
		::LoadString(hInstance, IDS_ERROR_CAPTION, szTitle, 1024);
		::MessageBox(NULL, szMessage, szTitle, MB_OK|MB_ICONERROR);
	    }
	    nRet = -2;
	    break;
	}
	//END - PRINT USAGE INFO

	::LoadString(hInstance, IDS_ERROR_CAPTION, szTitle, 1024);

	//CHECK TO SEE IF INTERNET CONNECTION IS VALID
  	HRESULT hr ;
    int iConnectTimeout = 0;

	while(FAILED(hr=IsReachable(szIsConnectedURL))){
        
        // If running via RunOnce, it may take a while to establish network
        // connections after FBA, so wait for the connection (timeout @ 2 minutes)
        if ((bRunOnce) && (iConnectTimeout < 121)){
            iConnectTimeout++;
            Sleep (1000);
            continue;
        }
        
           if ( hr == ERROR_PROXY_AUTH_REQ) {
               //disable sponsors if behind auth proxy
               bSponsorsEnabled = FALSE;
               break;
           }
	   if (hr == ERROR_URL_NOT_FOUND) {
	      // if it's msi file, popup error that the release might be old
	      if (IsThisMsi(szIsConnectedURL)) {
		::LoadString(hInstance, IDS_BETA_IFTW_ERROR, szMessage, 1024);
		if (!bSilentInstall) ::MessageBox(NULL, szMessage, szTitle, MB_OK);
		return ERROR_MSIFILE_NOTAVAIL;
	      }
	      else {
		TCHAR szErrorMsg[256];
		::LoadString(hInstance, IDS_DOWNLOADERROR_MSG, szErrorMsg, 256);
		wsprintf(szMessage, szErrorMsg, szIsConnectedURL);
		if (!bSilentInstall) ::MessageBox(NULL, szMessage, szTitle, MB_OK);
		return -1;
	      }
	   }
           if (bSilentInstall) return -1;

           CErrorHelpDialog edialog;
           edialog.setBodyText(IDS_ERROR_MESSAGE);
           int errRet = edialog.DoModal(NULL);
           if(errRet == IDCANCEL) {
               return -1;
           }
	}
	//INTERNET CONNECTION VALID, PROCEED

	//CHECK IF THERE IS ENOUGH DISK SPACE
	if (!IsEnoughDiskSpace())
	{
	    if (!bSilentInstall || bDownloadOnly)
	    {
                ::LoadString(hInstance, IDS_DISKSPACE, szMessage, 1024);
                ::LoadString(hInstance, IDS_TERMINATE, szTitle, 1024);
                lstrcat(szMessage, szTitle);

                if (bDownloadOnly) {
                    logit(szMessage);
                } else {
                    ::MessageBox(NULL, szMessage, szTitle, MB_OK|MB_ICONERROR);
                }
	    }
	    nRet = -3;
	    break;
	}
	//ENOUGH DISK SPACE, PROCEED

	BOOL bIsWIInstalled = IsInstallFromJavaUpdate() || IsWIVerValid(NULL);
	while(!bIsWIInstalled) {
           if (bSilentInstall) return -1;

           CErrorHelpDialog edialog;
           edialog.setBodyText(IDS_REQ_WI20_MSG);
           int errRet = edialog.DoModal(NULL);
           if(errRet == IDCANCEL) {
               return -1;
           }
	   bIsWIInstalled = IsWIVerValid(NULL);
	}
	//THIS PART IS FOR MSI & MST FILE DOWNLOAD/INSTALL 
	//It will also download/parse the config file

#ifdef JKERNEL
        bKernel = TRUE;
#endif //ifdef JKERNEL

        TCHAR szOtherLangs[BUFFER_SIZE] = {NULL};
        if (InstallIntlJRE()) {
             wsprintf(szOtherLangs, "-l");
        }


        WhatsInstalled what_is_installed;
        TCHAR szDownloadFile[BUFFER_SIZE] = {NULL};

        if (!bStatic) {  // figure out what we are doing for a requested Consumer install

            if (what_is_installed.IsSameVersionStaticInstalled()) {        // if same version installed as static
                // keep static install
                // there are no PIP MSIs for static install
                bStatic = TRUE;

            } else if (what_is_installed.IsConsumerInstalled()) {
                if (what_is_installed.IsUpdateable()) {
                    // updateable consumer version
                    // Now determine 
                    // if can upgrade using patch in place
                    //    upgrade using patch in place
                    // else offline
                    //    uninstall previous consumer and install current version Consumer.

                    if (what_is_installed.IsPatchable()) {

                        TCHAR szPatchFromString[BUFFER_SIZE] = {NULL};

                        what_is_installed.GetPatchFromString(szPatchFromString, sizeof(szPatchFromString));

                        wsprintf(szDownloadFile, "jre%s-pfrom%s", VERSION, szPatchFromString);
                        
                        // need to set flag to hide installdir checkbox
                        bPreviousInstallDir = TRUE;

                        // handle build to build patching
                        if (what_is_installed.IsSameVersionConsumerInstalled()) {  // installed as consumer,
                            // same version as the new release
                            // need to skip MSI prompt to reinstall
                    
                            lstrcat(szOnlyFirstInstallerCommandOptions, " REINSTALLMODE=vaums REINSTALL=ALL");
                        }
                                                
                    } else {
                        // uninstall older consumer release / install consumer release
                        lstrcat(szOnlyFirstInstallerCommandOptions, " REMOVEEXISTING=1");
#ifdef JKERNEL
                        if (bKernel) {      // consumer-kernel version 
                            wsprintf(szDownloadFile, "jre%s-c-k", VERSION);
                        } else
#endif //ifdef JKERNEL
                        {                   // consumer-nonkernel version 
                            wsprintf(szDownloadFile, "jre%s-c%s", VERSION, szOtherLangs);
                        }
                    }

                } else if (what_is_installed.IsSameVersionConsumerInstalled()) {  // installed as consumer,
                    //same version as the new release
                    // MSI will prompt if want to reinstall.
                    
                    // if this install is from a RunOnce cmd, exit quietly
                    if (bRunOnce) {
                        exit(0);
                    }
                    
#ifdef JKERNEL
                    if (bKernel) {      // consumer-kernel version 
                        wsprintf(szDownloadFile, "jre%s-c-k", VERSION);
                    } else
#endif //ifdef JKERNEL
                    {                   // consumer-nonkernel version 
                        wsprintf(szDownloadFile, "jre%s-c%s", VERSION, szOtherLangs);
                    }
                    lstrcat(szOnlyFirstInstallerCommandOptions, " REINSTALLMODE=vaums REINSTALL=ALL");

                } else if (what_is_installed.IsNewerConsumerVersionInstalled()) {
                    
                    // if this install is from a RunOnce cmd, exit quietly
                    if (bRunOnce) {
                        exit(0);
                    }
                    
                    // already a newer Consumer version installed
                    // interpret a request to install this older version as a request to install Static

                    bStatic = TRUE;

                } else {       // same, updateable, or newer version not already installed
                    // uninstall older consumer release / install consumer release
                    lstrcat(szOnlyFirstInstallerCommandOptions, " REMOVEEXISTING=1");
#ifdef JKERNEL
                    if (bKernel) {      // consumer-kernel version 
                        wsprintf(szDownloadFile, "jre%s-c-k", VERSION);
                    } else
#endif //ifdef JKERNEL
                    {                   // consumer-nonkernel version 
                        wsprintf(szDownloadFile, "jre%s-c%s", VERSION, szOtherLangs);
                    }
                }
            } else {
                // install consumer release
#ifdef JKERNEL
                if (bKernel) {      // consumer-kernel version 
                    wsprintf(szDownloadFile, "jre%s-c-k", VERSION);
                } else
#endif //ifdef JKERNEL
                {                   // consumer-nonkernel version 
                    wsprintf(szDownloadFile, "jre%s-c%s", VERSION, szOtherLangs);
                }
            }
        }

        if (bStatic) {    // bStatic
            if (what_is_installed.IsSameVersionInstalled()) {   // if already have same version
                if (what_is_installed.IsSameVersionStaticInstalled()) {        // if installed as static
                    // prompt if want to reinstall.
                    // The MSI should do that.
                    lstrcat(szOnlyFirstInstallerCommandOptions, " REINSTALLMODE=vaums REINSTALL=ALL");
                    
                } else { // same version consumer installed
                    
                    // if this install is from a RunOnce cmd, exit quietly
                    if (bRunOnce) {
                        exit(0);
                    }
                    
                    // have static MSI uninstall same version consumer and install current version Static
                    lstrcat(szOnlyFirstInstallerCommandOptions, " REMOVEEXISTING=1");
                }
            }
            // install static
            wsprintf(szDownloadFile, "jre%s-s%s", VERSION, szOtherLangs);
        }

        // do the install

        //Disable Sponsor offer for silent installs
        //But it should be on for DownloadOnly option for JavaAutoUpdate
        if (bSilentInstall && !bDownloadOnly) {
            bSponsorsEnabled = FALSE;
        }

	GetSingleMSIFileNames(szSinglemsiCachedFile, szMSTCachedDir, gLangid, szDownloadFile, b64bit);	  

        BOOL bAltInstallDir = FALSE;
        {
            CDownloadDialog dlgSingleMSI;

            dlgSingleMSI.setIsSilentInstall(bSilentInstall);
            dlgSingleMSI.setConfigURL(szConfigURL);
            dlgSingleMSI.setInstallerURL(szInstallerURL);
            dlgSingleMSI.setInstallerCmdOpts(szInstallerOptions);
            dlgSingleMSI.setSingleMSIFile(szSinglemsiCachedFile);
            dlgSingleMSI.setMSTFile(szMSTCachedFile);
            dlgSingleMSI.SetSponsorEnabled(bSponsorsEnabled);
            dlgSingleMSI.SetDownLoadVersion(szDownloadFile);
            dlgSingleMSI.SetDownloadOnly(bDownloadOnly);

            if (bRunOnce){
                dlgSingleMSI.SetShowSplash(TRUE);
                dlgSingleMSI.SetShowWelcome(FALSE);
            } else {
                dlgSingleMSI.SetShowWelcome(TRUE);
            }

            // Disable the Welcome Dialog's checkbox if PIP or Re-install
            if (bPreviousInstallDir) {
                dlgSingleMSI.SetIsPIP(TRUE);
            }

            // Execute Installation ...
            if (bSilentInstall) {
                dlgSingleMSI.SetShowWelcome(FALSE);
                BOOL bIgnoreNBI = FALSE;
                if (dlgSingleMSI.SilentDownloadProc(bDownloadOnly, &bIgnoreNBI) == FALSE) { 
                    nRet = ERROR_SILENT_DOWNLD_FAILED;
                } else if (bIgnoreNBI && bDownloadOnly) {
                    //running with -download and the ignoreNBI element was in the XML file
                    //  so just exit to force Notify Before Download policy
                    return 0;
                }

            } else if (dlgSingleMSI.DoModal() != IDOK) {
                nRet = ERROR_DOWNLD_DLG_CANCEL;
            }

            if (nRet != 0) {
                //download failed
                if (dlgSingleMSI.IsCancelled()) {
                    wsprintf(szMessage, 
                             "Download Cancelled by User: from=%s/%s.msi, to=%s",
                             szInstallerURL, szDownloadFile, szSinglemsiCachedFile);
                } else {
                    wsprintf(szMessage, "Download failed: from=%s/%s.msi, to=%s",
                             szInstallerURL, szDownloadFile, szSinglemsiCachedFile);
                }
                logit(szMessage);
            }
            SetInstallStatInfo();

            if (nRet == 0) {
                //log msi source & destination URL's, SQE requirement
                wsprintf(szMessage, "Msi file from URL=%s/%s.msi, downloaded to=%s",
                         szInstallerURL, szDownloadFile, szSinglemsiCachedFile);
                logit(szMessage);
            }

            // this flag is set in DownloadDialog, after parsing xml file
            bSponsorsEnabled = dlgSingleMSI.GetSponsorEnabled();

            // Check if files are downloaded properly
            if ( (nRet == 0)
                    && !IsFilesDownloaded(szSinglemsiCachedFile, szMSTCachedFile, szMessage) ) {
                nRet = ERROR_MSI_DOWNLD_FAILED;
            }

#ifdef EXTRA_COMP_LIB_NAME
            if ( (nRet == 0)
                    && (!bDownloadOnly)
                    && (!UncompressDownloadedFiles(hInstance, szSinglemsiCachedFile, szMSTCachedFile, szMSTCachedDir, szMessage)) ) {
                nRet = ERROR_MSI_UNCOMPRESS_FAILED;
            }
#endif // EXTRA_COMP_LIB_NAME

            if (nRet != 0) {
                if ( (!bSilentInstall)
                       && (nRet == ERROR_SILENT_DOWNLD_FAILED || nRet == ERROR_MSI_DOWNLD_FAILED
                           || nRet == ERROR_MSI_UNCOMPRESS_FAILED || nRet == ERROR_DOWNLD_DLG_CANCEL)) {
                    ::MessageBox(NULL, szMessage, szTitle, MB_OK | MB_ICONERROR);
            }
            if (bPostStatus) {
                SendHeadRequest(szPostStatusURL, szMethod, STATE_INSTALL_COMPLETE, szCountry, FCS_VERSION, oldVersion, INSTALLER_VER, 0, nRet, nAuRet, szProteusOldVersion, BUNDLED_AUVERSION); 
            }
            break;
        }
        if (bDownloadOnly){
            _Module.Term();
            CoUninitialize();
            return nRet;
        }
        nRet = CheckFileIntegrity(szSinglemsiCachedFile, szInstallerURL);
        if ( nRet != ERROR_SUCCESS) {
            SendHeadRequest(szPostStatusURL, szMethod, STATE_INSTALL_COMPLETE, szCountry, FCS_VERSION, oldVersion, INSTALLER_VER, 0, nRet, nAuRet, szProteusOldVersion, BUNDLED_AUVERSION);
	    break;
        }

        // So far, so good, install
        if (bSilentInstall) {
	    addSilentOptions(szSinglemsiCachedFile, szInstallerOptions);
        }

        //Set registry keys to be used by Download Initated & complete pings 
        //posting.
        //And also pass in the install method, so it can be passed  back to
        //SendHeadReaquest. This is also used by msi code, not to post 'di' &
        // 'dc' pings if it's from offline
        if (bPostStatus) {
	   lstrcat(szInstallerOptions, " METHOD=");
	   lstrcat(szInstallerOptions, szMethod);
	   lstrcat(szInstallerOptions, " ");
        }

#ifdef JVECTOR
        if (IsVectorInstall()){
	    bRunVector=TRUE;
            lstrcat(szInstallerOptions, " RUNVECTOR=1");
            lstrcat(szInstallerOptions, " ");
        }
#endif

        // If the new CWelcomeDialog has been displayed, we need to skip the MSI version of the dialog
        if (dlgSingleMSI.GetShowWelcome() || bRunOnce){
            lstrcat(szInstallerOptions, " SKIPLICENSE=1");
            lstrcat(szInstallerOptions, " EULA_JAVAFX_ACCEPT=yes");
            lstrcat(szInstallerOptions, " ");
        }

            // The AltInstalldir flag is set in the CWelcomeDialog class
            bAltInstallDir = dlgSingleMSI.GetIsAltInstalldir();
        }

        int dlgStatus = IDOK;

        do {

            // if looped back to welcome dialog
            if (dlgStatus == IDC_BACK_BUTTON) {

                // Display the License Agreement Dialog
                CWelcomeDialog dlgWelcomeBack;

                dlgWelcomeBack.setIsAltInstallDir(TRUE);

                dlgStatus = dlgWelcomeBack.DoModal();

                // exit if the user did not accept the License Agreement          
                if (dlgStatus != IDOK) {
                    break;
                }
                bAltInstallDir = dlgWelcomeBack.getIsAltInstallDir();
            }

            // if the AltInstalldir flag is set in the CWelcomeDialog class
            if (bAltInstallDir) {
                CChangeFolder dlgChangeFolder;

                // display the Change Folder dialog (which will display the Browse for Folder dialog)
                dlgStatus = dlgChangeFolder.DoModal();

                if (dlgStatus == IDOK) {
                    // if IDOK, user clicked Next, add installdir to installer options

                    TCHAR szInstallDir[MAX_PATH] = {NULL};

                    // get InstallDir
                    if (dlgChangeFolder.getInstallDir(szInstallDir) == ERROR_SUCCESS) {
                        TCHAR szInstallDirOption[BUFFER_SIZE] = {NULL};

                        // add install dir to installer options
                        wsprintf(szInstallDirOption, " INSTALLDIR=\"%s\" ", szInstallDir);
                        lstrcat(szInstallerOptions, szInstallDirOption);
                    }
                } else if (dlgStatus == IDCANCEL) {
                    nRet = ERROR_USER_CANCELLED_FROM_CHANGE_FOLDER;     // value to be "pinged"
                    break;         // break out of while loop
                }
            }
        }
        while (dlgStatus == IDC_BACK_BUTTON);

        if (dlgStatus != IDOK) {
            // don't ping decline (Cancel) from Welcome dialog, only ping when nRet is set
            if (nRet != ERROR_SUCCESS) {
                SendHeadRequest(szPostStatusURL, szMethod, STATE_INSTALL_COMPLETE, szCountry, FCS_VERSION, oldVersion, INSTALLER_VER, 0, nRet, nAuRet, szProteusOldVersion, BUNDLED_AUVERSION);
            }

            TCHAR szLogMessage[BUFFER_SIZE] = {NULL};
            wsprintf(szLogMessage, "Exited install ChangeFolder dialog: dlgStatus=%d, return=%d\n", dlgStatus, nRet);
            logit(szLogMessage);

            break;                  // break out of do loop
        }

#ifdef DEBUG
if (::MessageBox(NULL, szInstallerOptions, "DEBUG", MB_OKCANCEL|MB_ICONINFORMATION) == IDCANCEL) {
    break;
}
#endif

        ExtractSponsorDLL(hInstance);

        nRet = ExecuteInstallation(szSinglemsiCachedFile, szInstallerOptions, szOnlyFirstInstallerCommandOptions, szMSTCachedFile);
        
        RunStuffPostInstall();

        //include in kernel jre for 6u18b04       
        //#ifndef JKERNEL 
        //only attempt AU install if JRE install was successful
        if(nRet == 0) {
            //Install Auto Update
            TCHAR szExecutableName[MAX_PATH];
            TCHAR szCabName[MAX_PATH];
            TCHAR szAppDataDir[MAX_PATH] = {0};
            GetUserShellFolder(szAppDataDir); 
            //Extract au.msi
            wsprintf(szAppDataDir, "%s\\Sun\\Java\\AU\\",szAppDataDir);
            CreateDirectory(szAppDataDir, NULL);
            wsprintf(szExecutableName, "%sau.msi", szAppDataDir);
            if(ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_AUINSTALLER),   "JAVA_INSTALLER", szExecutableName)){
                 //Extract au.cab for au.msi
                 wsprintf(szCabName, "%sau.cab", szAppDataDir); 
                ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_AUINSTALLERCAB),   "JAVA_INSTALLER", szCabName);

                // Before installing the bundled AU msi, uninstall any older version first          
                RemoveAnyOlderAU2X();

                nAuRet = ExecuteInstallation(szExecutableName, "ALLUSERS=1 /qn","","");
                 wsprintf(szMessage, "install au.msi \"%s\nAU Install Return Code: %d", szExecutableName, nAuRet);
                 logit(szMessage);

                //Register jre to auto-update
                char CommonDir[MAX_PATH] = {0};//store the location of AppData directory

                if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, CommonDir))) {
                    wsprintf(szExecutableName, "\"%s\\Java\\Java Update\\jaureg.exe\" -r jre %s", CommonDir, FULLVERSION);
                    ExecProcess(NULL,szExecutableName);
                    logit(szExecutableName);
                } else {
                    logit("unable to register jre");
                }
            }
        }
//#endif
        // Send InstallComplete with error code.
        SendHeadRequest(szPostStatusURL, szMethod, STATE_INSTALL_COMPLETE,  szCountry, FCS_VERSION, oldVersion, INSTALLER_VER, 0, nRet, nAuRet, szProteusOldVersion, BUNDLED_AUVERSION);
        
        // If a the user has cancelled the installation via FilesInUse, and opted for a Re-try,
        // Finalize the RunOnce cmdline and set a flag to keep the MSI and MST files on the machine
        // NOTE: this call MUST be made AFTER SendHeadRequest() for the -138 and -139 pings to be set correctly
        if (FinalizeRunOnce()){
            bKeepAppDataFiles = TRUE;
        }
        
    }
    while (0);

    // We want to keep the MSI and MST files when the user has cancelled FilesInUse 
    // and opted for a retry. so only delete these files for installations that are
    // NOT setting up a RunOnce, aka install re-try.
    if (!bKeepAppDataFiles){
        ::DeleteFile(szSinglemsiCachedFile);
        ::DeleteFile(szMSTCachedFile);
        RemoveSP1Dll();
    }

    //Delete InstallStatus, Visid & method registry keys at the end of
    //installation
    SetInstallStatsKey(NULL);
	//Delete Country code after the installation  
	DeleteCountry();
	logit("Country code is deleted after installation");
#ifndef JKERNEL
    //need these for sending complete ping from java kernel
    SetJavaUpdateStringKey(NULL, REG_JUPDATE_VISID, NULL);
    SetJavaUpdateStringKey(NULL, REG_JUPDATE_METHOD, NULL);
#endif
    if (IsInstallFromJavaUpdate()) {
        CleanupJU();
    } 
    if (bRunVector) {
      DeleteRegVectorURL();   
    } 
 
    _Module.Term();
    CoUninitialize();

    return nRet;
}

//=--------------------------------------------------------------------------=
//  FinalizeRunOnce -- returns true or false
//
// If the 'RunOnce' key was created via FilesInUse, prepend the string value
// with the path to the wrapper exe (current process)
// Note: the HKLU\\MicroSoft\\Windows\\CurrentVersion\\RunOnce key should always
// exist, here we're checking for the FIURetry key
//=--------------------------------------------------------------------------=
BOOL FinalizeRunOnce()
{
    
    HKEY hKey;
    TCHAR szRunOnceCmd[BUFFER_SIZE] = {NULL};
    ULONG cchRunOnceSize = BUFFER_SIZE;
    BOOL bRet = FALSE;
    
    // Open the RunOnce Key, and look for "FIURetry" value
    // Ignore RunOnce keys that have been set up by other JRE installs
    // (i.e., the .exe name has already been written to the key value)
    if(RegOpenKeyEx(HKEY_CURRENT_USER, RUNONCEKEY, 0, KEY_QUERY_VALUE | KEY_WRITE, &hKey) == ERROR_SUCCESS){
        
        if (RegQueryValueEx(hKey, FIURETRYKEY, NULL, NULL, (LPBYTE)szRunOnceCmd, &cchRunOnceSize) == ERROR_SUCCESS  && (strstr(szRunOnceCmd, RUNONCEEXE) == NULL)){
            
            // Copy this process' iftw to the %APPDATA% dir
            TCHAR szThisPath[BUFFER_SIZE] = {NULL};
            if (GetModuleFileName(0, szThisPath, BUFFER_SIZE) != 0){
            
                // We don't want to put the retry iftw.exe in a version-specific directory
                // this will guarantee that only one runonce .exe will exist on the machine,
                // and facilitate cleanup for installers of different revisions.
                TCHAR szAppDataDir[BUFFER_SIZE] = {NULL};
                TCHAR szTempDirectory[BUFFER_SIZE] = {NULL};
                
                // First make sure %APPDATA% and %TEMP% can be obtained
                if ((GetUserShellFolder(szAppDataDir)) && (GetTempPath(BUFFER_SIZE, szTempDirectory) != 0)){
                
                    TCHAR szTempIftw[BUFFER_SIZE] = {NULL};
                    TCHAR szAppDataCfg[BUFFER_SIZE] = {NULL};
                    TCHAR szTempCfg[BUFFER_SIZE] = {NULL};
                    wsprintf(szTempIftw, "%s\\Sun\\Java\\%s", szAppDataDir, RUNONCEEXE);
                    wsprintf(szTempCfg, "%s\\%s", szTempDirectory, "jinstall.cfg");
                    wsprintf(szAppDataCfg, "%s\\Sun\\Java\\%s", szAppDataDir, "jinstall.cfg");
                    
                    // Now copy the iftw and jinstall.cfg files to %APPDATA%
                    if ((::CopyFile(szThisPath, szTempIftw, FALSE) != 0) && (::CopyFile(szTempCfg, szAppDataCfg, FALSE) != 0)){
                                        
                        // Build RunOnce cmdline and wrap in quotes 
                        TCHAR szRunOnceCmdLine[BUFFER_SIZE] = {NULL};
                        lstrcpy(szRunOnceCmdLine, "\"");
                        lstrcat(szRunOnceCmdLine, szTempIftw);
                        lstrcat(szRunOnceCmdLine, "\" ");
                        lstrcat(szRunOnceCmdLine, szRunOnceCmd);
                    
                        // Write cmdline to RunOnce Key value (remember +1 to strlen for NULL terminator!)
                        RegSetValueEx(hKey, FIURETRYKEY, 0, REG_SZ, (LPBYTE)szRunOnceCmdLine, lstrlen(szRunOnceCmdLine) + 1);
                        bRet = TRUE;
                    
                    }
                }
            }
            
            // if bRet is not TRUE here, there were issues in trying to
            // finalize the RunOnce regkey, abort the installer retry
            if (! bRet){
                RegDeleteValue(hKey, FIURETRYKEY);
            }
            
        }
        
        // Close the RunOnce hKey
        RegCloseKey(hKey);
        
    }
        
    return bRet;
    
}

//=--------------------------------------------------------------------------=
//  RemoveFile -- It takes a file path and returns true or false
//
//  This function checks its read-only and invalid attributes
//  and resets the attribs to archive and then deletes it
//=--------------------------------------------------------------------------=
BOOL RemoveFile(LPCTSTR lpszFilePath)
{
    DWORD dwAttribs = GetFileAttributes(lpszFilePath);
    if (dwAttribs==INVALID_FILE_ATTRIBUTES){
        return FALSE;
    }
    if (dwAttribs & FILE_ATTRIBUTE_READONLY){
        SetFileAttributes(lpszFilePath, FILE_ATTRIBUTE_ARCHIVE);
    }
    if(DeleteFile(lpszFilePath)){
        return TRUE;
    }
    return FALSE;
}
//=--------------------------------------------------------------------------=
//  ExecuteInstalltion -- It takes an application path and the command options
//		          and returns void.
//
//  This function is used to execute an application in the path and return 
//  when the application is exited.
//=--------------------------------------------------------------------------=
DWORD ExecuteInstallation(LPCTSTR lpApplicationName, LPTSTR lpszCommandOptions, LPCTSTR lpszOnlyFirstCommandOptions, LPCTSTR lpszMST)
{
    TCHAR szExecutableName[1024] = {NULL};
    TCHAR szCommandLine[1024] = {NULL};
    TCHAR szOnlyFirstCommandLine[1024] = {NULL};
    DWORD error = ERROR_UNKNOWN;
	DWORD dwCount;

    if (IsThisMsi(lpApplicationName))
    {
	//query the registry key for WI location and return the value.

    	if (!GetWIPath(szExecutableName, sizeof(szExecutableName))) 
	{
	    return error;
	}

	if (lstrlen(lpszMST) == 0) {
	    wsprintf(szCommandLine, "\"%s\" /i \"%s\" %s", 
		     szExecutableName, lpApplicationName, 
		     lpszCommandOptions);
	}
    else {
	    if (bSponsorsEnabled) {
	        if (lstrlen(szCountry) == 0){
					dwCount = sizeof(szCountry);
					GetCountry(szCountryServletURL, szCountry);

			}
                lstrcat(lpszCommandOptions, " COUNTRY=");
                lstrcat(lpszCommandOptions, szCountry);
                lstrcat(lpszCommandOptions, " PREFERENCEORDER=");
                lstrcat(lpszCommandOptions, szPreferenceOrder);
        }
		wsprintf(szCommandLine, "\"%s\" /i \"%s\" TRANSFORMS=\"%s\" %s",
		szExecutableName, lpApplicationName,
		lpszMST, lpszCommandOptions);
	}
    }
    else {
        wsprintf(szExecutableName, lpApplicationName);
        wsprintf(szCommandLine, "\"%s\" %s", lpApplicationName, lpszCommandOptions);
    }

    //New flag to handle situation when the user does a reinstall
    //PROG=0 shows the ATL setup dialog, otherwise it shows the
    //MSI setup. 
    wsprintf(szCommandLine, "%s PROG=0", szCommandLine);
    wsprintf(szOnlyFirstCommandLine, "%s %s", szCommandLine, lpszOnlyFirstCommandOptions);

    {
        //Create the setupProgress window in "hidden" mode
        CSetupProgressDialog dlgSetupProgress;
        dlgSetupProgress.Create(::GetDesktopWindow());
        // Execute the application
        //
        error = ExecProcess(szExecutableName, szOnlyFirstCommandLine);
        dlgSetupProgress.DestroyWindow();
    }
    
    //Check if it's the reinstall case. If so, then relaunch the cmd
    TCHAR buf[512] = {NULL};
    GetJavaSoftKey(REG_INSTALLSTATS_STATUS, buf, sizeof(buf));
    if (lstrcmp(buf, "reinstall") == 0) {
        SetInstallStatInfo();
        //Create the setupProgress window for the reinstall case
        CSetupProgressDialog dlgSetupReinstall;
        dlgSetupReinstall.Create(::GetDesktopWindow());
        error = ExecProcess(szExecutableName, szCommandLine);
        dlgSetupReinstall.DestroyWindow();
    }
    return error;
}

DWORD UninstallSixUTenBeta()
{
    
    DWORD error = ERROR_UNKNOWN;
    TCHAR szExeName[BUFFER_SIZE] = {NULL};
    TCHAR szAppDataDir[BUFFER_SIZE] = {NULL};
    TCHAR szActiveDir[BUFFER_SIZE] = {NULL};
    TCHAR szTmpDir[BUFFER_SIZE] = {NULL};
    
    // Get the path to msiexec
    if (!GetWIPath(szExeName, sizeof(szExeName))){
        return error;
    }
    
    // Save off the %APPDATA%/Sun/Java/jre1.6.0_10 directory
    // before uninstalling the 6u10 beta
    GetUserShellFolder(szAppDataDir);
    wsprintf(szActiveDir, "%s\\Sun\\Java\\jre1.6.0_10\\", szAppDataDir);
    wsprintf(szTmpDir, "%s\\Sun\\Java\\jre1.6.0_10.tmpsv\\", szAppDataDir);
    if (MoveFile(szActiveDir, szTmpDir) == FALSE){
        return error;
    }
    
    // Run the uninstall in basic mode for the 6u10beta GUID
    // and restore %APPDATA%/Sun/Java/jre1.6.0_10
    error = ExecProcess(szExeName, " /x {26A24AE4-039D-4CA4-87B4-2F83216010FF} /qb");
    if (MoveFile(szTmpDir, szActiveDir) == FALSE){
        return ERROR_UNKNOWN;
    }
    return error;

}

//=--------------------------------------------------------------------------=
//  DownloadFailure -- This function popup an error dialog and perform cleanup.
//=--------------------------------------------------------------------------=
void DownloadFailure()
{
    // Show failure dialog box
    CSimpleDialog<IDD_FAILURE_DIALOG> failDlg;
    failDlg.DoModal();

//    CleanupDownload();
}

void SetPostStatus(BOOL bFlag)
{
    // PostStatus is off for JDK installations
    if (!IsThisJDK()){
        bPostStatus = bFlag;
    }
    if (!bPostStatus)
        SetJavaUpdateStringKey(NULL, REG_JUPDATE_POSTSTATUSURL, (LPTSTR)NULL);
}

void SetPostStatusURL(LPCTSTR lpszURL)
{
    _tcscpy(szPostStatusURL, lpszURL);
}

void SetCountryLookupURL(LPCTSTR lpszURL)
{
    _tcscpy(szCountryServletURL, lpszURL);
}

BOOL IsInstallFromJavaUpdate()
{
   if (lstrcmpi(szMethod, INSTALLMETHOD_AUTOUPDATE)==0 || lstrcmpi(szMethod, INSTALLMETHOD_MANUALUPDATE)==0)
     return TRUE;
   return FALSE;
}

BOOL OverrideOptions()
{
    return bOverrideOptions;
}

BOOL SetMSTFileNames(BOOL bSponsor, LPSTR pszMSTCachedFile, LPSTR pszMSTSrcURL)
{
    TCHAR szFileName[128];
    if (!bSponsor && (gLangid == LANGID_ENGLISH)) {
        pszMSTCachedFile[0] = NULL;
        return FALSE;
    }
    if (bSponsor) {
      wsprintf(szFileName, "sp%d.MST", gLangid);
    }
    else {
      wsprintf(szFileName, "jre%d.MST", gLangid);
    }
    lstrcat(pszMSTSrcURL, szFileName);
    wsprintf(pszMSTCachedFile, "%s\\%s", szMSTCachedDir, szFileName);
    return TRUE;
}


void SetPreferenceEngineOrder(LPCTSTR lpszPreference)
{
    _tcscpy(szPreferenceOrder, lpszPreference);

}


void SetVPreloadURLValue(LPCTSTR lpszVPreloadURL)
{
    _tcscpy(szVPreloadURL, lpszVPreloadURL);

}

void SetVRunURLValue(LPCTSTR lpszVRunURL)
{
    _tcscpy(szVRunURL, lpszVRunURL);

}

void SetVMinOSValue(LPCTSTR lpszVMinOS)
{
    _tcscpy(szVMinOS, lpszVMinOS);

}

void SetVMinPSpeedValue(LPCTSTR lpszVMinPSpeed)
{
    _tcscpy(szVMinPSpeed, lpszVMinPSpeed);

}

void SetVMinMemoryValue(LPCTSTR lpszVMinMemory)
{
    _tcscpy(szVMinMemory, lpszVMinMemory);

}

void SetVLocaleValue(LPCTSTR lpszVLocale)
{
    _tcscpy(szVLocale, lpszVLocale);

}

void SetVGeosValue(LPCTSTR lpszVGeos)
{
    _tcscpy(szVGeos, lpszVGeos);

}

void SetVAuxNameValue(LPCTSTR lpszAuxName)
{
    _tcscpy(szVInstallAuxName, lpszAuxName);

}

//Create the temp registry keys of the URLs for the installer
BOOL SetRegVectorURL(LPCTSTR lpURLValue,LPCTSTR lpURLName)
{
    DWORD created = 0, dwCount = 0;
    HKEY hKey;

   if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft",
		0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, &created) != ERROR_SUCCESS)
	return FALSE;

    RegSetValueEx(hKey, lpURLName, 0, REG_SZ, 
			  (LPBYTE)lpURLValue, lstrlen(lpURLValue));
    RegCloseKey(hKey);

    return ERROR_SUCCESS;
}

//Delete the temp registry keys of the URLs
void DeleteRegVectorURL() {
  HKEY hKey;

  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft", 0, KEY_WRITE, &hKey) == ERROR_SUCCESS) {
    RegDeleteValue(hKey, "InstallAuxPreloadUrl");
    RegDeleteValue(hKey, "InstallAuxRunUrl");
    RegCloseKey(hKey);
  }

}

BOOL IsVectorInstall()
{
 

    //If Vector is already installed on the sytem do nothing for the vector
    if (lstrlen(szVInstallAuxName) != 0) {
      BOOL bIsAuxInstalled = IsAuxInstalled();
      if (bIsAuxInstalled) {
	return FALSE;
      }
    }

   //If both preload-url and run-url are empty do nothing for the vector
    if ((lstrlen(szVPreloadURL) == 0) && (lstrlen(szVRunURL) == 0)) {
      return FALSE;
    }

    //Verify if the user's system meets the vector requirement

    if ((lstrlen(szVMinPSpeed) != 0) && (!IsMinProcessSpeed())) {
      return FALSE;
    }

    if ((lstrlen(szVMinMemory) != 0) && (!IsMinMemorySize())) {
      return FALSE;
    }


    // This function may be called by the WelcomeDialog classe, which is created before
    // installstats determines the country, but after the Country lookup URL is determined
    // So set szCountry for the Vector check in the Welcome dialog.
    if (lstrlen(szVGeos) != 0) {
        DWORD dwCount = sizeof(szCountry);
        if( !GetJavaUpdateKey(NULL, REG_JUPDATE_COUNTRY, szCountry, &dwCount)){
            GetCountry(szCountryServletURL, szCountry);
        }
      if (!IsStringinXMLList(szVGeos, szCountry)) {
	return FALSE;
      }
    }

    //verify the locale meets the requirement
    if (lstrlen(szVLocale) != 0) {
      TCHAR  szLang[MAX_PATH] = {NULL};
      wsprintf(szLang, "%d", gLangid);
      if (!IsStringinXMLList(szVLocale, szLang)) {
	return FALSE;
      }
    }

    //verify the OS meets the requirement
    if ((lstrlen(szVMinOS) != 0) && (!IsMinOS())) {
      return FALSE;
    }

    BOOL bIsVectorRunOnce = IsVectorRunOnce();
    BOOL bIsJavaFXKeyFound = IsJavaFXKeyFound();
  
    //JavaFX should be on the system and the same run-url is not executed. If the conditions are met save the URLs in the registry for the installer to use later.
    if ( bIsJavaFXKeyFound && !bIsVectorRunOnce) {
      if (lstrlen(szVPreloadURL) != 0) {
	SetRegVectorURL(szVPreloadURL, "InstallAuxPreloadUrl");
      }
      if (lstrlen(szVRunURL) != 0) {
	SetRegVectorURL(szVRunURL, "InstallAuxRunUrl");
      }
      return TRUE;

    }
    return FALSE;
}

//Verify the value is in the list from xml
BOOL IsStringinXMLList(LPCTSTR lpList, LPCTSTR szLocalStr)
{
    char * strValue = NULL;
    TCHAR szList[MAX_PATH] = {NULL};
    wsprintf(szList, lpList);
    strValue = strtok (szList, " ");

    while (strValue != NULL) {
      if (lstrcmpi(strValue, szLocalStr) == 0){
	return TRUE;
      }        
      strValue = strtok (NULL, " ");
    }

    return FALSE;
}

// Find installed vector
BOOL IsAuxInstalled()
{

    CRegKey swKey, VectorKey;

    if (swKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", KEY_READ) != ERROR_SUCCESS)
	return FALSE;

    if (VectorKey.Open(swKey, szVInstallAuxName, KEY_READ) != ERROR_SUCCESS){
	return FALSE; 
    }
    return TRUE;
}


//Check if the OS meets the requirement
BOOL IsMinOS()
{

    OSVERSIONINFO os = { sizeof(os) };
    GetVersionEx(&os);
    TCHAR szOSVersion [20] = {NULL};
    double dLocalOSVersion;
    double dXMLOSVersion;
    

    wsprintf(szOSVersion, "%d.%d", os.dwMajorVersion, os.dwMinorVersion);
  
    dLocalOSVersion= atof(szOSVersion);
    dXMLOSVersion= atof(szVMinOS);

    if ( VER_PLATFORM_WIN32_NT == os.dwPlatformId && dLocalOSVersion >= dXMLOSVersion) {
            return TRUE;  
    } else {
      return FALSE;
    }
  
    return TRUE;
}


//Verify if the run-url has been executed before
BOOL IsVectorRunOnce()
{

  HKEY hKey;
  BOOL ret = FALSE;
  TCHAR szkeyvalue[MAX_PATH] = {NULL};
  DWORD dwType, cbData = MAX_PATH;


  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft", 0, KEY_READ, &hKey) ==
      ERROR_SUCCESS) {
    if (RegQueryValueEx(hKey, "InstallAuxRunOnce", NULL, &dwType,
			(UCHAR *)szkeyvalue, &cbData) == ERROR_SUCCESS) {
      if (lstrcmpi(szkeyvalue, szVRunURL)==0) {
	ret = TRUE;
      }
    }
    RegCloseKey(hKey);
  }
  return ret;


}


//Verify if the JavaFX exists by checking the javafx update key
BOOL IsJavaFXKeyFound()
{


DWORD BufSize = BUFFER_SIZE;
DWORD dwMHz = BUFFER_SIZE;
HKEY hKey;

// open the JavaFx key:
long lError = RegOpenKeyEx(HKEY_CURRENT_USER,
                        "SOFTWARE\\JavaSoft\\Java Update\\Policy\\JavaFX",
                        0,
                        KEY_READ,
                        &hKey);
    
    if(lError != ERROR_SUCCESS){
      // if the key is not found also check system location
        lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        "SOFTWARE\\JavaSoft\\Java Update\\Policy\\JavaFX",
                        0,
                        KEY_READ,
                        &hKey);
        if(lError != ERROR_SUCCESS){
            // if the key is not found
            return FALSE;
        }

    }
   return TRUE;
}

//Verify the OS has the required process speed for vector app
BOOL IsMinProcessSpeed()
{

DWORD BufSize = BUFFER_SIZE;
DWORD dwMHz = BUFFER_SIZE;
HKEY hKey;

// open the key where the proc speed is hidden:

long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                        0,
                        KEY_READ,
                        &hKey);
    
    if(lError != ERROR_SUCCESS){
      // if the key is not found
      return FALSE;
    }

    // query the key:
    RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE) &dwMHz, &BufSize);

    double dVMinPSpeed = ((atof(szVMinPSpeed))*1000);


    if  (dwMHz >= dVMinPSpeed) {
      return TRUE;
    }

    return FALSE;
}


//Verify the OS has the required memory size for vector app
BOOL IsMinMemorySize()
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    GlobalMemoryStatusEx(&statex);

    double dVMinMemory;
    dVMinMemory = ((atof(szVMinMemory))*1024);


    if  (((statex.ullTotalPhys/1024)/1024) >= dVMinMemory) {
      return TRUE;
    }
    return FALSE;
}

