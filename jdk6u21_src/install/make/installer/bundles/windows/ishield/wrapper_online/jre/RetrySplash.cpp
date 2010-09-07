/*
 * %W% %E%
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// RetrySplash.cpp : Implementation of CRetrySplashScreen
//
// By Paul Klingaman
//

#include "stdafx.h"
#include <atlhost.h>
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <math.h>
#include <shellapi.h>
#include "WelcomeDialog.h"
#include "common.h"
#include "RetrySplash.h"

#define BUFFER_SIZE 256

/////////////////////////////////////////////////////////////////////////////
// CRetrySplashScreen

CRetrySplashScreen::CRetrySplashScreen()
{
    m_hMemDC = NULL;
    m_hBitmap = NULL;
	m_hDialogFont = NULL;
    m_hDialogHeaderFont = NULL;
}

CRetrySplashScreen::~CRetrySplashScreen()
{
    EnableWindow(FALSE);
    FreeGDIResources();
}

LRESULT CRetrySplashScreen::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    
    // SetForegroundWindow() was deprecated for Vista (attn: session 0 isolation)
    // Use SetWindowPos() for Vista+ in order to bring the window to the top of the Z order
    if (IsPlatformWindowsVista()){
        SetWindowPos(HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);
        SetWindowPos(HWND_NOTOPMOST,0,0,0,0,SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    } else {
        if (::SetForegroundWindow(GetDlgItem(IDOK)) == NULL){
            return 1;
        }
    }
    
    return 0;
    
}

LRESULT CRetrySplashScreen::OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return (LRESULT) GetStockObject(WHITE_BRUSH);
}

LRESULT CRetrySplashScreen::OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    
    HDC hdc = (HDC) wParam;
    HWND hwnd = (HWND) lParam;
    
    int DlgCtrlID = ::GetDlgCtrlID(hwnd);
	
	if (DlgCtrlID == IDC_SPLASH_LINE1){
		
        if (m_hDialogFont == NULL){
			m_hDialogFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 10, FW_MEDIUM, FALSE);
        }
        
        ::SelectObject(hdc, m_hDialogFont);
        ::SetBkMode(hdc, TRANSPARENT);
        return (LRESULT) GetStockObject(WHITE_BRUSH);	
        
	} else if (DlgCtrlID == IDC_SPLASH_HEADER){
        
        if (m_hDialogHeaderFont == NULL){
            m_hDialogHeaderFont = CreateDialogFont(hdc, TEXT("Sun Sans"), 12, FW_BOLD, FALSE);
        }
        
        ::SelectObject(hdc, m_hDialogHeaderFont);
        ::SetBkMode(hdc, TRANSPARENT);
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
    }
    
    return (LRESULT) GetStockObject(WHITE_BRUSH);
    
}

void CRetrySplashScreen::FreeGDIResources()
{
    
    ::DeleteObject(m_hDialogHeaderFont);
    m_hDialogHeaderFont = NULL;
    
    ::DeleteObject(m_hDialogFont);
    m_hDialogFont = NULL;
    
    ::DeleteDC(m_hMemDC);
    m_hMemDC = NULL;
    
}

HFONT CRetrySplashScreen::CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline)
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

