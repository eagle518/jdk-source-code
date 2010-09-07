/*
 * @(#)SetupProgress.cpp    1.1 03/26/2009
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// SetupProgress.cpp : Implementation of CSetupProgressDialog

#include "stdafx.h"
#include <atlhost.h>
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <math.h>
#include <shellapi.h>
#include "SetupProgressDialog.h"
#include "common.h"

#define BUFFER_SIZE 256
#define PROGRESS_AMOUNT  8


/////////////////////////////////////////////////////////////////////////////
// CSetupProgressDialog

CSetupProgressDialog::CSetupProgressDialog()
{
    //progress bar initialization
    m_Progress    = 2;
    m_ProgressMax = 100;
    m_visible     = false;
    cAdvertisment = NULL;

    //Set the default path for APPDATA
    CSetupProgressDialog::setOpenOfficePath();

    // Load up commctrl.dll
    InitCommonControls();
}

/////////////////////////////////////////////////////////////////////////////
// ~CSetupProgressDialog

CSetupProgressDialog::~CSetupProgressDialog()
{
    if(cAdvertisment != NULL){
        //delete the reference to cAdvertisment
        delete cAdvertisment;
    }
}

//=--------------------------------------------------------------------------=
// CSetupProgressDialog::OnInitDialog
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
LRESULT CSetupProgressDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    m_hDlgBrush1 = CreateSolidBrush(RGB(231, 229, 213)); //default windows bg color
    m_hMemDC = NULL;
    m_hMemDCOO = NULL;
    m_hDialogFont = NULL;
    m_hDialogHeaderFont = NULL;


    HWND hWnd;
    // Set colors for the status bar
    hWnd = GetDlgItem(IDC_SETUP_PROGRESS);
    ::PostMessage(hWnd, PBM_SETBKCOLOR, NULL, (LONG) RGB(255, 255, 255)); 
    ::PostMessage(hWnd, PBM_SETBARCOLOR, NULL, (LONG) RGB(50, 205, 50));

    //Disable the close and minimize button
    LONG lStyle  = GetWindowLong(GWL_STYLE);
    SetWindowLong(GWL_STYLE, lStyle & ~WS_MAXIMIZEBOX );
    
    HMENU temp = this->GetSystemMenu(FALSE);
    EnableMenuItem(temp, SC_CLOSE ,MF_BYCOMMAND | MF_GRAYED); 

    // Create a new thread to wait for the event
    DWORD dwThreadId = NULL;
    //Pass in this instance of the Dialog
    ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) UpdateProgressBar, this, 0, &dwThreadId);
    this->ShowWindow(SW_HIDE);

    return 0;

}


LRESULT CSetupProgressDialog::OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LONG)m_hDlgBrush1;
}


// this message is sent each time a static control is drawn.
// we get the Control ID and then set background color and font
// as appropriate for that control.
//
LRESULT CSetupProgressDialog::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    HDC hdc = (HDC) wParam;
    HWND hwnd = (HWND) lParam;

    int DlgCtrlID = ::GetDlgCtrlID(hwnd);

    if (DlgCtrlID == IDC_BACK_COLOR){

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

    } else if ((DlgCtrlID == IDC_SETUP_TEXT)){

        if (m_hDialogFont == NULL){
            m_hDialogFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 8, FW_MEDIUM, FALSE);
        }

        ::SelectObject(hdc, m_hDialogFont);
        ::SetBkMode(hdc, TRANSPARENT);

        return (LRESULT) GetStockObject(NULL_BRUSH);

    }
    //check the various locales
    else if ( DlgCtrlID == IDC_ADVERTISMENT ){
        HMODULE hModule = ::LoadLibrary("gdiplus.dll");
	if(NULL != hModule){
            //only load this if it is not initialized
            if (m_hMemDCOO == NULL){
                cAdvertisment = new CImage();
                //use the CImage class to load the jpeg downloaded jpg
                if(((CImage *)cAdvertisment)->Load(this->m_pszOpenOfficePath) != S_OK){
                    ((CImage *)cAdvertisment)->LoadFromResource(_Module.GetModuleInstance(),
                                                    MAKEINTRESOURCE(IDI_OPENOFFICE_BANNER));
                }
                m_hMemDCOO = CreateCompatibleDC(NULL);
            }

            //Rectangle that contains the dimension of the window
            RECT rect;
            ::GetClientRect(hwnd, &rect);

            //CImage to draw the jpeg, with resizing
            ((CImage *)cAdvertisment)->BitBlt(hdc, 0, 0);
	}
        return (LRESULT) GetStockObject(NULL_BRUSH);
    }

    return (LONG)m_hDlgBrush1;

}

//Override the CWindow::ShowWindow call to ensure that 
//Window starts out as not visible. In Vista, the OnInitDialog will
//always end with a ShowWindow(SW_SHOW) call - this override prevents
//that call from making our dialog visible
BOOL CSetupProgressDialog::ShowWindow( int nCmdShow ){
    //Ensure that m_visible true before allowing a 
    //call to ShowWindow(SW_SHOW) to succeed
    if(nCmdShow == SW_SHOW && !m_visible){
        return false;
    }
    else{
        //else if its not a SW_SHOW option, just pass
        //the flag down to parent method
        return CWindow::ShowWindow(nCmdShow);
    }
}

//Thread that first waits for MSI install to start before making this dialog visible
//and then subsequently waiting for the JRE_SETUP_DIALOG_PROGRESS event to be fired
//by the MSI installer to update the progress bar 
DWORD WINAPI CSetupProgressDialog::UpdateProgressBar(LPVOID lpParameter){
    //grab a handle to the JRE_SETUP_MSI_STARTED event
    HANDLE myHandle = ::CreateEvent(NULL, TRUE, FALSE, JRE_SETUP_MSI_STARTED);
    if(myHandle == NULL){
        return 1;
    }

    //Wait for MSI install to start before setting dialog view
    ::WaitForSingleObject(myHandle, INFINITE);

    //Make dialog visible
    CSetupProgressDialog * dialogInstance = (CSetupProgressDialog *)lpParameter;
    dialogInstance->m_visible = true;
    dialogInstance->ShowWindow(SW_SHOW);
    //Set the Z ordering of the dialog
    if (IsPlatformWindowsVista()){
        //For vista use the SetWindowPos function
        dialogInstance->SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
        dialogInstance->SetWindowPos(HWND_NOTOPMOST,0,0,0,0,SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    } else {
        //For non vista just use the SetForeGround Window item
        ::SetForegroundWindow(dialogInstance->GetDlgItem(IDC_ADVERTISMENT));
    }
    ResetEvent(myHandle);

    //grab a handle to the JRE_SETUP_DIALOG_PROGRESS event triggered from the MSI
    myHandle = ::CreateEvent(NULL, TRUE, FALSE, JRE_SETUP_DIALOG_PROGRESS);
    if(myHandle == NULL){
        return 1;
    }

    //Loop while waiting for the event
    //When setup progress reaches the max, kill the dialog
    while(true){

        //Wait for installation progress without timing out
        ::WaitForSingleObject(myHandle, INFINITE);
        dialogInstance->m_Progress += PROGRESS_AMOUNT;
        HWND progressBar = dialogInstance->GetDlgItem(IDC_SETUP_PROGRESS);
        ::PostMessage((HWND)progressBar, PBM_SETPOS,(WPARAM)(dialogInstance->m_Progress), NULL);

        //Reset the event handle
        ResetEvent(myHandle);

        //if the toolbar is full, close the window
        if(dialogInstance->m_Progress >= dialogInstance->m_ProgressMax){
            //hide
            dialogInstance->FreeGDIResources();
            dialogInstance->ShowWindow(SW_HIDE);
            break;
        }
    }
    return 0;
}

void CSetupProgressDialog::FreeGDIResources()
{
    ::DeleteObject(m_hDialogHeaderFont);
    m_hDialogHeaderFont = NULL;

    ::DeleteObject(m_hDialogFont);
    m_hDialogFont = NULL;

    ::DeleteObject(m_hDlgBrush1);
    m_hDlgBrush1 = NULL;

    ::DeleteDC(m_hMemDC);
    m_hMemDC = NULL;

    ::DeleteDC(m_hMemDCOO);
    m_hMemDC = NULL;
}


HFONT CSetupProgressDialog::CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline)
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

