/*
 * @(#)ErrorHelpDialog.h	1.1 03/26/2009
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// WelcomeDialog.h : Declaration of the CWelcomeDialog
//

#ifndef __WELCOME_DIALOG_H_
#define __WELCOME_DIALOG_H_

#include "resource.h"       // main symbols

// IDC_HAND is defined in winUser.h, but only if (WINVER >= 0x0500),
// which is not the case in our builds.

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDialog
//
class CWelcomeDialog : 
	public CAxDialogImpl<CWelcomeDialog>
{
public:
	CWelcomeDialog();

	enum { IDD = IDD_WELCOME_DIALOG };

BEGIN_MSG_MAP(CWelcomeDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDC_WELCOME_CLICK_HERE, ShowClickHere)
	COMMAND_ID_HANDLER(IDC_WELCOME_LICENSE_AGREEMENT, ShowLicenseAgreement)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ShowClickHere(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT ShowLicenseAgreement(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void setBodyText(int iBodyText)
	{
            m_iBodyText = iBodyText;
	}
	
    void setIsAltInstallDir(BOOL bAltInstallDir)
	{
	    m_bAltInstallDir = bAltInstallDir;
	}
    
    BOOL getIsAltInstallDir()
	{
	    return m_bAltInstallDir;
	}
    
    void setIsVector(BOOL bVector)
    {
        m_bIsVector = bVector;
    }
    
    BOOL getIsVector()
    {
        return m_bIsVector;
    }
    
    void setIsCheckboxDisabled(BOOL bCheckDisabled)
    {
        m_bIsCheckDisabled = bCheckDisabled;
    }
    
    BOOL getIsCheckboxDisabled()
    {
        return m_bIsCheckDisabled;
    }
    
    private:
    BOOL    m_bAltInstallDir;
    BOOL    m_bIsVector;
    BOOL    m_bIsCheckDisabled;
	HFONT CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline);
	void  FreeGDIResources ();
	TCHAR	m_szDescription[1024];
	TCHAR	m_szUrl[256];
	int     m_iBodyText;
	HDC	    m_hMemDC;
    HBITMAP m_hBitmap;
	BITMAP	m_bmBannerJFX;
	HBRUSH	m_hDlgBrush1;
	HFONT   m_hDialogFont;
    HFONT   m_hDialogHeaderFont;

};

#endif //__WELCOME_DIALOG_H_
