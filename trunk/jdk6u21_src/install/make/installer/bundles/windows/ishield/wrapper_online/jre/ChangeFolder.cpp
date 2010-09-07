/*
 * @(#)ChangeFolder.cpp	1.1 03/26/2009
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// ChangeFolder.cpp : Implementation of CChangeFolder

#include "stdafx.h"
#include <atlhost.h>    // COINIT_MULTITHREADED
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <math.h>       // fabs()
#include <shellapi.h>
#include <shlobj.h>     // BROWSEINFO

#include "WrapperUtils.h"   // BUFFER_SIZE
#include "ChangeFolder.h"
#include "common.h"
#include "UpdateConf.hpp"   // logit()


/////////////////////////////////////////////////////////////////////////////
// CChangeFolder

CChangeFolder::CChangeFolder()
{
    // initialize m_szInstallDir once and allow it to be set before displaying dialog
    strcpy(m_szInstallDir, "");

    CRegKey WindowsKey;

    if (WindowsKey.Open(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", KEY_READ) == ERROR_SUCCESS) {
        TCHAR szProgramPath[MAX_PATH + 1] = {NULL};
        DWORD dwCount = MAX_PATH + 1;

        if (WindowsKey.QueryValue(szProgramPath, "ProgramFilesDir", &dwCount) == ERROR_SUCCESS) {
            wsprintf(m_szInstallDir, "%s\\Java\\jre6\\", szProgramPath );
        }
    }
    // if error, the default InstallDir will be empty

    // Load up commctrl.dll
    InitCommonControls();
}


//=--------------------------------------------------------------------------=
// CChangeFolder::OnInitDialog
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
LRESULT CChangeFolder::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_hMemDC = NULL;
    m_hDialogFont = NULL;
    m_hDialogFont_Directory = NULL;
    m_hDialogHeaderFont = NULL;
    m_hBitmap = NULL;

    m_hDialogBrush = CreateSolidBrush(RGB(231, 229, 213)); //default windows bg color

    // Note: if SetForegroundWindow(GetDlgItem(IDOK) is called (on XP)
    // the welcome dialog will not be displayed if the back button is clicked and the program exists

    return 0;
}


//=--------------------------------------------------------------------------=
// CChangeFolder::OnNext
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

LRESULT CChangeFolder::OnNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{

    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);

    FreeGDIResources();
  
    // Destroy dialog
    EndDialog(wID);

    return 0;
}


//=--------------------------------------------------------------------------=
// CChangeFolder::OnCancel
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
LRESULT CChangeFolder::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // First display an "are you sure you want to cancel" prompt
    TCHAR szAskMsg[BUFFER_SIZE] = {NULL};
    TCHAR szCaption[BUFFER_SIZE] = {NULL};
    TCHAR szMsg[BUFFER_SIZE] = {NULL};

    ::LoadString(_Module.GetResourceInstance(), IDS_AREYOUSURE_MESSAGE, szAskMsg, BUFFER_SIZE);
    ::LoadString(_Module.GetResourceInstance(), IDS_AREYOUSURE_CAPTION, szCaption, BUFFER_SIZE);
    wsprintf(szMsg, szAskMsg, VERSION);

    if (MessageBox(szMsg, szCaption, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) != IDYES) {
        return 0;
    }
    
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);
    FreeGDIResources();

    // Destroy dialog
    EndDialog(wID);

    return 0;
}


LRESULT CChangeFolder::OnBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);
    FreeGDIResources();

    // Destroy dialog
    EndDialog(IDC_BACK_BUTTON);

    return 0;
}


LRESULT CChangeFolder::OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LONG)m_hDialogBrush;
}


// this message is sent each time a static control is drawn.
// we get the Control ID and then set background color and font
// as appropriate for that control.

LRESULT CChangeFolder::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HDC hdc = (HDC) wParam;
    HWND hwnd = (HWND) lParam;

    int DlgCtrlID = ::GetDlgCtrlID(hwnd);
	
    if ((DlgCtrlID == IDC_DIALOG_TEXT) || (DlgCtrlID == IDC_DIALOG_TEXT2)) {
	    
        if (m_hDialogFont == NULL){
            m_hDialogFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 10, FW_MEDIUM, FALSE);
        }
         
        ::SelectObject(hdc, m_hDialogFont);
        return (LRESULT) GetStockObject(WHITE_BRUSH);	

    } else if (DlgCtrlID == IDC_DIRECTORY) {
	    
        if (m_hDialogFont_Directory == NULL){
            m_hDialogFont_Directory = CreateDialogFont(hdc, TEXT("Sun Sans"), 8, FW_MEDIUM, FALSE);
        }
         
        ::SetWindowText(hwnd, m_szInstallDir);
        
        ::SelectObject(hdc, m_hDialogFont_Directory);
        return (LRESULT) GetStockObject(WHITE_BRUSH);	

    } else if (DlgCtrlID == IDC_DIALOG_HEADER){

        TCHAR szHeaderText[BUFFER_SIZE] = {NULL};
        
        if (m_hDialogHeaderFont == NULL){
            m_hDialogHeaderFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 12, FW_BOLD, FALSE);
        }
        
//        ::LoadString(_Module.GetResourceInstance(), IDS_WELCOME_MAIN, szHeaderText, BUFFER_SIZE);
//        ::SetWindowText(hwnd, szHeaderText);
        
        ::SelectObject(hdc, m_hDialogHeaderFont);
        ::SetBkMode(hdc, TRANSPARENT);
        return (LONG)m_hDialogBrush;

    } else if (DlgCtrlID == IDC_BACK_COLOR){

        return (LRESULT) GetStockObject(WHITE_BRUSH);
        
    } else if (DlgCtrlID == IDC_BANNER){

     	if (m_hMemDC == NULL) {
            m_hBitmap = LoadBitmap(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDI_BANNER));
            GetObject(m_hBitmap, sizeof(BITMAP), &m_bmBanner);
            m_hMemDC = CreateCompatibleDC(NULL);
            SelectObject(m_hMemDC, m_hBitmap);
	}

	RECT rect;
	::GetClientRect(hwnd, &rect);
	StretchBlt( hdc, rect.left, rect.top, (rect.right - rect.left), (rect.bottom - rect.top),
                    m_hMemDC, 0, 0, m_bmBanner.bmWidth, m_bmBanner.bmHeight, SRCCOPY);

	return (LRESULT) GetStockObject(NULL_BRUSH);
    }

    return (LONG)m_hDialogBrush;
}


void CChangeFolder::FreeGDIResources()
{
    ::DeleteObject(m_hDialogHeaderFont);
    m_hDialogHeaderFont = NULL;

    ::DeleteObject(m_hDialogFont);
    m_hDialogFont = NULL;

    ::DeleteObject(m_hDialogFont_Directory);
    m_hDialogFont_Directory = NULL;

    ::DeleteObject(m_hDialogBrush);
    m_hDialogBrush = NULL;

    ::DeleteDC(m_hMemDC);
    m_hMemDC = NULL;
}


// Create the fonts we need for the dialog
HFONT CChangeFolder::CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline)
{
    int iSavedDC = SaveDC(hdc);

    SetGraphicsMode (hdc, GM_ADVANCED);
    ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
    SetViewportOrgEx (hdc, 0,0, NULL);
    SetWindowOrgEx (hdc, 0,0, NULL);

    FLOAT cxDPI = (FLOAT) GetDeviceCaps(hdc, LOGPIXELSX);
    FLOAT cyDPI = (FLOAT) GetDeviceCaps(hdc, LOGPIXELSY);

    int iDeciPtWidth = 0;
    int iDeciPtHeight = 10 * ptSize;

    POINT pt;
    pt.x = (int) (iDeciPtWidth * cxDPI / 72);
    pt.y = (int) (iDeciPtHeight * cyDPI / 72);

    DPtoLP(hdc, &pt, 1);

    LOGFONT lf;
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

    TCHAR szLocaleData[BUFFER_SIZE] = {NULL};

    int iRet = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGCOUNTRY, szLocaleData, BUFFER_SIZE);

    if (strncmp(szLocaleData, "Japan", 5) != 0){
	strcpy (lf.lfFaceName, szFaceName);
    } else {
	strcpy (lf.lfFaceName, TEXT("MS UI Gothic"));
    }

    HFONT hFont = CreateFontIndirect(&lf);

    RestoreDC (hdc, iSavedDC);

    return hFont;
}


LRESULT CChangeFolder::OnBnClickedChangeButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);

    // We don't have the current headers / libraries to support this, BUT:
    // For Windows Vista or later, it is recommended that you use IFileDialog 
    // with the FOS_PICKFOLDERS option rather than the SHBrowseForFolder function. 
    // This uses the Open Files dialog in pick folders mode and is the preferred implementation.

    // You must initialize Component Object Model (COM) before you call SHBrowseForFolder. 
    // CoInitialize is currently called in _tWinMain in jinstall.cpp

    BROWSEINFO bi = { 0 };
    LPITEMIDLIST pidl = NULL;
    TCHAR szDisplayName[MAX_PATH] = {NULL};
    TCHAR szTitle[BUFFER_SIZE] = {NULL};

    bi.hwndOwner = hWndCtl; 
    ::LoadString(_Module.GetResourceInstance(), IDS_BROWSE_TITLE, szTitle, BUFFER_SIZE);
    bi.lpszTitle = szTitle;

    bi.pszDisplayName = szDisplayName;

    // The documentation says:
    //    If COM is initialized using CoInitializeEx with the COINIT_MULTITHREADED flag, 
    //    SHBrowseForFolder fails if the calling application uses the 
    //    BIF_USENEWUI or BIF_NEWDIALOGSTYLE flag in the BROWSEINFO structure.
    // but I have not seen a problem with BIF_NEWDIALOGSTYLE (I did with BIF_USENEWUI)
    // and it is needed to get a Make New Folder button.

    bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    // could display an edit box in the browse dialog box
    // bi.ulFlags |= BIF_EDITBOX;

    // setting the root means the user can only install under that folder
    // CSIDL_PROGRAM_FILES would only allow install under Program Files
    // set to CSIDL_DRIVES (drives under My Computer) instead of the default My Computer
    {
        LPITEMIDLIST pidlRoot = NULL;

        if (SHGetFolderLocation(hWndCtl, CSIDL_DRIVES , NULL, 0, &pidlRoot) == S_OK) {
            bi.pidlRoot = pidlRoot;
        }
    }

    pidl = SHBrowseForFolder(&bi);
    if (pidl == NULL) {
        // if pidl is NULL, user cancelled browse for folder
    } else {
        // get the name of the folder
        TCHAR szPath[MAX_PATH] = {NULL};

        if ( SHGetPathFromIDList(pidl, szPath) ) {
            lstrcpy(m_szInstallDir, szPath);

            TCHAR szMessage[BUFFER_SIZE] = {NULL};
            wsprintf(szMessage, "Selected Folder: %s\n", szPath);
            logit(szMessage);

            // update directory text
            HWND hDirectoryText = GetDlgItem(IDC_DIRECTORY);
            ::SetWindowText(hDirectoryText, m_szInstallDir);
        }

        // free the item id list
        CoTaskMemFree(pidl);
    }

    EnableWindow(TRUE);
// try after window enabled
//    SendDlgItemMessage(IDC_DIRECTORY, WM_CTLCOLORSTATIC, 0, 0);

    return 0;
}

