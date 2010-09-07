/*
 * @(#)DownloadDialog.cpp   1.46 04/10/21
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// DownloadDialog.cpp : Implementation of CDownloadDialog
//
// By Stanley Man-Kit Ho
//

#include "stdafx.h"
#include <atlhost.h>
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <urlmon.h>
#include <wininet.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include "DownloadDialog.h"
#include "WelcomeDialog.h"
#include "jinstall.h"
#include "XmlParser.h"
#include "Locale.h"
#include "PluginVersion.h"
#include "WrapperUtils.h"
#include "UpdateConf.hpp"
#include "DownloadFile.h"
#include "RetrySplash.h"
#include "UserProfile.h"

#define XML_JUPDATE_ELEMENT "java-update"
#define XML_INFORMATION_ELEMENT "information"
#define XML_VERSION_ATTRIBUTE "version"
#define XML_VERSION_ELEMENT "version"
#define XML_LANGUAGE_ATTRIBUTE "xml:lang"
#define XML_MSI_URL_ELEMENT "msi-url"
#define XML_OPTIONS_ELEMENT "online-options"
#define XML_JDK_OPTIONS_ELEMENT "jdk-options"
#define XML_UPDATE_OPTIONS_ELEMENT "options"
#define XML_PREFERENCE_ENGINE_ELEMENT "sponsor-preference"
#define XML_IGNORE_NBI_ELEMENT "ignore-nbi"
//set to the url of InstallStats servlet. The status information is posted, if 
//value is non zero. Default is NOT to post, in the absence of this element.
#define XML_POSTSTATUS_ELEMENT "post-status"
//Server name for CountryLookup
#define XML_CNTRYLOOKUP_ELEMENT "cntry-lookup"
#define SPONSOR_CMDLINE_OPTION "SPWEB="

#define XML_VAUXNAME_ELEMENT "installaux-name"
#define XML_VPRELOADURL_ELEMENT "preload-url"
#define XML_VRUNVECTOR_ELEMENT "run-url"
#define XML_VMINOS_ELEMENT "min-os"
#define XML_VMINPSPEED_ELEMENT "min-processor-speed"
#define XML_VMINMEMORY_ELEMENT "min-memory"
#define XML_VLOCALES_ELEMENT "allowable-locales"
#define XML_VGEOS_ELEMENT "allowable-geos"

/////////////////////////////////////////////////////////////////////////////
// CDownloadDialog

CDownloadDialog::CDownloadDialog()
{
    m_ulProgress = 0;
    m_ulProgressMax = 0;
    m_iProgressFactor = 0;
    m_iMaxProgressFactor = 1;
    m_WindowCaption = 0;
    m_WindowText = 0;
    m_bXMLFile = FALSE;
    m_bUserCancelled = FALSE;
    m_bError = FALSE;
    m_bAltInstalldir = FALSE;
    m_bShowWelcome = FALSE;
    m_bShowSplash = FALSE;
    m_bPIP = FALSE;
    
    m_hCancelEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hDownloadThreadExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hVectorParsingEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    // Load up commctrl.dll
    InitCommonControls();
}


CDownloadDialog::~CDownloadDialog()
{
    ::CloseHandle(m_hCancelEvent);
    ::CloseHandle(m_hDownloadThreadExitEvent);
}



//=--------------------------------------------------------------------------=
// CDownloadDialog::OnInitDialog
//=--------------------------------------------------------------------------=
// Message handler for WM_INITDIALOG
//
// Parameters:
//  uMsg        Windows Message
//  wParam      WPARAM
//  lParam      LPARAM
//  bHandled    FALSE if not handled
//
// Output:
//  LRESULT     
//
// Notes:
//
LRESULT CDownloadDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    // Set timer
    SetTimer(iTimerID, 500);

    // Create a new thread for download
    DWORD dwThreadId = NULL;
    ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) DownloadThreadProc, this, 0, &dwThreadId);
    
    if (GetShowSplash()){
        HWND hWnd = (HWND) lParam;
        CRetrySplashScreen splash;
        splash.Create(hWnd);
        Sleep(3000);
        splash.DestroyWindow();
    }    
    
    if (GetShowWelcome()){
        
        // Wait for Vector info to be parsed before displaying any dialogs...timeout after 30 seconds
        WaitForSingleObject(m_hVectorParsingEvent, 30000);
    
        // Display the License Agreement Dialog
        CWelcomeDialog dlgWelcome;
    
        // Propogate PIP info for the Welcome dialog
        if (GetIsPIP()){
            dlgWelcome.setIsCheckboxDisabled(TRUE);
        }
    
        // Kill all threads and clean up if the user did not accept the License Agreement
        if (dlgWelcome.DoModal() != IDOK){
                
            // Disable window first to avoid any keyboard input
            EnableWindow(FALSE);
            m_bUserCancelled = TRUE;
        
            // Signal another thread to stop downloading
            SetEvent(m_hCancelEvent);
        
            // Wait for the download thread to complete
            AtlWaitWithMessageLoop(m_hDownloadThreadExitEvent);
        
            // Kill timer and free resources
            KillTimer(iTimerID);
            FreeGDIResources();
        
            // Destroy dialog
            EndDialog(0);
        
            return 0;
        }
    
        // Pass alt installdir bool to CDownloadDialog class
        if (dlgWelcome.getIsAltInstallDir()){
            SetIsAltInstallDir(TRUE);
        }
        
    }
    
    // If the download thread has not exited, display the dialog
    if (WAIT_OBJECT_0 != WaitForSingleObject(m_hDownloadThreadExitEvent, 0)) {

        m_hNormalFont = NULL;
        m_hMediumFont = NULL;
        m_hBoldFont = NULL;
        m_hMemDC = NULL;  
        m_hDlgBrush = CreateSolidBrush(RGB(231, 229, 213));

        // Allows for browser open via the 'java.com' hypertext
        HWND hWnd = GetDlgItem(IDC_MARKETING_INFO2);
        DWORD style = ::GetWindowLong(hWnd, GWL_STYLE);
        ::SetWindowLong(hWnd, GWL_STYLE, style | SS_NOTIFY);

        // Set colors for the status bar
        hWnd = GetDlgItem(IDC_DOWNLOAD_PROGRESS);
        ::PostMessage(hWnd, PBM_SETBKCOLOR, NULL, (LONG) RGB(255, 255, 255)); 
        ::PostMessage(hWnd, PBM_SETBARCOLOR, NULL, (LONG) RGB(50, 205, 50));

    }

    return 1;  // Let the system set the focus

}


//=--------------------------------------------------------------------------=
// CDownloadDialog::OnOK
//=--------------------------------------------------------------------------=
// Message handler for WM_COMMAND with IDOK
//
// Parameters:
//  wNotifyCode Notify Code
//  wID     ID of control
//  hWndCtl     HWND of control
//  bHandled    FALSE if not handled
//
// Output:
//  LRESULT     
//
// Notes:
//
LRESULT CDownloadDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);

    // Kill timer
    KillTimer(iTimerID);

    // Signal another thread to stop downloading
    SetEvent(m_hCancelEvent);

    // Wait for the download thread to complete
    AtlWaitWithMessageLoop(m_hDownloadThreadExitEvent);

    FreeGDIResources();

    // Destroy dialog
    EndDialog(wID);

    return 0;
}



//=--------------------------------------------------------------------------=
// CDownloadDialog::OnCancel
//=--------------------------------------------------------------------------=
// Message handler for WM_COMMAND with IDCANCEL
//
// Parameters:
//  wNotifyCode Notify Code
//  wID     ID of control
//  hWndCtl     HWND of control
//  bHandled    FALSE if not handled
//
// Output:
//  LRESULT     
//
// Notes:
//
LRESULT CDownloadDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);
    m_bUserCancelled = TRUE;

    // Signal another thread to stop downloading
    SetEvent(m_hCancelEvent);

    // Wait for the download thread to complete
    AtlWaitWithMessageLoop(m_hDownloadThreadExitEvent);

    // Kill timer
    KillTimer(iTimerID);

    FreeGDIResources();

    // Destroy dialog
    EndDialog(wID);

    return 0;
}


//=--------------------------------------------------------------------------=
// CDownloadDialog::OnTimer
//=--------------------------------------------------------------------------=
// Message handler for WM_TIMER
//
// Parameters:
//  uMsg        Windows Message
//  wParam      WPARAM
//  lParam      LPARAM
//  bHandled    FALSE if not handled
//
// Output:
//  LRESULT     
//
// Notes:
//
LRESULT CDownloadDialog::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (iTimerID == (int)wParam) {
    __try {
        m_csDownload.Lock();

        if (m_ulProgress && m_ulProgressMax) {
        HWND hProgressWnd = GetDlgItem(IDC_DOWNLOAD_PROGRESS);
        HWND hStatusWnd = GetDlgItem(IDC_DOWNLOAD_STATUS);
        TCHAR szBuffer[BUFFER_SIZE];
        TCHAR szTimeBuffer[BUFFER_SIZE];
        time_t currentTime;
        time(&currentTime);

        double elapsed_time = difftime(currentTime, m_startTime);
        double remain_time = (elapsed_time / m_ulProgress) 
                  * (m_ulProgressMax - m_ulProgress);

        ::PostMessage(hProgressWnd, PBM_SETPOS, 
                   (WPARAM) (m_ulProgress * 100 
                      / m_ulProgressMax), NULL);

        int hr = 0, min = 0;
        if (remain_time > 60 * 60) {
            hr = int(remain_time / (60 * 60));
            remain_time = remain_time - hr * 60 * 60;
        }
        if (remain_time > 60) {
            min = int(remain_time / 60);
            remain_time = remain_time - min * 60;
        }
        if (hr > 0) {
          // Progress status text           
          if (hr > 1)
              LoadString(_Module.GetResourceInstance(), IDS_HOURSMINUTESECOND, szTimeBuffer, BUFFER_SIZE);
          else
             LoadString(_Module.GetResourceInstance(), IDS_HOURMINUTESECOND, szTimeBuffer, BUFFER_SIZE);
          sprintf(szBuffer, szTimeBuffer, hr, min, remain_time);
        }
        else {
          if (min > 0) {
              // Progress status text           
              LoadString(_Module.GetResourceInstance(), IDS_MINUTESECOND, szTimeBuffer, BUFFER_SIZE);
              sprintf(szBuffer, szTimeBuffer, min, remain_time);
          }
          else {
              // Progress status text           
              LoadString(_Module.GetResourceInstance(), IDS_SECOND, szTimeBuffer, BUFFER_SIZE);
              sprintf(szBuffer, szTimeBuffer, remain_time);
          }
        }
        
        // Update status message
        ::SetWindowText(hStatusWnd, szBuffer);

        if (m_WindowCaption != 0) {
            // Update window caption
            LoadString(_Module.GetResourceInstance(), m_WindowCaption, 
                szBuffer, BUFFER_SIZE); 
            ::SetWindowText(m_hWnd, szBuffer);

        }
        }
    }
    __finally
    {
       m_csDownload.Unlock();
    }
    }
    
    return 0;
}

LRESULT CDownloadDialog::OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LONG)m_hDlgBrush;
}

// this message is sent each time a static control is drawn.
// we get the Control ID and then set background color and font
// as appropriate for that control.
LRESULT CDownloadDialog::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HDC hdc = (HDC) wParam;
    HWND hwnd = (HWND) lParam;

    int DlgCtrlID = ::GetDlgCtrlID(hwnd);

    if ((DlgCtrlID == IDC_DOWNLOAD_HEADER) || (DlgCtrlID == IDC_MARKETING_INFO1) || (DlgCtrlID == IDC_MARKETING_INFO2) || (DlgCtrlID == IDC_MARKETING_INFO3) || (DlgCtrlID == IDC_MARKETING_INFO4) || (DlgCtrlID == IDC_DOWNLOAD_STATUS)){
        
        // Set medium font for all text resources
        if (m_hMediumFont == NULL){
            m_hMediumFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 10, FW_MEDIUM, FALSE);
        }
        ::SelectObject(hdc, m_hMediumFont);
        
        // Set background color to default gray for all text resources
        SetBkColor(hdc, RGB(231, 229, 213));
        
        // Set java.com text color = blue
        if (DlgCtrlID == IDC_MARKETING_INFO2){
            ::SetTextColor(hdc, RGB(255, 0, 0));
        }
        
        return (LONG)m_hDlgBrush;
        
    }

    SetBkMode(hdc, TRANSPARENT);
    return (LONG)m_hDlgBrush;

}


//=--------------------------------------------------------------------------=
// CDownloadDialog::OnStartBinding
//=--------------------------------------------------------------------------=
// Called when download is started
//
// Parameters:
//
// Output:
//  HRESULT
//
// Notes:
//
STDMETHODIMP CDownloadDialog::OnStartBinding()
{
    __try
    {
    m_csDownload.Lock();

    time(&m_startTime);
    }
    __finally
    {
    m_csDownload.Unlock();
    }

    return S_OK;
}


//=--------------------------------------------------------------------------=
// CDownloadDialog::OnStartInstallerBinding
//=--------------------------------------------------------------------------=
// Called when installer download is started
//
// Parameters:
//
// Output:
//  HRESULT
//
// Notes:
//
STDMETHODIMP CDownloadDialog::OnStartInstallerBinding()
{
    __try
    {
    m_csDownload.Lock();

    // Double progress bars
    m_iProgressFactor = 0;
    m_iMaxProgressFactor = 2;
    }
    __finally
    {
    m_csDownload.Unlock();
    }

    return S_OK;
}


//=--------------------------------------------------------------------------=
// CDownloadDialog::OnStopBinding
//=--------------------------------------------------------------------------=
// Called when download is stopped
//
// Parameters:
//
// Output:
//  HRESULT
//
// Notes:
//
STDMETHODIMP CDownloadDialog::OnStopBinding()
{
    return S_OK;
}


//=--------------------------------------------------------------------------=
// CDownloadDialog::OnProgress
//=--------------------------------------------------------------------------=
// Called when download is in progress
//
// Parameters:
//
// Output:
//  HRESULT
//
// Notes:
//
STDMETHODIMP CDownloadDialog::OnProgress(ULONG ulProgress, ULONG ulProgressMax)
{
    __try
    {
    m_csDownload.Lock();

    m_ulProgress = m_iProgressFactor * ulProgressMax + ulProgress;
    m_ulProgressMax = m_iMaxProgressFactor * ulProgressMax;
    }
    __finally
    {
    m_csDownload.Unlock();
    }

    return S_OK;
}

//=--------------------------------------------------------------------------=
// CDownloadDialog::DownloadThreadProc
//=--------------------------------------------------------------------------=
// Thread procedure for downloading installer
//
// Parameters:
//  lpParameter  Thread parameter
//
// Output:
//  DWORD       
//
// Notes:
//
DWORD WINAPI CDownloadDialog::DownloadThreadProc(LPVOID lpParameter)
{
    CDownloadDialog* pDlg = (CDownloadDialog *) lpParameter;

    TCHAR szCachedLocalFile[BUFFER_SIZE];

    BOOL bResumable = FALSE;
    BOOL bSuccess = FALSE;

    __try {
    pDlg->OnStartBinding();
    do {
        //if Config URL not set, use default msi location and skip this part
        if (lstrlen(pDlg->m_pszConfigURL) > 0 ) {
          // Download configuration file
          if (FAILED(pDlg->DownloadConfiguration(pDlg->m_pszConfigURL, szCachedLocalFile, BUFFER_SIZE)))
        break;        

          // Check if download has been cancelled
          if (WAIT_OBJECT_0 == WaitForSingleObject(pDlg->m_hCancelEvent, 0))
        break;
          
          // Parse configuration file
          if (FAILED(pDlg->ParseConfiguration(szCachedLocalFile, 
                                                pDlg->m_pszInstallerURL, 
                        pDlg->m_pszInstallerOptions, 
                        &bResumable)))
        break;
        }

        // Check if download has been cancelled
        if (WAIT_OBJECT_0 == WaitForSingleObject(pDlg->m_hCancelEvent, 0))
        break;

        //all installations have config file
        pDlg->m_WindowCaption = 0;
        pDlg->m_WindowText = 0;

        //Download the Open Office banner
        char * lang = ::GetLocaleStr();
        //Download the correct version of the open office banner
        if(lang != NULL){
            //temp buffer for open office banner url
            TCHAR pszOpenOfficeBannerUrl[BUFFER_SIZE] = {NULL};
            //temp buffer for open office banner save path 
            TCHAR pszOpenOfficeBannerPath[BUFFER_SIZE] = {NULL};
            //Get the local APPDATA path
            TCHAR szAppDataDir[MAX_PATH] = {NULL};
            GetUserShellFolder(szAppDataDir); 

            //copy the m_pszInstallerURL + sponsorname + locale + jpg extension string into buffer
            wsprintf(pszOpenOfficeBannerUrl, "%s/%s_%s.jpg", pDlg->m_pszInstallerURL,
                        SPONSOR_DEFAULT_STRING, lang);
            //copy the APPDATA + Sun\\Java\\jreVERSION\\SPONSOR_DEFAULT_STRING + .jpg string into buffer
            wsprintf(pszOpenOfficeBannerPath, "%s\\Sun\\Java\\jre%s\\%s.jpg",
                        szAppDataDir, VERSION, SPONSOR_DEFAULT_STRING);
            //download the jpeg in the directory location
            URLDownloadToFile(NULL, pszOpenOfficeBannerUrl, pszOpenOfficeBannerPath, 0, NULL);
            
            ////Com string 
            //CComBSTR bstrTemp;
            //// IDS_SPONSOR_BANNER_CHECKSUM checksum stored as resource in string table
            //bstrTemp.LoadString(IDS_SPONSOR_BANNER_CHECKSUM);
			TCHAR szHash[BUFFER_SIZE] = {NULL};
            ::LoadString(::GetModuleHandle(NULL), IDS_SPONSOR_BANNER_CHECKSUM, szHash, BUFFER_SIZE);
            //verify the checksum of the downloaded image and delete it if erroneous
            if(!verifySHA1(pszOpenOfficeBannerPath, szHash)){
                ::DeleteFile(pszOpenOfficeBannerPath);
            }
            
        }

            // Download Singlemsi with UI feedback
#ifdef SDK
            lstrcpy(pDlg->m_pszFileName, pDlg->m_pszInstallerURL);
            //if auth-proxy OR win2003, all installations have config file
            if (pDlg->IsSingleMsi()) {

              TCHAR singleMSIURL[1024], mstURL[1024];
              wsprintf(singleMSIURL, "%s/%s%s.msi", pDlg->m_pszInstallerURL, BUNDLE, VERSION);
              wsprintf(mstURL, "%s/%s", pDlg->m_pszInstallerURL, pDlg->m_pszMSTBaseFileName);
              if (FAILED(pDlg->DownloadFile(singleMSIURL, pDlg->m_pszSingleMSIFileName, bResumable, TRUE)))
                break;
              if(strstr(pDlg->m_pszMSTBaseFileName, "1033.MST") == NULL)
                 if (FAILED(pDlg->DownloadFile(mstURL, pDlg->m_pszMSTFileName, bResumable, TRUE)))
                 break;
            }
#else
        TCHAR msiURL[512], mstURL[512], pszBaseURL[1024];
        msiURL[0] = NULL;
        mstURL[0] = NULL;
        pszBaseURL[0] = NULL;

        wsprintf(msiURL, "%s/%s.msi", pDlg->m_pszInstallerURL, pDlg->m_szDownLoadVersion);
        pDlg->m_bSponsorEnabled = (pDlg->IsSponsorEnabled(pDlg->m_pszInstallerOptions, pszBaseURL)); 
            if (pDlg->m_bSponsorEnabled)
            wsprintf(mstURL, "%s/", pszBaseURL); 
            else
            wsprintf(mstURL, "%s/", pDlg->m_pszInstallerURL); 
            SetMSTFileNames(pDlg->m_bSponsorEnabled, pDlg->m_pszMSTCachedFile, mstURL);

            //check if files are predownloaded by javaupdate or earlier downld
            if (IsFilesDownloaded(pDlg->m_pszSingleMSIFileName, pDlg->m_pszMSTCachedFile, NULL))
            {
                bSuccess=TRUE;
                break;
            }

        if (FAILED(pDlg->DownloadFile(msiURL, pDlg->m_pszSingleMSIFileName, bResumable, TRUE)))
            break;       
            if ((lstrlen(pDlg->m_pszMSTCachedFile) > 1) &&
          (FAILED(pDlg->DownloadFile(mstURL, pDlg->m_pszMSTCachedFile, bResumable, TRUE))))
            {
            break;
        }
#endif
        // Check if download has been cancelled
        if (WAIT_OBJECT_0 == WaitForSingleObject(pDlg->m_hCancelEvent, 0))
        break;

        // Installer has been downloaded
        bSuccess = TRUE;
    } while (0);

    // Post message to main thread to dis-miss dialog if users haven't hit cancel
    //
    if (WAIT_OBJECT_0 != WaitForSingleObject(pDlg->m_hCancelEvent, 0)) {
        if (bSuccess)
        ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, NULL);
        else {
        ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDCANCEL, NULL);
                pDlg->m_bError = TRUE;
            }
    }
    }
    __finally {
    pDlg->OnStopBinding();
    // Signal main thread
    ::SetEvent(pDlg->m_hDownloadThreadExitEvent);   
    }

    return 0;
}
//=--------------------------------------------------------------------------=
// CDownloadDialog::DownloadConfiguration
//=--------------------------------------------------------------------------=
// Download configuration file
//
// Parameters:
//  szURL       URL of configuration
//  szConfigFile    Local path to downloaded configuration file
//  dwBufLength Buffer length
//
// Output:
//  HRESULT
//
// Notes:
//
HRESULT CDownloadDialog::DownloadConfiguration(const TCHAR* szURL, TCHAR* szConfigFile, DWORD dwBufLength)
{
    szConfigFile[0] = NULL;

    // Since the config file is relatively small - use
    // URL Moniker to download it.
    //

    // Obtain temp file name for config file
    ::GetTempPath(dwBufLength, szConfigFile);
    lstrcat(szConfigFile, "jinstall.cfg");

    // If this installation is a re-try (RunOnce), copy the config file
    // from %APPDATA% instead of downloading via the URL
    if (GetShowSplash()){
        
        TCHAR szAppDataDir[BUFFER_SIZE] = {NULL};
        if (GetUserShellFolder(szAppDataDir)){
            TCHAR szAppDataCfg[BUFFER_SIZE] = {NULL};
            wsprintf(szAppDataCfg, "%s\\Sun\\Java\\%s", szAppDataDir, "jinstall.cfg");
            if (::CopyFile(szAppDataCfg, szConfigFile, FALSE) != 0){
                return S_OK;
            }
        }
    
    }
    
    // Silent download: No UI feedback, no resumable download
    m_bXMLFile = TRUE;
    HRESULT hr = DownloadFile(szURL, szConfigFile, FALSE, FALSE);
    m_bXMLFile = FALSE;

    if (FAILED(hr))
    {
    TCHAR szErrorMsg[BUFFER_SIZE], szErrorCaption[BUFFER_SIZE], szMsg[BUFFER_SIZE];

    ::LoadString(_Module.GetResourceInstance(), IDS_DOWNLOADERROR_MSG, szErrorMsg, BUFFER_SIZE);
    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_CAPTION, szErrorCaption, BUFFER_SIZE);

    wsprintf(szMsg, szErrorMsg, szURL);

    // Display error messages
    if (!IsSilentInstall()) MessageBox(szMsg, szErrorCaption, MB_ICONERROR | MB_OK);    
    }

    return hr;
}

//=--------------------------------------------------------------------------=
// CDownloadDialog::DownloadFile
//=--------------------------------------------------------------------------=
// Download installer
//
// Parameters:
//  szURL       URL of file
//  szLocalFile Local path to downloaded file
//  bResumable  Resumabe download enabled
//
// Output:
//  HRESULT     
//
// Notes:
//
HRESULT CDownloadDialog::DownloadFile(const TCHAR* szURL, const TCHAR* szLocalFile, BOOL bResumable, BOOL bUIFeedback)
{
    HINTERNET hOpen = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwDownloadError = 0;
    BOOL bHttps = FALSE;

    __try {

#ifndef SDK
        TCHAR szURLFullPath[BUFFER_SIZE];
    szURLFullPath[0] = NULL;
    lstrcpy(szURLFullPath, szURL);
#endif

    // If file is from local URL, simply copy it
    if (strstr(szURL, TEXT("file://")) != NULL) {
        URLDownloadToFile(NULL, szURL, szLocalFile, NULL, NULL);

        dwDownloadError = 0;
        __leave;
    }
    else if (strstr(szURL, TEXT("https://")) != NULL) bHttps = TRUE;

    HWND hProgressInfo = NULL;
    TCHAR szStatus[BUFFER_SIZE];
    
    if (bUIFeedback) {
        hProgressInfo = GetDlgItem(IDC_DOWNLOAD_PROGRESS);

        ::LoadString(_Module.GetResourceInstance(), IDS_DOWNLOAD_STATUS_OPENING, szStatus, BUFFER_SIZE);
        ::SetWindowText(hProgressInfo, szStatus);
    }

    // Open Internet Call
    TCHAR szHostName[BUFFER_SIZE], szUrlPath[BUFFER_SIZE], szExtraInfo[BUFFER_SIZE];
    GetJInstallUserAgent(szExtraInfo, BUFFER_SIZE);
    hOpen = ::InternetOpen(szExtraInfo, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);

    if (hOpen == NULL) {
        dwDownloadError = 1;
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


    // Update status
    if (bUIFeedback) {
        ::LoadString(_Module.GetResourceInstance(), IDS_DOWNLOAD_STATUS_CONNECTING, szStatus, BUFFER_SIZE);
        ::SetWindowText(hProgressInfo, szStatus);
    }
    
    // Open Internet Connection
    hConnect = ::InternetConnect(hOpen, url_components.lpszHostName, url_components.nPort,
                     "", "", INTERNET_SERVICE_HTTP, NULL, NULL);

    if (hConnect == NULL) {
        dwDownloadError = 1;
        __leave;
    }   

    // Determine the relative URL path by combining 
    // Path and ExtraInfo
    char szURL[4096];

    if (url_components.dwUrlPathLength !=  0)
        lstrcpy(szURL, url_components.lpszUrlPath);
    else
        lstrcpy(szURL, "/");

    if (url_components.dwExtraInfoLength != 0)
        lstrcat(szURL, url_components.lpszExtraInfo);
#ifndef SDK
        GetTranId(NULL, szURL);
#endif

    BOOL bRetryHttpRequest = FALSE;
    int numberOfRetry = 0;
    long secondsToWait = 60;

    do {
        bRetryHttpRequest = FALSE;

        // Update status
        if (bUIFeedback) {
        ::LoadString(_Module.GetResourceInstance(), IDS_DOWNLOAD_STATUS_OPENING_HTTP, szStatus, BUFFER_SIZE);
        ::SetWindowText(hProgressInfo, szStatus);
        }

        // Make a HTTP GET request
        if (bHttps) {
          hRequest = ::HttpOpenRequest(hConnect, "GET", szURL, "HTTP/1.1", 
                     "", NULL,
                     INTERNET_FLAG_KEEP_CONNECTION | 
                     INTERNET_FLAG_DONT_CACHE|
                     INTERNET_FLAG_SECURE, 0);
        } else {
          hRequest = ::HttpOpenRequest(hConnect, "GET", szURL, "HTTP/1.1", 
                     "", NULL,
                     INTERNET_FLAG_KEEP_CONNECTION | 
                     INTERNET_FLAG_DONT_CACHE, 0);
        }

        if (hRequest == NULL) {
        dwDownloadError = 1;
        __leave;
        }   

        // Create or open existing destination file
        hFile = ::CreateFile(szLocalFile, GENERIC_WRITE, 0, NULL, 
                    OPEN_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);

            if (hFile == INVALID_HANDLE_VALUE) {
        dwDownloadError = 1;
        __leave;
        }   
        DWORD fileSize = GetFileSize(hFile, NULL);
       
        // Check if resumable download is enabled
        if (bResumable == FALSE) {
        // Start from scratch           
        fileSize = 0;
        }

        FILETIME tWrite;
        BOOL rangereq = FALSE;
            if ((fileSize != 0) && (fileSize != 0xFFFFFFFF) &&
        GetFileTime(hFile, NULL, NULL, &tWrite))
        {
            char szHead[100];
        SYSTEMTIME tLocal;
        char buf[INTERNET_RFC1123_BUFSIZE];

        FileTimeToSystemTime(&tWrite, &tLocal);
        InternetTimeFromSystemTime(&tLocal, INTERNET_RFC1123_FORMAT,
                    buf, INTERNET_RFC1123_BUFSIZE);
            sprintf(szHead, "Range: bytes=%d-\r\nIf-Range: %s\r\n", fileSize, buf);
        HttpAddRequestHeaders(hRequest, szHead, lstrlen(szHead),
             HTTP_ADDREQ_FLAG_ADD|HTTP_ADDREQ_FLAG_REPLACE);
            rangereq = TRUE;
        }

        // This is a loop to handle various potential error when the 
        // connection is made
        BOOL bCont = TRUE;


        // Update status
        if (bUIFeedback) {
        ::LoadString(_Module.GetResourceInstance(), IDS_DOWNLOAD_STATUS_SENDING_HTTP, szStatus, BUFFER_SIZE);
        ::SetWindowText(hProgressInfo, szStatus);
        }


        while ((FALSE == ::HttpSendRequest(hRequest, NULL, NULL, NULL, NULL))
            && bCont ) 
        {
        // We might have an invalid CA. Ask the 
        DWORD dwErrorCode = GetLastError();
        
        switch(dwErrorCode) {
            case ERROR_HTTP_TIMEOUT: 
            case ERROR_HTTP_NAME_NOT_RESOLVED: 
            case ERROR_HTTP_CANNOT_CONNECT: 
            {
            bCont = FALSE;
            if (!IsSilentInstall()) {
            TCHAR szCaption[BUFFER_SIZE];
            TCHAR szBuffer[BUFFER_SIZE];
            if (dwErrorCode == ERROR_HTTP_TIMEOUT)
                ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_REQUEST_TIMEOUT, szBuffer, BUFFER_SIZE);
            else if (dwErrorCode == ERROR_HTTP_NAME_NOT_RESOLVED)
                ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_SERVER_NOT_REACHABLE, szBuffer, BUFFER_SIZE);
            else
                ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_SERVER_NOT_REACHABLE, szBuffer, BUFFER_SIZE);

            ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_CAPTION, szCaption, BUFFER_SIZE);
            // Display the information dialog
            if (!IsSilentInstall()) ::MessageBox(m_hWnd, szBuffer, szCaption, MB_OK);
            }
            break;
            }
            case ERROR_INTERNET_INVALID_CA: 
            case ERROR_INTERNET_SEC_CERT_CN_INVALID:
            case ERROR_INTERNET_SEC_CERT_DATE_INVALID:
            case ERROR_INTERNET_HTTP_TO_HTTPS_ON_REDIR:
            case ERROR_INTERNET_INCORRECT_PASSWORD:
            case ERROR_INTERNET_CLIENT_AUTH_CERT_NEEDED:
            default:
            {
            // Unless the user agrees to continue, we just abandon now !
            bCont = FALSE;

            // Make sure to test the return code from InternetErrorDlg
            // user may click OK or Cancel. In case of Cancel, request
            // should not be resubmitted
            if (ERROR_SUCCESS == ::InternetErrorDlg(
                    m_hWnd, hRequest, 
                    dwErrorCode,
                    FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                    FLAGS_ERROR_UI_FLAGS_GENERATE_DATA |
                    FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS,
                    NULL))
                bCont = TRUE;
            }
        }
        } 

        if (bCont == FALSE) {
        // User has denied the request
        dwDownloadError = 1;
        __leave;
        }

        //
        // Read HTTP status code
        //
        DWORD dwErrorCode = GetLastError();
        DWORD dwStatus=0;
        DWORD dwStatusSize = sizeof(DWORD);
       
        if (FALSE == ::HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | 
                     HTTP_QUERY_STATUS_CODE, &dwStatus, &dwStatusSize, NULL))
        {
        dwErrorCode = GetLastError();
        }

        bCont = TRUE;

        while ((dwStatus == HTTP_STATUS_PROXY_AUTH_REQ || 
            dwStatus == HTTP_STATUS_DENIED) &&
            bCont)
        {

            DWORD dwErrorDlgReturn = ::InternetErrorDlg(m_hWnd, hRequest, dwErrorCode,  
                    FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                    FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS |
                    FLAGS_ERROR_UI_FLAGS_GENERATE_DATA,
                    NULL);
        if ((dwErrorDlgReturn == ERROR_CANCELLED) ||  ((dwErrorDlgReturn == ERROR_SUCCESS) && (dwStatus == HTTP_STATUS_PROXY_AUTH_REQ)))
        {
            bCont = FALSE;
        }
        else {
            ::HttpSendRequest(hRequest, NULL, 0, NULL, 0);

            // Reset buffer length          
            dwStatusSize = sizeof(DWORD);

            ::HttpQueryInfo(hRequest, HTTP_QUERY_FLAG_NUMBER | 
                    HTTP_QUERY_STATUS_CODE, &dwStatus, &dwStatusSize, NULL);
        }
        }

        if (dwStatus == HTTP_STATUS_OK || dwStatus == HTTP_STATUS_PARTIAL_CONTENT) 
        {

        // Determine content length, so we may show the progress bar
        // meaningfully
        //
        DWORD nContentLength = 0;
        DWORD nLengthSize = sizeof(DWORD);
        ::HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                &nContentLength, &nLengthSize, NULL);

        if (nContentLength <= 0) {
            // If can't estimate content length, estimate it 
            // to be 6MB
            nContentLength = 15000000;
        }   
        else if (rangereq && (fileSize != 0) &&
            (nContentLength == fileSize)) 
        {
            // If the file is already downloaded completely and then
            // we send a range request, the whole file is sent instead
            // of nothing. So avoid downloading again.
            // Some times return value is 206, even when whole file
            // is sent. So check if "Content-range:" is present in the reply
            char buffer[256];
            DWORD length = sizeof(buffer);

            if(!HttpQueryInfo(hRequest, HTTP_QUERY_CONTENT_RANGE, buffer, &length, NULL))
            {
            if(HttpQueryInfo(hRequest, HTTP_QUERY_LAST_MODIFIED, buffer, &length, NULL))
            {
                SYSTEMTIME systime;
                FILETIME filtime;
                InternetTimeToSystemTime(buffer, &systime, NULL);
                SystemTimeToFileTime(&systime, &filtime);
                if ((CompareFileTime(&tWrite, &filtime)) == 1) {
                    // no need to download
                    dwDownloadError = 0;
                    __leave;
                }
            }
            else {
                ::SetFilePointer(hFile, 0, 0, FILE_BEGIN);
                ::SetEndOfFile(hFile); // truncate the file
            }
            }
        }

        TCHAR szBuffer[8096];
        DWORD dwBufferSize = 8096;

        // Read from HTTP connection and write into 
        // destination file
        //
        DWORD nRead = 0;
        DWORD dwTotalRead = 0;
        BOOL bCancel = FALSE;
        
        if (dwStatus == HTTP_STATUS_PARTIAL_CONTENT) {
            // If we are using resumable download, fake
            // start time so it looks like we have begun 
            // the download several minutes again.
            //
            m_startTime = m_startTime - 100; 

            ::SetFilePointer(hFile, 0, 0, FILE_END); // seek to end
        }
        else {
            ::SetFilePointer(hFile, 0, 0, FILE_BEGIN);
            ::SetEndOfFile(hFile); // truncate the file
        }

        do {
            nRead=0;
        
            if (::InternetReadFile(hRequest, szBuffer, dwBufferSize, &nRead)) 
            { 
            if (nRead) {
                DWORD dwNumberOfBytesWritten = NULL;
    
                ::WriteFile(hFile, szBuffer, nRead, &dwNumberOfBytesWritten, NULL);
                        }

            dwTotalRead += nRead;
           
            if (bUIFeedback) {
                OnProgress(fileSize + dwTotalRead, fileSize + nContentLength);
            }

            // Check if download has been cancelled
            if (WAIT_OBJECT_0 == WaitForSingleObject(m_hCancelEvent, 0))
            {
                bCancel = TRUE;
                break;
            }       
            } 
            else {
            bCancel = TRUE;
            break;
            } 
        }
        while (nRead);


        if (bCancel) {
            // User has cancelled the operation or InternetRead failed
            // don't do return here, we need to cleanup
            dwDownloadError = 1;
            __leave;
        }
        }
        else if (dwStatus == 416 && (fileSize != 0) && 
            (fileSize != 0xFFFFFFFF))
        {
        // This error could be returned, When the full file exists 
        // and a range request is sent with range beyond filessize.
        // The best way to fix this is in future is, to send HEAD 
        // request and get filelength before sending range request. 
        dwDownloadError = 0;
        __leave;
        } else if (dwStatus >= 400 && dwStatus < 600) {
        TCHAR szCaption[BUFFER_SIZE];
        TCHAR szBuffer[BUFFER_SIZE];

                if (dwStatus == HTTP_STATUS_FORBIDDEN) {
#define EXPORT_DENIED_HTTP_STATUS_TEXT "Export Denied"
                    TCHAR sQuery[sizeof(EXPORT_DENIED_HTTP_STATUS_TEXT)+1];
                    DWORD dwLengthBufQuery = sizeof(sQuery);

                    if ( ::HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_TEXT, sQuery, &dwLengthBufQuery, NULL)
                          && (0 == lstrcmp(sQuery, EXPORT_DENIED_HTTP_STATUS_TEXT)) )  {
                        ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_EXPORT_FAILURE_CAPTION, szCaption, BUFFER_SIZE);
                        ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_EXPORT_FAILURE, szBuffer, BUFFER_SIZE);
                    } else {
                        ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_CAPTION, szCaption, BUFFER_SIZE);
                        ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_FORBIDDEN, szBuffer, BUFFER_SIZE);
                    }

                    // Display the information dialog
                    if (!IsSilentInstall()) {
                        ::MessageBox(m_hWnd, szBuffer, szCaption, MB_ICONERROR | MB_OK);
                    }
                } else if (dwStatus == HTTP_STATUS_SERVER_ERROR) {
            ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_SERVER_ERROR_CAPTION, szCaption, BUFFER_SIZE);
            ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_INTERNAL_ERROR, szBuffer, BUFFER_SIZE);

            // Display the warning dialog
                if (!IsSilentInstall() && ::MessageBox(m_hWnd, szBuffer, szCaption, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
            {
            bRetryHttpRequest = TRUE;
            }
        }
        else if (dwStatus == HTTP_STATUS_SERVICE_UNAVAIL) {
            if (numberOfRetry < 5) {
            // If the server is busy, automatically retry

            // We wait couple seconds before retry to avoid congestion
            for (long i = (long) secondsToWait; i >= 0; i--) {
                // Update status
                if (bUIFeedback) {
                char szBuffer[BUFFER_SIZE];
                ::LoadString(_Module.GetResourceInstance(), IDS_DOWNLOAD_STATUS_RETRY, szStatus, BUFFER_SIZE);
                wsprintf(szBuffer, szStatus, i);

                ::SetWindowText(hProgressInfo, szBuffer);
                }

                // Sleep 1 second
                ::Sleep(1000);
            }

            // We use a semi-binary backoff algorithm to 
            // determine seconds to wait
            numberOfRetry += 1;
            secondsToWait = secondsToWait + 30;
            bRetryHttpRequest = TRUE;

            continue;
            }
            else {
            ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_SERVER_BUSY_CAPTION, szCaption, BUFFER_SIZE);
            ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_SERVER_BUSY, szBuffer, BUFFER_SIZE);

            // Display the warning dialog
                if (!IsSilentInstall() && ::MessageBox(m_hWnd, szBuffer, szCaption, MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
            {
                numberOfRetry = 0;
                secondsToWait = 60;
                bRetryHttpRequest = TRUE;

                continue;
            }
            }
        }
        else {
            ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_SERVER_ERROR_CAPTION, szCaption, BUFFER_SIZE);
            ::LoadString(_Module.GetResourceInstance(), dwStatus, szBuffer, BUFFER_SIZE);
            if (lstrlen(szBuffer) ==0) wsprintf(szBuffer, "HTTP ErrorCode=%d", dwStatus);

            // Display the warning dialog
#ifndef SDK
            lstrcat(szBuffer, "\n");
            lstrcat(szBuffer, szURLFullPath);
#endif
                if (!IsSilentInstall()) ::MessageBox(m_hWnd, szBuffer, szCaption, MB_ICONINFORMATION | MB_OK);
        }

        dwDownloadError = 1;
        }
        else {
        TCHAR szCaption[BUFFER_SIZE];
        ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_UNKNOWN_ERROR_CAPTION, szCaption, BUFFER_SIZE);

        TCHAR szBuffer[BUFFER_SIZE], szBuffer2[BUFFER_SIZE];
        ::LoadString(_Module.GetResourceInstance(), IDS_HTTP_STATUS_UNKNOWN_ERROR, szBuffer, BUFFER_SIZE);
        
        // Print out status code
        wsprintf(szBuffer2, szBuffer, dwStatus);

        if (!IsSilentInstall()) ::MessageBox(m_hWnd, szBuffer2, szCaption, MB_ICONINFORMATION | MB_OK);

        dwDownloadError = 1;
        }   

        // Update status
        if (bUIFeedback) {
        ::LoadString(_Module.GetResourceInstance(), IDS_DOWNLOAD_STATUS_DISCONNECTING, szStatus, BUFFER_SIZE);
        ::SetWindowText(hProgressInfo, szStatus);
        }

        // Close HTTP request 
        // 
        // This is necessary if the HTTP request
        // is retried
        if (hRequest)
        ::InternetCloseHandle(hRequest);
        if (hFile != INVALID_HANDLE_VALUE) {
        ::CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
        }
    }
    while (bRetryHttpRequest);
    }
    __finally {
    if (hRequest)
        ::InternetCloseHandle(hRequest);

    if (hConnect)
        ::InternetCloseHandle(hConnect);

    if (hOpen)
        ::InternetCloseHandle(hOpen);

    if (hFile != INVALID_HANDLE_VALUE)
        ::CloseHandle(hFile);
    }

    // Exit dialog
    if (dwDownloadError == 0)
    return S_OK;
    else
    return E_FAIL;
}

//=--------------------------------------------------------------------------=
// CDownloadDialog::ParseConfiguration
//=--------------------------------------------------------------------------=
// Parse configuration file.
//
// Parameters:
//      szConfigFile    Local path to configuration file
//      szInstallerURL  Installer URL
//      szInstallerOptions Command line options
//      bResumable      Resumable download
//
// Output:
//      HRESULT
//
// Notes:
//
HRESULT CDownloadDialog::ParseConfiguration(const TCHAR* szConfigFile, 
                    TCHAR* szInstallerURL, 
                    TCHAR* szInstallerOptions, 
                    BOOL* pbResumable)
{
    BOOL bIgnoreDBI;
    return ParseConfiguration(szConfigFile, szInstallerURL, szInstallerOptions, pbResumable, &bIgnoreDBI);
}

//=--------------------------------------------------------------------------=
// CDownloadDialog::ParseConfiguration
//=--------------------------------------------------------------------------=
// Parse configuration file.
//
// Parameters:
//      szConfigFile    Local path to configuration file
//      szInstallerURL  Installer URL
//      szInstallerOptions Command line options
//      bResumable      Resumable download
//      pbIgnoreNBI     Indicates whether the flag for ignoring NBI policy exists
//
// Output:
//      HRESULT
//
// Notes:
//
HRESULT CDownloadDialog::ParseConfiguration(const TCHAR* szConfigFile, 
					TCHAR* szInstallerURL, 
					TCHAR* szInstallerOptions, 
					BOOL* pbResumable,
                    BOOL* pbIgnoreNBI )
{
    FILE* fp = NULL;
    const int len = BUFFER_SIZE * 10;
    char szBuffer[len];
#ifndef SDK
    BOOL bPostStatus = FALSE;
#endif

    *pbIgnoreNBI = FALSE;

    // Enable resumable download
    *pbResumable = TRUE;

    ::ZeroMemory(szBuffer, len);
    
    if ((fp = fopen(szConfigFile, "r")) != NULL) {
    fread(szBuffer, sizeof(char), len, fp);
    fclose(fp);
    }

    bool bParsingError = true;

    do {
    char *lang = GetLocaleStr();
    char *englang = "en";

    // Parse XML document
    XMLNode* xmlNode = ParseXMLDocument(szBuffer);

    // Root element must be "java-update"
    if (xmlNode != NULL && lstrcmp(xmlNode->_name, XML_JUPDATE_ELEMENT) == 0)
    {    
        // Iterate "information" element
        XMLNode* xmlInfoNode = xmlNode->_sub;

        while (xmlInfoNode != NULL) {       
        if (lstrcmp(xmlInfoNode->_name, XML_INFORMATION_ELEMENT) == 0)
        {
            char* version = FindXMLAttribute(xmlInfoNode->_attributes, XML_VERSION_ATTRIBUTE);
            char* szLanguage = FindXMLAttribute(xmlInfoNode->_attributes, XML_LANGUAGE_ATTRIBUTE);
    
            if ((lstrcmp(version, "1.0") == 0)
             && ((lstrcmp(szLanguage, lang) == 0) ||
                   (lstrcmp(szLanguage, englang) == 0)))
            {
            // this is the right locale or english(default)
            // if we don't find the right locale, take 
            // english locale part of XML 
    
            XMLNode* xmlUrlNode = FindXMLChild(xmlInfoNode->_sub, XML_MSI_URL_ELEMENT);
            XMLNode* xmlOptionsNode = NULL;

#ifdef SDK
                        xmlOptionsNode = FindXMLChild(xmlInfoNode->_sub, XML_JDK_OPTIONS_ELEMENT);
#else
                        
                        xmlOptionsNode = FindXMLChild(xmlInfoNode->_sub, XML_OPTIONS_ELEMENT);
                        XMLNode* xmlPostStatusNode = FindXMLChild(xmlInfoNode->_sub, XML_POSTSTATUS_ELEMENT);
            			XMLNode* xmlCntryLookupNode = FindXMLChild(xmlInfoNode->_sub, XML_CNTRYLOOKUP_ELEMENT);
            			XMLNode* xmlPreferenceEngineNode = FindXMLChild(xmlInfoNode->_sub, XML_PREFERENCE_ENGINE_ELEMENT);
                        
                        XMLNode* xmlIgnoreNbiNode = FindXMLChild(xmlInfoNode->_sub, XML_IGNORE_NBI_ELEMENT);
                        if( xmlIgnoreNbiNode != NULL && xmlIgnoreNbiNode->_sub != NULL) {
                            *pbIgnoreNBI = TRUE;
                        }

                        TCHAR szFCSVersion[128];
                        DWORD dwCount = sizeof(szFCSVersion);
                        szFCSVersion[0] = NULL;
                        //If FCS version is not already stored in the registry (from auto update)
                        if(!GetJavaUpdateKey(NULL, REG_JUPDATE_NEWJREVER, szFCSVersion, &dwCount))
                        {
                            //Then parse it from the XML file
                            XMLNode* xmlFCSVersionNode = FindXMLChild(xmlInfoNode->_sub, XML_VERSION_ELEMENT);
                            if(xmlFCSVersionNode != NULL && xmlFCSVersionNode->_sub != NULL)
                            {
                                //And then store it in the registry
                                XMLNode* xmlPcData = xmlFCSVersionNode->_sub;
                                SetJavaUpdateStringKey(NULL, REG_JUPDATE_NEWJREVER, xmlPcData->_name);
                            }
                        }
#endif

            if (xmlUrlNode != NULL && xmlUrlNode->_sub != NULL)
            {
                XMLNode* xmlPcData = xmlUrlNode->_sub;
    
                _tcscpy(szInstallerURL, xmlPcData->_name);
    
                // Look for installer command options
                //
                if (xmlOptionsNode != NULL && xmlOptionsNode->_sub != NULL)
                {
                    XMLNode* xmlPcData = xmlOptionsNode->_sub;
                    if (lstrlen(szInstallerOptions) >1) {
                        //if installing from auto-update and we want to override options then only save the XML options
                        BOOL bOverride = OverrideOptions();
                        if(IsInstallFromJavaUpdate()) {
                            if(bOverride) {
                            //zero szInstallerOptions memory
                            memset(szInstallerOptions, NULL, lstrlen(szInstallerOptions));
                            //copy XML options to szInstallerOptions
                            _tcscpy(szInstallerOptions, xmlPcData->_name);
                            }
                            else {
                            //if installing from auto-update and we DON'T want to override the cmdline options
                            //szInstallerOptions already contains cmdline options, so do nothing
                            }
                        }
                        else {
                            //if NOT installing from auto-update
                            //append cmdline options to xml file options
                           TCHAR szTemp[256];
                           _tcscpy(szTemp, szInstallerOptions);
                           _tcscpy(szInstallerOptions, xmlPcData->_name);
                           _tcscat(szInstallerOptions, szTemp);
                       }
                    }
                    else {
                        //There are no cmdline options, so just copy XML options to szInstallerOptions
                       _tcscpy(szInstallerOptions, xmlPcData->_name);
                    }
                }
#ifndef JKERNEL
                // Get vector setting in the xml file
                    XMLNode* xmlVPreURLNode = FindXMLChild(xmlInfoNode->_sub, XML_VPRELOADURL_ELEMENT);
                    if (xmlVPreURLNode != NULL && xmlVPreURLNode->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVPreURLNode->_sub;
                    SetVPreloadURLValue(xmlPcData->_name);

                      }
                    XMLNode* xmlVRunURLNode = FindXMLChild(xmlInfoNode->_sub, XML_VRUNVECTOR_ELEMENT);
                    if (xmlVRunURLNode != NULL && xmlVRunURLNode->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVRunURLNode->_sub;
                    SetVRunURLValue(xmlPcData->_name);
                    
                      }
                    XMLNode* xmlVMinOSNode = FindXMLChild(xmlInfoNode->_sub, XML_VMINOS_ELEMENT);
                    if (xmlVMinOSNode != NULL && xmlVMinOSNode->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVMinOSNode->_sub;
                    SetVMinOSValue(xmlPcData->_name);

                      }
                    XMLNode* xmlVMinPSpeedNode = FindXMLChild(xmlInfoNode->_sub, XML_VMINPSPEED_ELEMENT);
                    if (xmlVMinPSpeedNode != NULL && xmlVMinPSpeedNode->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVMinPSpeedNode->_sub;
                    SetVMinPSpeedValue(xmlPcData->_name);

                      }
                    XMLNode* xmlVMinMemory = FindXMLChild(xmlInfoNode->_sub, XML_VMINMEMORY_ELEMENT);
                    if (xmlVMinMemory != NULL && xmlVMinMemory->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVMinMemory->_sub;
                    SetVMinMemoryValue(xmlPcData->_name);

                      }
                    XMLNode* xmlVLocaleNode = FindXMLChild(xmlInfoNode->_sub, XML_VLOCALES_ELEMENT);
                    if (xmlVLocaleNode != NULL && xmlVLocaleNode->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVLocaleNode->_sub;
                    SetVLocaleValue(xmlPcData->_name);

                      }
                    XMLNode* xmlVGeosNode = FindXMLChild(xmlInfoNode->_sub, XML_VGEOS_ELEMENT);
                    if (xmlVGeosNode != NULL && xmlVGeosNode->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVGeosNode->_sub;
                    SetVGeosValue(xmlPcData->_name);

                      }
                    XMLNode* xmlVAuxNameNode = FindXMLChild(xmlInfoNode->_sub, XML_VAUXNAME_ELEMENT);
                    if (xmlVAuxNameNode != NULL && xmlVAuxNameNode->_sub !=NULL)
                      {
                    XMLNode* xmlPcData = xmlVAuxNameNode->_sub;
                    SetVAuxNameValue(xmlPcData->_name);

                      }

                 
#endif                
#ifndef SDK
                bPostStatus = FALSE;
                if (xmlPostStatusNode != NULL && xmlPostStatusNode->_sub != NULL)
                {
                xmlPcData = xmlPostStatusNode->_sub;
                if (lstrcmp(xmlPcData->_name, "0") != 0) {
                    SetPostStatusURL(xmlPcData->_name);
                    bPostStatus = TRUE;
                }
                }
                // Get preference engine order
                //
                if (xmlPreferenceEngineNode != NULL && xmlPreferenceEngineNode->_sub !=NULL)
                {

                XMLNode* xmlPcData = xmlPreferenceEngineNode->_sub;

                SetPreferenceEngineOrder(xmlPcData->_name);

                } 
                if (xmlCntryLookupNode != NULL && xmlCntryLookupNode->_sub != NULL)
                {
                xmlPcData = xmlCntryLookupNode->_sub;
                SetCountryLookupURL(xmlPcData->_name);
                }
#endif
                            // Signal an event as soon as Vector info is parsed, to display the License Agreement dialog
                            ::SetEvent(m_hVectorParsingEvent);

                bParsingError = false;
            }
            else {
                // found the right locale, but identifier or URL or not set
                bParsingError = true;
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
    }
    while (0);

#ifndef SDK
    //XMLFile setting should override any compile time option
    SetPostStatus(bPostStatus);
#endif
    
    // There is a parsing error
    if (bParsingError) {
    TCHAR szErrorCaption[BUFFER_SIZE];

    ::LoadString(_Module.GetResourceInstance(), IDS_PARSINGERROR_MSG, szErrorCaption, BUFFER_SIZE);

    // Cut message to 1024 characters to avoid screen overrun if config file is big
    sprintf(szBuffer + 1024, "....");

    // Popup dialog
    if (!IsSilentInstall()) MessageBox(szBuffer, szErrorCaption, MB_ICONERROR | MB_OK);

    return E_FAIL;
    }
    else {   
#ifdef SDK
        if (strstr(szInstallerURL, TEXT("https://")) != NULL) SetSingleMsi(TRUE);
#endif
    return S_OK;
    }
}

BOOL CDownloadDialog::SilentDownloadProc(BOOL bDownloadOnly, BOOL *pbIgnoreNBI)
{
    TCHAR szCachedLocalFile[BUFFER_SIZE];

    BOOL bResumable = FALSE;
    BOOL bSuccess = FALSE;
    
    do {
        // Config URL is not set, for default MSI file
    if (lstrlen(m_pszConfigURL) > 0) {
        // Download configuration file
        if (FAILED(DownloadConfiguration(m_pszConfigURL, szCachedLocalFile, BUFFER_SIZE)))
        break;

	    // Parse configuration file
	    if (FAILED(ParseConfiguration(szCachedLocalFile, m_pszInstallerURL, 
					   m_pszInstallerOptions, &bResumable, pbIgnoreNBI)))
		break;

        //if -download was passed and the Ignore NBI element was in the config, then return without downloading
        if (bDownloadOnly && *pbIgnoreNBI == TRUE) {
            return TRUE;
        }
	}
#ifdef SDK
            lstrcpy(m_pszFileName, m_pszInstallerURL);
            if (IsSingleMsi()) {
                // Download Singlemsi without UI feedback
                TCHAR singleMSIURL[1024], mstURL[1024];
                wsprintf(singleMSIURL, "%s/%s%s.msi", m_pszInstallerURL, BUNDLE, VERSION);
                wsprintf(mstURL, "%s/%s", m_pszInstallerURL, m_pszMSTBaseFileName);
                if (FAILED(DownloadFile(singleMSIURL, m_pszSingleMSIFileName, bResumable, TRUE)))
                  break;
                if(strstr(m_pszMSTBaseFileName, "1033.MST") == NULL)
                   if (FAILED(DownloadFile(mstURL, m_pszMSTFileName, bResumable, TRUE)))
                     break;
            }
#else
    // Download Singlemsi & MST files without UI feedback
    TCHAR msiURL[512], mstURL[512], pszBaseURL[1024];

    msiURL[0] = NULL;
    mstURL[0] = NULL;
    pszBaseURL[0] = NULL;

        //Silent installation does not offer any sponsor, but it's
        //required for Java Update which does silent download &
        //UI mode installation.

    wsprintf(msiURL, "%s/%s.msi", m_pszInstallerURL, m_szDownLoadVersion);
    m_bSponsorEnabled = IsSponsorEnabled(m_pszInstallerOptions, pszBaseURL);
        if (m_bSponsorEnabled) wsprintf(mstURL, "%s/", pszBaseURL); 
        else wsprintf(mstURL, "%s/", m_pszInstallerURL); 
        SetMSTFileNames(m_bSponsorEnabled, m_pszMSTCachedFile, mstURL);

        //check if files are predownloaded by javaupdate or earlier downld
        if (IsFilesDownloaded(m_pszSingleMSIFileName, m_pszMSTCachedFile, NULL))
        {
            bSuccess=TRUE;
            break;
        }
        if (m_bDownloadOnly) {
            //use Async download
            CComObject<CDownloadFile> downloadFile;
            downloadFile.SaveInRegistry(FALSE);
            LPCTSTR urls[2];
            LPCTSTR dest[2];
            DWORD numfiles = 1;
            urls[0] = msiURL;
            dest[0] = m_pszSingleMSIFileName; 
            if (lstrlen(m_pszMSTCachedFile)>1) {
                numfiles = 2;
                urls[1] = mstURL;
                dest[1] = m_pszMSTCachedFile; 
            }
                 
            downloadFile.SetFiles(numfiles, urls, dest);
            if (FAILED(downloadFile.AsyncDownload())) break;
        }
        else {
            if (FAILED(DownloadFile(msiURL, m_pszSingleMSIFileName, bResumable, FALSE)))
            break;
        if ((lstrlen(m_pszMSTCachedFile) > 1) && 
                 FAILED(DownloadFile(mstURL, m_pszMSTCachedFile, bResumable, FALSE)))
            {
            break;
            }
        }
#endif
        // Installer has been downloaded
        bSuccess = TRUE;
    } while (0);
    return bSuccess;
}


// Create the fonts we need for the download and 
// install UE
HFONT CDownloadDialog::CreateDialogFont (HDC hdc, LPCTSTR lpszFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline)
{
    POINT pt;
    FLOAT cxDPI, cyDPI;
    HFONT hFont;
    LOGFONT lf;

    int iDeciPtWidth = 0;
    int iDeciPtHeight = 10 * ptSize;

    int iSavedDC = SaveDC(hdc);

    SetGraphicsMode (hdc, GM_ADVANCED);
    ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
    SetViewportOrgEx (hdc, 0,0, NULL);
    SetWindowOrgEx (hdc, 0,0, NULL);

    cxDPI = (FLOAT) GetDeviceCaps(hdc, LOGPIXELSX);
    cyDPI = (FLOAT) GetDeviceCaps(hdc, LOGPIXELSY);

    pt.x = (int) (iDeciPtWidth * cxDPI / 72);
    pt.y = (int) (iDeciPtHeight * cyDPI / 72);

    DPtoLP(hdc, &pt, 1);

    lf.lfHeight = - (int) (fabs ((double) pt.y) / 10.0 + 0.5);
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = dwWeight;
    lf.lfItalic = 0;
    lf.lfUnderline = bUnderline;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = 0;
    lf.lfOutPrecision = 0;
    lf.lfClipPrecision = 0;
    lf.lfQuality = 0;
    lf.lfPitchAndFamily = 0;

    TCHAR szLocaleData[BUFFER_SIZE];
    int iRet = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGCOUNTRY, szLocaleData, BUFFER_SIZE);

    if (strncmp(szLocaleData, "Japan", 5) != 0)
    strcpy (lf.lfFaceName, lpszFaceName);
    else
    strcpy (lf.lfFaceName, TEXT("MS UI Gothic"));

    hFont = CreateFontIndirect(&lf);

    RestoreDC (hdc, iSavedDC);
    return hFont;
}

#ifndef SDK
BOOL CDownloadDialog::IsSponsorEnabled(LPSTR lpszCmdlineOptions, LPSTR lpszBaseURL)
{
    if (!m_bSponsorEnabled) return FALSE;
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
    DWORD dwCount=sizeof(DWORD), dwVal = 1;
    GetJavaSoftKey("SPONSORS", (LPSTR)&dwVal, dwCount);
    if (dwVal == 0)
       return FALSE;
    return TRUE;
}
#endif


// Message handler for WM_SETCURSOR.
// the goal here is to set a "hand" cursor for the "More Information..." link
LRESULT CDownloadDialog::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
    POINT pt;
    RECT rect;

    ::GetCursorPos(&pt);
    ::GetWindowRect (GetDlgItem(IDC_MARKETING_INFO2), &rect);

    if (::PtInRect(&rect, pt)) {
        ::SetCursor(::LoadCursor(NULL, IDC_HAND));
    }
    else {
        ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    }
    return TRUE;
}

LRESULT CDownloadDialog::ShowMoreInfo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wID == IDC_MARKETING_INFO2) {
          ShellExecute(NULL, "open", "http://java.com", NULL, NULL, SW_SHOWNORMAL);
    }
    return 0;
}

void CDownloadDialog::FreeGDIResources ()
{
    ::DeleteObject(m_hNormalFont);
    m_hNormalFont = NULL;

    ::DeleteObject(m_hMediumFont);
    m_hMediumFont = NULL;

    ::DeleteObject(m_hBoldFont);
    m_hBoldFont = NULL;

    ::DeleteObject(m_hBitmap);
    m_hBitmap = NULL;

    ::DeleteObject(m_hDlgBrush);
    m_hDlgBrush = NULL;

    ::DeleteDC(m_hMemDC);
    m_hMemDC = NULL;
}

