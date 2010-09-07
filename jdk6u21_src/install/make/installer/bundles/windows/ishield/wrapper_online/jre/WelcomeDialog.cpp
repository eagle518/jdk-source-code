/*
 * @(#)WelcomeDialog.cpp	1.1 03/26/2009
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// WelcomeDialog.cpp : Implementation of CWelcomeDialog

#include "stdafx.h"
#include <atlhost.h>
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <math.h>
#include <shellapi.h>
#include "WelcomeDialog.h"
#include "common.h"

#define BUFFER_SIZE 256


/////////////////////////////////////////////////////////////////////////////
// CWelcomeDialog

CWelcomeDialog::CWelcomeDialog()
{
    
    m_szDescription[0] = NULL;
    m_bIsVector = FALSE;
    m_bAltInstallDir = FALSE;
    m_bIsCheckDisabled = FALSE;

    // Load up commctrl.dll
    InitCommonControls();
}


//=--------------------------------------------------------------------------=
// CWelcomeDialog::OnInitDialog
//=--------------------------------------------------------------------------=
// Message handler for WM_INITDIALOG
//
// Parameters:
//	uMsg	    Windows Message
//	wParam	    WPARAM
//	lParam	    LPARAM
//	bHandled    FALSE if not handled
//
// Output:
//	LRESULT	    
//
// Notes:
//
LRESULT CWelcomeDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    m_hDlgBrush1 = CreateSolidBrush(RGB(231, 229, 213)); //default windows bg color
    m_hMemDC = NULL;
    m_hDialogFont = NULL;
    m_hDialogHeaderFont = NULL;

    HWND hWnd;
    DWORD style;

    // Allow SS_NOTIFY msgs for the URL links
    hWnd = GetDlgItem(IDC_WELCOME_CLICK_HERE);
    style = ::GetWindowLong(hWnd, GWL_STYLE);
    ::SetWindowLong(hWnd, GWL_STYLE, style | SS_NOTIFY);    
    hWnd = GetDlgItem(IDC_WELCOME_LICENSE_AGREEMENT);
    style = ::GetWindowLong(hWnd, GWL_STYLE);
    ::SetWindowLong(hWnd, GWL_STYLE, style | SS_NOTIFY);

    if (getIsAltInstallDir()) {
        CheckDlgButton(IDC_WELCOME_CHECKBOX, BST_CHECKED);
    }

    // Make dialog box not having any focus by default
    // Set Keyboard focus on a 0 size button
    GotoDlgCtrl(GetDlgItem(IDNO));
    
    // SetForegroundWindow() was deprecated for Vista (attn: session 0 isolation)
    // Use SetWindowPos() for Vista+ in order to bring the window to the top of the Z order
    // First call without SHOWWINDOW for TOPMOST window handles, then 2nd call with SHOWWINDOW for NOTOPMOST
    // has the same effect as BringWindowToTop()...both are suggested implementations for Vista's answer
    // to "stealing focus" problem. Note: SwitchToThisWindow() was removed, as it will not work on some varients of Window Vista.
    if (IsPlatformWindowsVista()){
        SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(HWND_NOTOPMOST,0,0,0,0,SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    } else {
        if (::SetForegroundWindow(GetDlgItem(IDOK)) == NULL){
            return 1; // If error, Let the system set the focus
        }
    }

    return 0;

}


//=--------------------------------------------------------------------------=
// CWelcomeDialog::OnOK
//=--------------------------------------------------------------------------=
// Message handler for WM_COMMAND with IDOK
//
// Parameters:
//	wNotifyCode Notify Code
//	wID	    ID of control
//	hWndCtl	    HWND of control
//	bHandled    FALSE if not handled
//
// Output:
//	LRESULT	    
//
// Notes:
//
LRESULT CWelcomeDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);
    FreeGDIResources();

    // Trap User request to change installation dir
    if (IsDlgButtonChecked(IDC_WELCOME_CHECKBOX)){
        setIsAltInstallDir(TRUE);
    } else {
        setIsAltInstallDir(FALSE);
    }
    
    // Destroy dialog
    EndDialog(wID);

    return 0;
}


//=--------------------------------------------------------------------------=
// CWelcomeDialog::OnCancel
//=--------------------------------------------------------------------------=
// Message handler for WM_COMMAND with IDCANCEL
//
// Parameters:
//	wNotifyCode Notify Code
//	wID	    ID of control
//	hWndCtl	    HWND of control
//	bHandled    FALSE if not handled
//
// Output:
//	LRESULT	    
//
// Notes:
//
LRESULT CWelcomeDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    // First display an "are you sure you want to cancel" prompt
    TCHAR szAskMsg[BUFFER_SIZE] = {NULL};
    TCHAR szCaption[BUFFER_SIZE] = {NULL};
    TCHAR szMsg[BUFFER_SIZE] = {NULL};
    
	::LoadString(_Module.GetResourceInstance(), IDS_AREYOUSURE_MESSAGE, szAskMsg, BUFFER_SIZE);
	::LoadString(_Module.GetResourceInstance(), IDS_AREYOUSURE_CAPTION, szCaption, BUFFER_SIZE);
	wsprintf(szMsg, szAskMsg, VERSION);
	
    if (MessageBox(szMsg, szCaption, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES){
        return 0;
    }

    // We need to set the 'declined' regkey to avoid sending an IS2 ping
    // this was done via a custom action kicked off by the MSI Welcome Screen
    DWORD created = 0;
    HKEY hKey;
    
    RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\JavaSoft", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &created);
    RegSetValueEx(hKey, "InstallStatus", 0, REG_SZ, (LPBYTE)"decline", lstrlen("decline"));
    RegCloseKey(hKey);
    
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);
    FreeGDIResources();

    // Destroy dialog
    EndDialog(wID);

    return 0;
}

LRESULT CWelcomeDialog::OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LONG)m_hDlgBrush1;
}


// this message is sent each time a static control is drawn.
// we get the Control ID and then set background color and font
// as appropriate for that control.
//
LRESULT CWelcomeDialog::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    HDC hdc = (HDC) wParam;
    HWND hwnd = (HWND) lParam;

    int DlgCtrlID = ::GetDlgCtrlID(hwnd);
	
	if ((DlgCtrlID == IDC_WELCOME_LINE1) || (DlgCtrlID == IDC_WELCOME_LICENSE_TEXT1) || (DlgCtrlID == IDC_WELCOME_LICENSE_TEXT2)
		|| (DlgCtrlID == IDC_WELCOME_NOTE) || (DlgCtrlID == IDC_WELCOME_MORE_INFO)){
		
        if (m_hDialogFont == NULL){
			m_hDialogFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 10, FW_MEDIUM, FALSE);
        }
         
        if (DlgCtrlID == IDC_WELCOME_LINE1){
            
            TCHAR szText[BUFFER_SIZE] = {NULL};
            ::LoadString(_Module.GetResourceInstance(), IDS_WELCOME_LINE1, szText, BUFFER_SIZE);
            ::SetWindowText(hwnd, szText);

        }
        
        ::SelectObject(hdc, m_hDialogFont);
        return (LRESULT) GetStockObject(WHITE_BRUSH);	

	} else if (DlgCtrlID == IDC_WELCOME_HEADER){

        TCHAR szHeaderText[BUFFER_SIZE] = {NULL};
        
        if (m_hDialogHeaderFont == NULL){
            m_hDialogHeaderFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 12, FW_BOLD, FALSE);
        }
        
        ::LoadString(_Module.GetResourceInstance(), IDS_WELCOME_MAIN, szHeaderText, BUFFER_SIZE);
        ::SetWindowText(hwnd, szHeaderText);
        
        ::SelectObject(hdc, m_hDialogHeaderFont);
        ::SetBkMode(hdc, TRANSPARENT);
        return (LONG)m_hDlgBrush1;

    } else if (DlgCtrlID == IDC_BACK_COLOR){

        return (LRESULT) GetStockObject(WHITE_BRUSH);
        
    } else if (DlgCtrlID == IDC_BANNER){

     	if (m_hMemDC == NULL){

            m_hBitmap = LoadBitmap(_Module.GetModuleInstance(),
                                   MAKEINTRESOURCE(IDI_BANNER));
            GetObject(m_hBitmap, sizeof(BITMAP), &m_bmBannerJFX);
            m_hMemDC = CreateCompatibleDC(NULL);
            SelectObject(m_hMemDC, m_hBitmap);

	    }

	    RECT rect;
	    ::GetClientRect(hwnd, &rect);
	    StretchBlt(hdc, rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top),
                   m_hMemDC, 0, 0, m_bmBannerJFX.bmWidth, m_bmBannerJFX.bmHeight, SRCCOPY);

	    return (LRESULT) GetStockObject(NULL_BRUSH);

    } else if ((DlgCtrlID == IDC_WELCOME_LICENSE_AGREEMENT) || (DlgCtrlID == IDC_WELCOME_CLICK_HERE)){

        if (m_hDialogFont == NULL){
	        m_hDialogFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 10, FW_MEDIUM, FALSE);
	    }

	    ::SelectObject(hdc, m_hDialogFont);
	    ::SetBkMode(hdc, TRANSPARENT);
        ::SetTextColor(hdc, RGB(255, 0, 0));

        return (LRESULT) GetStockObject(NULL_BRUSH);

    } else if (DlgCtrlID == IDC_WELCOME_CHECKBOX){

        // Disable the checkbox if PIP/Re-install
        if (getIsCheckboxDisabled()){
            ::ShowWindow(hwnd, SW_HIDE);
        }
        
        // Set Checkbox text transparent
        ::SetBkMode(hdc, TRANSPARENT);

    }

    return (LONG)m_hDlgBrush1;

}


// Message handler for WM_SETCURSOR.
// the goal here is to set a "hand" cursor for the "License Agreement" and "Click Here" links
//
LRESULT CWelcomeDialog::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    POINT pt;
    RECT rectClickHere;
    RECT rectLicenseAgreement;
	
    ::GetCursorPos(&pt);
    ::GetWindowRect (GetDlgItem(IDC_WELCOME_CLICK_HERE), &rectClickHere);
    ::GetWindowRect (GetDlgItem(IDC_WELCOME_LICENSE_AGREEMENT), &rectLicenseAgreement);

    if ((::PtInRect(&rectClickHere, pt)) || (::PtInRect(&rectLicenseAgreement, pt))){
     	::SetCursor(::LoadCursor(NULL, IDC_HAND));
    } else {
     	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    }

    return TRUE;

}

LRESULT CWelcomeDialog::ShowClickHere(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    if (wID == IDC_WELCOME_CLICK_HERE){

        ::LoadString(_Module.GetModuleInstance(), IDS_CLICK_HERE_URL, m_szUrl, BUFFER_SIZE);
        ShellExecute(NULL, "open", m_szUrl, NULL, NULL, SW_SHOWNORMAL);
        
    }

    return 0;

}

LRESULT CWelcomeDialog::ShowLicenseAgreement(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    if (wID == IDC_WELCOME_LICENSE_AGREEMENT){

        TCHAR szLicUrl[BUFFER_SIZE] = {NULL};
        ::LoadString(_Module.GetModuleInstance(), IDS_LICENSE_AGREEMENT_URL, szLicUrl, BUFFER_SIZE);
        wsprintf(m_szUrl, szLicUrl, VERSION);
        ShellExecute(NULL, "open", m_szUrl, NULL, NULL, SW_SHOWNORMAL);

    }

    return 0;

}

void CWelcomeDialog::FreeGDIResources()
{

    ::DeleteObject(m_hDialogHeaderFont);
    m_hDialogHeaderFont = NULL;

    ::DeleteObject(m_hDialogFont);
    m_hDialogFont = NULL;

    ::DeleteObject(m_hDlgBrush1);
    m_hDlgBrush1 = NULL;

    ::DeleteDC(m_hMemDC);
    m_hMemDC = NULL;

}

HFONT CWelcomeDialog::CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline)
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
    lf.lfUnderline = (bUnderline ? 1 : 0);
    lf.lfStrikeOut = 0;
    lf.lfCharSet = 0;
    lf.lfOutPrecision = 0;
    lf.lfClipPrecision = 0;
    lf.lfQuality = 0;
    lf.lfPitchAndFamily = 0;

    TCHAR szLocaleData[BUFFER_SIZE];
    int iRet = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGCOUNTRY, szLocaleData, BUFFER_SIZE);

    if (strncmp(szLocaleData, "Japan", 5) != 0){
	strcpy (lf.lfFaceName, szFaceName);
    } else {
	strcpy (lf.lfFaceName, TEXT("MS UI Gothic"));
    }

    hFont = CreateFontIndirect(&lf);
    RestoreDC (hdc, iSavedDC);

    return hFont;

}

