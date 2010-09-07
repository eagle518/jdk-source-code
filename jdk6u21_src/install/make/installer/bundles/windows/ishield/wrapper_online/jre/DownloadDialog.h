/*
 * @(#)DownloadDialog.h	1.31 04/10/21
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// DownloadDialog.h : Declaration of the CDownloadDialog
//
// By Stanley Man-Kit Ho
//

#ifndef __DOWNLOADDIALOG_H_
#define __DOWNLOADDIALOG_H_

#include "resource.h"       // main symbols
#include <time.h>

#define BUFFER_SIZE 2048
#define iTimerID    1000


/////////////////////////////////////////////////////////////////////////////
// CDownloadDialog
class CDownloadDialog : 
	public CAxDialogImpl<CDownloadDialog>
{
public:
	CDownloadDialog();
	~CDownloadDialog();

	enum { IDD = IDD_DOWNLOAD_DIALOG };

BEGIN_MSG_MAP(CDownloadDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_TIMER, OnTimer)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDC_MARKETING_INFO2, ShowMoreInfo)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ShowMoreInfo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	STDMETHODIMP OnStartBinding();
	STDMETHODIMP OnStartInstallerBinding();
	STDMETHODIMP OnProgress(ULONG ulProgress, ULONG ulProgressMax);
	STDMETHODIMP OnStopBinding();
 	BOOL WINAPI SilentDownloadProc(BOOL bDownloadOnly, BOOL *pbIgnoreNBI);

	void setConfigURL(LPCTSTR pszURL)
	{
	    m_pszConfigURL = pszURL;
	}

	void setSingleMSIFile(LPTSTR pszFileName)
	{
	    m_pszSingleMSIFileName = pszFileName;
	}

	void setInstallerURL(LPTSTR pszInstallerURL)
	{
	    m_pszInstallerURL = pszInstallerURL;
	}

	void setInstallerCmdOpts(LPTSTR pszInstallerOptions)
	{
	    m_pszInstallerOptions =  pszInstallerOptions;
	}
	
	void setIsSilentInstall(BOOL bSilentInstall)
	{
	    m_bSilentInstall = bSilentInstall;
	}

	BOOL IsSilentInstall()
	{
	    return m_bSilentInstall;
	}

	BOOL IsCancelled()
	{
	    return (!m_bError && m_bUserCancelled);
	}
#ifdef SDK
	void setMSTFile(LPTSTR pszMSTFileName, LPTSTR pszMSTBaseFileName)
	{
            m_pszMSTFileName = pszMSTFileName;
            m_pszMSTBaseFileName = pszMSTBaseFileName;
	}

        void setInstallerFile(LPTSTR pszFileName)
        {
            m_pszFileName = pszFileName;
        }

        LPTSTR getInstallerURL()
        {
          return m_pszInstallerURL;
        }

        void SetSingleMsi(BOOL bSingleMsi)
        {
            m_bSingleMsi = bSingleMsi;
        }

        BOOL IsSingleMsi()
        {
            return m_bSingleMsi;
        }
#else
	void setMSTFile(LPTSTR pszMSTCachedFile)
	{
	    m_pszMSTCachedFile = pszMSTCachedFile;
	}

	BOOL GetSponsorEnabled()
	{
	    return m_bSponsorEnabled;
	}

	void SetSponsorEnabled(BOOL bSponsor)
	{
	    m_bSponsorEnabled = bSponsor;
	}

	void SetDownLoadVersion(LPTSTR szDownLoadVersion)
	{
	    m_szDownLoadVersion = szDownLoadVersion;
	}

	void SetDownloadOnly(BOOL bDownloadOnly)
	{
	    m_bDownloadOnly = bDownloadOnly;
	}
    void SetIsAltInstallDir(BOOL bAltInstalldir)
    {
        m_bAltInstalldir = bAltInstalldir;
    }
    BOOL GetIsAltInstalldir()
    {
        return m_bAltInstalldir;
    }
    void SetShowWelcome(BOOL bWelcome)
    {
        m_bShowWelcome = bWelcome;
    }
    BOOL GetShowWelcome()
    {
        return m_bShowWelcome;
    }
    void SetShowSplash(BOOL bSplash)
    {
        m_bShowSplash = bSplash;
    }
    BOOL GetShowSplash()
    {
        return m_bShowSplash;
    }
    void SetIsPIP(BOOL bPIP)
    {
        m_bPIP = bPIP;
    }
    
    BOOL GetIsPIP()
    {
        return m_bPIP;
    }
    
#endif

        private:
	static DWORD WINAPI DownloadThreadProc(LPVOID lpParameter);
	HRESULT DownloadConfiguration(const TCHAR* szConfigURL, TCHAR* szConfigFile, DWORD dwBufLength);
	HRESULT ParseConfiguration(const TCHAR* szConfigFile, TCHAR* szURL, TCHAR* szCommandOptions, BOOL* bResumable);
    HRESULT ParseConfiguration(const TCHAR* szConfigFile, TCHAR* szInstallerURL, TCHAR* szInstallerOptions, 
					BOOL* pbResumable, BOOL* pbIgnoreNBI );
	HRESULT DownloadFile(const TCHAR* szURL, const TCHAR* szLocalFile, BOOL bResumable, BOOL bUIFeedback);
	HFONT CreateDialogFont (HDC hdc, LPCTSTR lpszFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline);
	void  FreeGDIResources ();
	BOOL IsSponsorEnabled(LPSTR lpszCmdlineOptions, LPSTR lpszBaseURL);

	LPCTSTR			m_pszConfigURL;
	LPTSTR			m_pszSingleMSIFileName;
	LPTSTR			m_pszInstallerURL;
	LPTSTR			m_pszInstallerOptions;
#ifdef SDK
        LPTSTR                  m_pszMSTFileName;
        LPTSTR                  m_pszMSTBaseFileName;
        LPTSTR                  m_pszFileName;
        BOOL                    m_bSingleMsi;
#else
	LPTSTR			m_pszMSTFileDir;
	LPTSTR			m_pszMSTCachedFile;
	LPTSTR			m_szDownLoadVersion;
	BOOL                    m_bSponsorEnabled;
	BOOL                    m_bDownloadOnly;
#endif
	time_t			m_startTime;
	ULONG			m_ulProgress;
	ULONG			m_ulProgressMax;
	int             m_iProgressFactor;
	int             m_iMaxProgressFactor;
	CComAutoCriticalSection m_csDownload;
	HANDLE			m_hCancelEvent;
    BOOL            m_bUserCancelled;
    BOOL            m_bError;
    BOOL            m_bAltInstalldir;
    BOOL            m_bShowWelcome;
    BOOL            m_bShowSplash;
    BOOL            m_bPIP;
	HANDLE			m_hDownloadThreadExitEvent;
    HANDLE          m_hVectorParsingEvent;
	BOOL			m_bSilentInstall;
	UINT			m_WindowCaption;
	UINT			m_WindowText;
	BOOL            m_bXMLFile;
	BITMAP			m_bmJavaBanner;
	HBITMAP			m_hBitmap;
	HFONT			m_hNormalFont;
    HFONT           m_hMediumFont;
	HFONT			m_hBoldFont;
	HDC             m_hMemDC;
	HBRUSH			m_hDlgBrush;
};

#endif //__DOWNLOADDIALOG_H_
