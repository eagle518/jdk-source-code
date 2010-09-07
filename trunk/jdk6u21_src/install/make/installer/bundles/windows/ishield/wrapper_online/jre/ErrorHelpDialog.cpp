/*
 * @(#)ErrorHelpDialog.cpp	1.1 06/01/19
 *
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// ErrorHelpDialog.cpp : Implementation of CErrorHelpDialog

#include "stdafx.h"
#include <atlhost.h>
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <math.h>
#include <shellapi.h>
#include "ErrorHelpDialog.h"

#define BUFFER_SIZE 256


/////////////////////////////////////////////////////////////////////////////
// CErrorHelpDialog

CErrorHelpDialog::CErrorHelpDialog()
{
    m_szDescription[0] = NULL;

    // Load up commctrl.dll
    InitCommonControls();
}


//=--------------------------------------------------------------------------=
// CErrorHelpDialog::OnInitDialog
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
LRESULT CErrorHelpDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    m_hDlgBrush1 = CreateSolidBrush(RGB(255, 255, 255)); //white color
    m_hMemDC = NULL;
    m_hDialogFont = NULL;
    m_hDialogBoldFont = NULL;
	m_hDialogUndelineFont = NULL;

    TCHAR szBuffer[BUFFER_SIZE];

	HWND hWnd = GetDlgItem(IDC_EBODY_TEXT);
    ::LoadString(_Module.GetModuleInstance(), m_iBodyText, szBuffer, BUFFER_SIZE);
    ::SetWindowText(hWnd, szBuffer);

	hWnd = GetDlgItem(IDC_MORE_INFO);
    DWORD style = ::GetWindowLong(hWnd, GWL_STYLE);
    ::SetWindowLong(hWnd, GWL_STYLE, style | SS_NOTIFY);

    if (::SetForegroundWindow(GetDlgItem(IDOK)) == NULL) 
	return 1; // If error, Let the system set the focus
    return 0;
}



//=--------------------------------------------------------------------------=
// CErrorHelpDialog::OnOK
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
LRESULT CErrorHelpDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);
    FreeGDIResources();
    // Destroy dialog
    EndDialog(wID);

    return 0;
}



//=--------------------------------------------------------------------------=
// CErrorHelpDialog::OnCancel
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
LRESULT CErrorHelpDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);
    FreeGDIResources();
    // Destroy dialog
    EndDialog(wID);

    return 0;
}

// this message is sent each time a static control is drawn.
// we get the Control ID and then set background color and font
// as appropriate for that control.

LRESULT CErrorHelpDialog::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HDC hdc = (HDC) wParam;
    HWND hwnd = (HWND) lParam;

    int DlgCtrlID = ::GetDlgCtrlID(hwnd);

	if (DlgCtrlID == IDC_EBODY_TEXT) {
        if (m_hDialogFont == NULL){
            m_hDialogFont = CreateDialogFonts(hdc, TEXT("Arial"), 8, FALSE, FALSE);
        }

        ::SelectObject(hdc, m_hDialogFont);
        return 0;
    } else if (DlgCtrlID == IDC_MORE_INFO)  {
	if (m_hDialogUndelineFont == NULL) {
	    m_hDialogUndelineFont = CreateDialogFonts(hdc, TEXT("Arial"), 8, 0, TRUE);
	}
		::SelectObject(hdc, m_hDialogUndelineFont);
		::SetBkMode(hdc, TRANSPARENT);
		if (DlgCtrlID == IDC_MORE_INFO) {
			::SetTextColor(hdc, RGB(255, 0, 0));
			::SetBkMode(hdc, TRANSPARENT);
			return (LRESULT) GetStockObject(NULL_BRUSH);
	}
    }
    return (LONG)m_hDlgBrush1;
}

// Message handler for WM_SETCURSOR.
// the goal here is to set a "hand" cursor for the "More Information..." link
LRESULT CErrorHelpDialog::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
    POINT pt;
    RECT rect;

    ::GetCursorPos(&pt);
    ::GetWindowRect (GetDlgItem(IDC_MORE_INFO), &rect);

    if (::PtInRect(&rect, pt)) {
     	::SetCursor(::LoadCursor(NULL, IDC_HAND));
    }
    else {
     	::SetCursor(::LoadCursor(NULL, IDC_ARROW));
    }
    return TRUE;
}

LRESULT CErrorHelpDialog::ShowMoreInfo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    if (wID == IDC_MORE_INFO) {
	    ::LoadString(_Module.GetModuleInstance(), IDS_ERROR_MESSAGEURL, m_szUrl, BUFFER_SIZE);
		ShellExecute(NULL, "open", m_szUrl, NULL, NULL, SW_SHOWNORMAL);
    }
    return 0;
}

void CErrorHelpDialog::FreeGDIResources ()
{
    ::DeleteObject(m_hDialogBoldFont);
    m_hDialogBoldFont = NULL;

	::DeleteObject(m_hDialogUndelineFont);
    m_hDialogUndelineFont = NULL;

    ::DeleteObject(m_hDialogFont);
    m_hDialogFont = NULL;

    ::DeleteObject(m_hDlgBrush1);
    m_hDlgBrush1 = NULL;

    ::DeleteDC(m_hMemDC);
    m_hMemDC = NULL;
}

HFONT CErrorHelpDialog::CreateDialogFonts (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline)
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

    if (strncmp(szLocaleData, "Japan", 5) != 0)
	strcpy (lf.lfFaceName, szFaceName);
    else
	strcpy (lf.lfFaceName, TEXT("MS UI Gothic"));


    hFont = CreateFontIndirect(&lf);

    RestoreDC (hdc, iSavedDC);
    return hFont;
}

