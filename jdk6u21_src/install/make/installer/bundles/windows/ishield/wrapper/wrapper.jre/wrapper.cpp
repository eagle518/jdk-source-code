/*
 *  @(#)wrapper.cpp	1.67 10/06/17 
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
#include "PatchInPlace.hpp"

CComModule _Module;

#define BUFFER_SIZE 1024
#define ERROR_PROXY_AUTH_REQ HRESULT_FROM_WIN32(0x210)

#define SPONSOR_CMDLINE_OPTION "SPWEB="
BOOL bSponsors = TRUE;
BOOL bPostStatus = FALSE;
TCHAR szPostStatusURL[256];
TCHAR szCountryServletURL[256];
TCHAR szPreference[256];
TCHAR szCountry[20] = {NULL};
TCHAR szCountryOverride[20] = {NULL};
LANGID gLangid;
TCHAR szPath[MAX_PATH];
TCHAR szMethod[20];

#ifdef SPONSOR
//Enabled if SPWEB=<url> exists in cmdline options && SPONSORS=0 not set on cmdline
//If Enabled, then fetch the baseurl for sponsor software,
//append COUNTRY to the cmdline options & return TRUE
BOOL IsSponsorEnabled(LPSTR lpszCmdlineOptions, LPSTR lpszBaseURL)
{
    if (lstrlen(lpszCmdlineOptions)== 0) return FALSE;
    LPSTR begin = strstr(lpszCmdlineOptions, SPONSOR_CMDLINE_OPTION);
    if (begin != NULL ) {
        begin += lstrlen(SPONSOR_CMDLINE_OPTION);
        LPSTR end  = strstr(begin, " ");
	if (end == NULL)
            lstrcpy(lpszBaseURL, begin);
	else
	    lstrcpyn(lpszBaseURL, begin, end-begin+1);
    }
    if ((lstrlen(lpszBaseURL)== 0) || !IsThisAllowedURL(lpszBaseURL)) return FALSE;
    lstrcat(lpszCmdlineOptions, " COUNTRY=");
    lstrcat(lpszCmdlineOptions, szCountry);
    return TRUE;
}
#endif //ifdef SPONSOR

#define XML_JUPDATE_ELEMENT "java-update"
#define XML_INFORMATION_ELEMENT "information"
#define XML_VERSION_ATTRIBUTE "version"
#define XML_LANGUAGE_ATTRIBUTE "xml:lang"
#define XML_OFFLINE_OPTIONS_ELEMENT "offline-options"
//set to the url of InstallStats servlet. The status information is posted, if 
//value is non zero. Default is NOT to post, in the absence of this element.
#define XML_POSTSTATUS_ELEMENT "post-status"
//Server name for CountryLookup
#define XML_CNTRYLOOKUP_ELEMENT "cntry-lookup"
#define XML_PREFERENCE_ENGINE_ELEMENT "sponsor-preference"


//parse the xml file contents
//prepend the options from xml file to the cmd line options given.
//set szPostStatusURL
void ParseConfiguration(LPTSTR szConfigBuffer, LPTSTR szInstallerOptions)
{
    char *lang = GetLocaleStr();
    char *englang = "en";

    // Parse XML document
    XMLNode* xmlNode = ParseXMLDocument(szConfigBuffer);
    
    // Root element must be "java-update"
    if (xmlNode != NULL && lstrcmp(xmlNode->_name, XML_JUPDATE_ELEMENT) == 0) {    
	    // Iterate "information" element
	    XMLNode* xmlInfoNode = xmlNode->_sub;

	    while (xmlInfoNode != NULL) {
		if (lstrcmp(xmlInfoNode->_name, XML_INFORMATION_ELEMENT) == 0) {
		    char* version = FindXMLAttribute(xmlInfoNode->_attributes, XML_VERSION_ATTRIBUTE);
		    char* szLanguage = FindXMLAttribute(xmlInfoNode->_attributes, XML_LANGUAGE_ATTRIBUTE);
    
		    if ((lstrcmp(version, "1.0") == 0) 
			 && ((lstrcmp(szLanguage, lang) == 0) ||
			       (lstrcmp(szLanguage, englang) == 0))) {
			// this is the right locale or english(default)
			// if we don't find the right locale, take 
			// english locale part of XML 
    
			XMLNode* xmlOptionsNode = FindXMLChild(xmlInfoNode->_sub, XML_OFFLINE_OPTIONS_ELEMENT);
                        XMLNode* xmlPostStatusNode = FindXMLChild(xmlInfoNode->_sub, XML_POSTSTATUS_ELEMENT);
			XMLNode* xmlCntryLookupNode = FindXMLChild(xmlInfoNode->_sub, XML_CNTRYLOOKUP_ELEMENT);
                        XMLNode* xmlPreferenceEngineNode = FindXMLChild(xmlInfoNode->_sub, XML_PREFERENCE_ENGINE_ELEMENT);
			XMLNode* xmlPcData;
    
			 if (xmlOptionsNode != NULL && xmlOptionsNode->_sub != NULL)
			 {
				xmlPcData = xmlOptionsNode->_sub;
				_tcscpy(szInstallerOptions, xmlPcData->_name);
			}
                        bPostStatus = FALSE;
                        if (xmlPostStatusNode != NULL && xmlPostStatusNode->_sub != NULL)
                        {
                            xmlPcData = xmlPostStatusNode->_sub;
			    if (lstrcmp(xmlPcData->_name, "0") != 0) {
                                _tcscpy(szPostStatusURL, xmlPcData->_name);
                                bPostStatus = TRUE;
                            }
                        }
                        if (xmlCntryLookupNode != NULL && xmlCntryLookupNode->_sub != NULL)
                        {
                           xmlPcData = xmlCntryLookupNode->_sub;
			   _tcscpy(szCountryServletURL, xmlPcData->_name);
                        }
                        // Get preference engine order
                        //
                        if (xmlPreferenceEngineNode != NULL && xmlPreferenceEngineNode->_sub !=NULL)
                        {
                            XMLNode* xmlPcData = xmlPreferenceEngineNode->_sub;
                            _tcscpy(szPreference, xmlPcData->_name);
                        } 

			// break if we found the right locale stuff
			if (lstrcmp(szLanguage, lang) == 0)
			    break; 
		    }
		}
		// Iterate over next information node
		xmlInfoNode = xmlInfoNode->_next;
	    }
    }
    return ;
}

BOOL DownloadFile(LPCTSTR szSrc, LPCTSTR szDest, LPSTR szBuffer, DWORD dwSize)
{
    HINTERNET hOpen = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    BOOL bRet = TRUE;

    __try {
	// Open Internet Call
	TCHAR szHostName[BUFFER_SIZE], szUrlPath[BUFFER_SIZE], szExtraInfo[BUFFER_SIZE];
	hOpen = ::InternetOpen("jupdate", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);

	if (hOpen == NULL) {
	    bRet = FALSE;
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
	::InternetCrackUrl(szSrc, lstrlen(szSrc), NULL, &url_components);
	
	// Open Internet Connection
	hConnect = ::InternetConnect(hOpen, url_components.lpszHostName, 
                                     url_components.nPort,
				     "", "", INTERNET_SERVICE_HTTP, NULL, NULL);
	if (hConnect == NULL) {
	    bRet = FALSE;
	    __leave;
	}   

	// Determine the relative URL path by combining 
	// Path and ExtraInfo

	if (url_components.dwUrlPathLength !=  0)
	    lstrcpy(szBuffer, url_components.lpszUrlPath);
	else
	    lstrcpy(szBuffer, "/");

	if (url_components.dwExtraInfoLength != 0)
	    lstrcat(szBuffer, url_components.lpszExtraInfo);

	// Make a HTTP GET request
	hRequest = ::HttpOpenRequest(hConnect, "GET", szBuffer, "HTTP/1.1", "",
                       NULL, 
                       INTERNET_FLAG_KEEP_CONNECTION | INTERNET_FLAG_DONT_CACHE,
                       0);

	if (hRequest == NULL) {
	    bRet = FALSE;
	    __leave;
	}   
	// Don't retry for offline installers & Don't continue if auth proxy
	if (FALSE == ::HttpSendRequest(hRequest, NULL, NULL, NULL, NULL)) {
	    bRet = FALSE;
	    __leave;
	}
	//
	// Read HTTP status code
	DWORD dwStatus=0;
	DWORD dwStatusSize = sizeof(DWORD);
	   
	::HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | 
		 HTTP_QUERY_STATUS_CODE, &dwStatus, &dwStatusSize, NULL);

	// Read from HTTP connection and write into destination file or buffer
        bRet = FALSE;
	if (dwStatus == HTTP_STATUS_OK) {
	    DWORD nRead = 0;
            if (szDest == NULL) {
	        if (::InternetReadFile(hRequest, szBuffer, dwSize, &nRead)
	              && (nRead > 0)) {
                    bRet = TRUE;
                }
                __leave;
            }
            DWORD dwNumberOfBytesWritten = 0;
            DWORD dwTotalRead = 0;
            HANDLE hFile = INVALID_HANDLE_VALUE;
            hFile = ::CreateFile(szDest, GENERIC_WRITE, 0, NULL,
                                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE) {
                __leave;
            }
            bRet = TRUE;
            do {
	      if (::InternetReadFile(hRequest, szBuffer, dwSize, &nRead)) {
	        if (nRead > 0) {
                  ::WriteFile(hFile, szBuffer, nRead, &dwNumberOfBytesWritten, 
                                 NULL);
                  dwTotalRead += nRead;
                }
              } else {
                  bRet = FALSE;
                  break;
              }
	    } while (nRead);
            ::CloseHandle(hFile);
            if (dwTotalRead <= 0) {
                bRet = FALSE;
            }
	}
    } __finally {
	if (hRequest)
	    ::InternetCloseHandle(hRequest);
	if (hConnect)
	    ::InternetCloseHandle(hConnect);
	if (hOpen)
	    ::InternetCloseHandle(hOpen);
    }
    return bRet;
}

void GetOptionsFromXmlIfOnline(HINSTANCE hInstance, LPTSTR szXmlCmdLine, LPCTSTR szLocalDir, LPTSTR szTransformPath)
{
    szCountryServletURL[0] = NULL;
    szXmlCmdLine[0] = NULL;
    lstrcpy(szPostStatusURL, "");

    const int size = BUFFER_SIZE * 10;
    TCHAR szBuffer[size];
    if (!GetJavaSoftKey(REG_INSTALLER_XMLURL, szPath, BUFFER_SIZE))
        wsprintf(szPath, "%s", DEFAULT_XML);
    if (DownloadFile(szPath, NULL, szBuffer, size)) {
	ParseConfiguration(szBuffer, szXmlCmdLine); 
        if (bPostStatus || bSponsors) {
            DWORD dwCount = sizeof(szCountry);
            if( !GetJavaUpdateKey(NULL, REG_JUPDATE_COUNTRY, szCountry, &dwCount)){
    	        GetCountry(szCountryServletURL, szCountry);
            }
	}
#ifdef SPONSOR
        szBuffer[0] = NULL;
        if (bSponsors) {
            bSponsors = IsSponsorEnabled(szXmlCmdLine, szBuffer);
        }
        if (bSponsors) {
            // download sponsor transform
            wsprintf(szTransformPath, "%ssp%d.MST", szLocalDir, gLangid);
            wsprintf(szPath, "%s/sp%d.MST", szBuffer, gLangid);
            if ( DownloadFile(szPath, szTransformPath, szBuffer, size)
  #ifdef EXTRA_COMP_LIB_NAME
                  && ExtraUnCompression(hInstance, szLocalDir, szTransformPath)
  #endif
                  ) {
  #ifdef EXTRA_COMP_LIB_NAME
                // Unload the library.
                ReleaseExtraCompression(hInstance);
  #endif
            } else {
                bSponsors=FALSE;
            }
        }
#endif
    } else {
        bSponsors = FALSE;
    }

    return;
}

int ExecCommand(LPSTR lpszCommand) 
{
	PROCESS_INFORMATION pf;
	STARTUPINFO sf;
	DWORD dwExitCode = 1;

	::ZeroMemory(&pf, sizeof(pf));
	::ZeroMemory(&sf, sizeof(sf));

	if (bPostStatus) {
	    SetJavaUpdateStringKey(NULL, REG_JUPDATE_COUNTRY, szCountry);
	    SetJavaUpdateStringKey(NULL, REG_JUPDATE_POSTSTATUSURL, szPostStatusURL);
	    SetJavaUpdateStringKey(NULL, REG_JUPDATE_METHOD, szMethod);
	}
	else {
	    SetJavaUpdateStringKey(NULL, REG_JUPDATE_POSTSTATUSURL, (LPTSTR)NULL);
	}
	if(::CreateProcess(NULL, lpszCommand, NULL, NULL, FALSE, 0, NULL, NULL, &sf, &pf)) {
		::WaitForSingleObject(pf.hProcess, INFINITE);
		GetExitCodeProcess(pf.hProcess, &dwExitCode);
		::CloseHandle(pf.hProcess);
	}

	return dwExitCode;
}

//int APIENTRY WinMain
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
	             LPTSTR     lpCmdLine,
                     int       nCmdShow)
{

    lpCmdLine = GetCommandLine(); 
    gLangid = DetectLocale();
    int langId=0;
    TCHAR transform[MAX_PATH] = {NULL};
    if (IsThisValidLang(gLangid)){
        langId=gLangid;
        char bufr [4];
        itoa (gLangid,bufr,10);
        wsprintf(transform, "jre%s.MST", bufr);
    }
    TCHAR command[BUFFER_SIZE];
    TCHAR szExecutableName[MAX_PATH], szExecutableNameCommand[BUFFER_SIZE];
    TCHAR szCabName[MAX_PATH] = {0} ;
    command[0] = NULL;
    int iReturn = 1;
    BOOL bSilentInstall = FALSE;
    BOOL bStatic = FALSE;
    BOOL bOEMUPDATE = FALSE;
    BOOL b64bit = FALSE;
    char * langMst;
    TCHAR szMstDir[MAX_PATH] = {NULL};
    bool transformExtracted=FALSE;
     //reset the installstats registry key
     SetInstallStatsKey(NULL);
     lstrcpy(szCountry, "YY");
     lstrcpy(szMethod, INSTALLMETHOD_OFFLINE);
     
     //Get the Old Version if there is one
    TCHAR oldVersion[128] = {NULL};
    if (!GetLatestJreKey(oldVersion, TRUE)) {
        //GetLatestJreKey doesn't change oldVersion if it doesn't exist, so no need to set to NULL again, just move on (SendHeadRequest will set to XX)
    }
    
    DWORD AuVersionSize = 128;
    TCHAR szProteusOldVersion[128]= {NULL};
    //Default Au return code to AU_NOT_INSTALLED value
    int nAuRet = AU_NOT_INSTALLED;
    
    //Get the old AU version if there is one
    if( ! GetAuVersion(szProteusOldVersion, AuVersionSize) ) {
        //if no old version was found, set old version to XX
        lstrcpy(szProteusOldVersion, "XX");
        logit("No Old AU Version found\n");
    }

#ifdef ARCH
     if (lstrcmpi(ARCH, "amd64") == 0) {
	    b64bit = TRUE;
        }
#endif

#ifdef SPONSOR
    lstrcpy(szMethod, INSTALLMETHOD_OFFLINE_SPONSOR);
    //Enterprise Registry DWORD value set to '0' 
    //Or SPONSORS=0 passed to this program.
    DWORD dwCount=sizeof(DWORD), dwVal = 1;
    GetJavaSoftKey("SPONSORS", (LPSTR)&dwVal, dwCount);
    if (dwVal == 0) bSponsors = FALSE;

#endif

    for (int i=1; i< __argc; i++) {
#ifdef SPONSOR
        if (lstrcmpi(__argv[i], "SPONSORS=0") == 0) {
	    bSponsors = FALSE;
        } else
#endif
        if (lstrcmpi(__argv[i], "STATIC=1") == 0)  {
            bStatic = TRUE;
        }
        if (lstrcmpi(__argv[i], "OEMUPDATE=1") == 0)  {
            bOEMUPDATE = TRUE; 
        }
        else if (strstr(__argv[i], "/lang=") != NULL) {
            LANGID lang = (LANGID) atoi(strstr(__argv[i], "=")+1);
            if (IsThisValidLang(lang)){
                gLangid = lang;
                langMst = strtok (__argv[i],"=");
                while (langMst != NULL){
                    wsprintf(transform, "jre%s.MST", langMst);
                    langId=atoi(langMst);
                    langMst = strtok (NULL, "=");
                }
            }
        }
        else if (strstr(__argv[i], "COUNTRYOVERRIDE=") != NULL) {
            lstrcpyn(szCountryOverride, (strstr(__argv[i], "=")+1), sizeof(szCountryOverride));
            SetJavaUpdateStringKey(NULL, REG_JUPDATE_COUNTRY, szCountryOverride);
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
        //The argument is either /x
        else {
            lstrcat(command, __argv[i]);
            lstrcat(command, " ");
        }
    }

    TCHAR szMessage[1024] = {NULL};

    if (!IsWIVerValid(NULL)){ 
      ::LoadString(hInstance, IDS_ERROR_WI20, szMessage, 1024);
      ::MessageBox(NULL, szMessage, "", MB_OK|MB_ICONERROR);
      return -2;
    }

    TCHAR szTitle[256] = {NULL};
    TCHAR szOnlyFirstInstallerCommandOptions[256] = {NULL};

    WhatsInstalled what_is_installed;

    if (!bStatic) {  // figure out what we are doing for a requested Consumer install

        if (what_is_installed.IsSameVersionStaticInstalled()) {        // if same version installed as static
            // keep static installed
            bStatic = TRUE;

        } else if (what_is_installed.IsConsumerInstalled()) {
            if (what_is_installed.IsSameVersionConsumerInstalled()) {  // installed as consumer,
                //same version as the new release
                // MSI will prompt if want to reinstall.
                lstrcat(szOnlyFirstInstallerCommandOptions, " REINSTALLMODE=vaums REINSTALL=ALL");
            } else if (what_is_installed.IsNewerConsumerVersionInstalled()) {
                // already a newer Consumer version installed
                // interpret a request to install this older version as a request to install Static

                bStatic = TRUE;
            } else {       // same or newer version not already installed

                // uninstall older consumer release / install consumer release
                lstrcat(szOnlyFirstInstallerCommandOptions, " REMOVEEXISTING=1");
            }
        } else {       // same or later version not already installed

            // install consumer release
        }
    }

    if (bStatic) {    // bStatic
        if (what_is_installed.IsSameVersionInstalled()) {   // if already have same version
            if (what_is_installed.IsSameVersionStaticInstalled()) {        // if installed as static
                // prompt if want to reinstall.
                // The MSI should do that.
                    lstrcat(szOnlyFirstInstallerCommandOptions, " REINSTALLMODE=vaums REINSTALL=ALL");
            }
            else { // same version consumer installed
                // have static MSI uninstall same version consumer and install current version Static
                lstrcat(szOnlyFirstInstallerCommandOptions, " REMOVEEXISTING=1");
            }
        }
        // install static
    }


    GetSingleMSIFileNames(szExecutableName, b64bit);

    TCHAR szLocalDir[MAX_PATH] = {NULL};
    TCHAR szUserShellFolder[MAX_PATH] = {0};
    GetUserShellFolder(szUserShellFolder); 

    {
       
	if (b64bit) {
	  wsprintf(szCabName, "%s\\Sun\\Java\\%s%s_x64\\Data1.cab", szUserShellFolder, BUNDLE, VERSION);
	  wsprintf(szLocalDir, "%s\\Sun\\Java\\%s%s_x64\\", szUserShellFolder, BUNDLE, VERSION);
          wsprintf(szMstDir, "%s\\Sun\\Java\\%s%s_x64\\%s", szUserShellFolder, BUNDLE, VERSION,transform);  
	}
	else {
	  wsprintf(szCabName, "%s\\Sun\\Java\\%s%s\\Data1.cab", szUserShellFolder, BUNDLE, VERSION);
          wsprintf(szLocalDir, "%s\\Sun\\Java\\%s%s\\", szUserShellFolder, BUNDLE, VERSION);
          wsprintf(szMstDir, "%s\\Sun\\Java\\%s%s\\%s", szUserShellFolder, BUNDLE, VERSION,transform);
	}
    }

    if (ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_CAB), "JAVA_INSTALLER", szCabName) 
        && ( bStatic ? ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_STATIC),   "JAVA_INSTALLER", szExecutableName)
                     : ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_CONSUMER), "JAVA_INSTALLER", szExecutableName) )) {

        TCHAR szXmlCmdLine[BUFFER_SIZE] = {NULL};
        TCHAR szTransformPath[MAX_PATH] = {NULL};

	GetOptionsFromXmlIfOnline(hInstance, szXmlCmdLine, szLocalDir, szTransformPath);

	if (!GetWIPath(szPath, sizeof(szPath)))
	    return -3;
	if (bSilentInstall) {
	    if (strstr(command, "/qn") == 0) lstrcat(command, " /qn");
	}

	//Set registry keys to be used by Download Initated & complete pings posting
	//And also pass in the install method, so it can be passed back to
	//SendHeadReaquest. This is also used by msi code, notto post 'di' & 'dc'
	//pings if it's from offline

	if (bPostStatus) {
	    lstrcat(command, " METHOD=");
	    lstrcat(command, szMethod);
	    lstrcat(command, " ");
	}
    switch (langId) {
         case 1028:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1028_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1031:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1031_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1034:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1034_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1036:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1036_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1040:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1040_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1041:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1041_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1042:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1042_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1046:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1046_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 1053:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_1053_MST), "JAVA_INSTALLER", szMstDir);
             break;
         case 2052:
             transformExtracted=ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_INSTALLER_2052_MST), "JAVA_INSTALLER", szMstDir);
             break;
         default:
             break;
    }
#ifdef SPONSOR
	if (!bSponsors) 
#endif
	{
	  if (gLangid != LANGID_ENGLISH)  {
	    // language transform only
	    wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\" TRANSFORMS=%s %s",
		     szPath, szExecutableName, transform, command);
	  } else  {
	      // else no transforms
	      wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\" %s",
		    szPath, szExecutableName, command);
          }
	}
#ifdef SPONSOR
	else {
    	    //SPONSOR enabled, add TRANSFORMS=<sponsor transforms>
    	    //country is added to as COUNTRY=<> to options.
            //append cmdline options to xml file options

            ExtractSponsorDLL(hInstance);

	    wsprintf(szExecutableNameCommand,
		"\"%s\" /i \"%s\" TRANSFORMS=\"%s\" PREFERENCEORDER=%s %s %s",
		szPath, szExecutableName, szTransformPath, szPreference,
		szXmlCmdLine, command);
	}

#endif

        TCHAR szOnlyFirstCommandLine[1024] = {NULL};

        wsprintf(szOnlyFirstCommandLine, "%s %s", szExecutableNameCommand, szOnlyFirstInstallerCommandOptions);

	iReturn = ExecCommand(szOnlyFirstCommandLine);
	//Check if it's the reinstall case. If so, then relaunch the same cmd
	TCHAR buf[512];
	lstrcpy(buf, "");
	GetJavaSoftKey(REG_INSTALLSTATS_STATUS, buf, sizeof(buf));
	if (lstrcmp(buf, "reinstall") == 0) {
            SetInstallStatsKey(NULL);
	    iReturn = ExecCommand(szExecutableNameCommand);
	}

	//???????? Should we delete This???  ::DeleteFile(szExecutableName);

        RunStuffPostInstall();

        //only attempt AU install if JRE install was successful and NOT 64-bit
        if(!b64bit && iReturn == 0){
            //Install Auto Update 
            //Extract au.msi
            TCHAR szAppDataPath[MAX_PATH] = {0};
            wsprintf(szAppDataPath,"%s\\Sun\\Java\\AU\\",szUserShellFolder);
            CreateDirectory(szAppDataPath, NULL);
            wsprintf(szExecutableName, "%sau.msi", szAppDataPath);
            wsprintf(szCabName, "%sau.cab", szAppDataPath);
            ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_AUINSTALLERCAB),   "JAVA_INSTALLER", szCabName);
            if(ExtractFileFromResource(hInstance, MAKEINTRESOURCE(IDP_AUINSTALLER),   "JAVA_INSTALLER", szExecutableName)){
                
                 // Before installing the bundled AU msi, uninstall any older version first
                 RemoveAnyOlderAU2X();
                
                 if(bOEMUPDATE){
                     wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\" ALLUSERS=1 OEMUPDATE=1 /qn",szPath, szExecutableName);
                 }else{
#ifdef J4B
                     wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\"DISABLE=1 ALLUSERS=1 /qn",szPath, szExecutableName);
#else
                     wsprintf(szExecutableNameCommand, "\"%s\" /i \"%s\" ALLUSERS=1 /qn",szPath, szExecutableName);
#endif
                 }
	         nAuRet = ExecCommand(szExecutableNameCommand);

             //Register jre to auto-update
             char CommonDir[MAX_PATH] = {0};//store the location of AppData directory

             if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES_COMMON, NULL, 0, CommonDir)))
	         {
		            wsprintf(szExecutableNameCommand, "%s\\Java\\Java Update\\jaureg.exe -r jre %s", CommonDir, FULLVERSION);
                    ExecCommand(szExecutableNameCommand);
	         }

            }
       } // end of if (!b64 bit)
       
       //Send InstallComplete ping
        //For offline installer, InstallerVer is always the same as FCSVer (FULLVERSION)
        SendHeadRequest(szPostStatusURL, szMethod, STATE_INSTALL_COMPLETE, szCountry, FULLVERSION, oldVersion, FULLVERSION, 0, iReturn, nAuRet, szProteusOldVersion, BUNDLED_AUVERSION);
        
        //remove the JreMetrics info for OEMUPDATE installs
        if( bOEMUPDATE ) {
            RemoveJreMetrics();
        }
    }  
    //Delete InstallStatus, Visid & method registry keys at the end of
    //installation
    SetInstallStatsKey(NULL);
    SetJavaUpdateStringKey(NULL, REG_JUPDATE_VISID, NULL);
    SetJavaUpdateStringKey(NULL, REG_JUPDATE_METHOD, NULL);

    return iReturn;
}
