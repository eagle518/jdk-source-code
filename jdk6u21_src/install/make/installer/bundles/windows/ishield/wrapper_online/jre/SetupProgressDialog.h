/*
 * @(#)SetupProgressDialog.h    
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// SetupProgressDialog.h : Definition of CSetupProgressDialog
//
// By Wai Yong Low
//

#pragma once
#include "resource.h" // main symbols
#include "userprofile.h"
#include <atlstr.h>
#include <atlimage.h>
#include "PluginVersion.h"
#define JRE_SETUP_DIALOG_PROGRESS "JreSetupDialogProgress"
#define JRE_SETUP_MSI_STARTED     "JreSetupMsiStarted"
#define BUFFER_SIZE 2048

class CSetupProgressDialog : 
    public CAxDialogImpl<CSetupProgressDialog>
{
public:
    CSetupProgressDialog(void);
   ~CSetupProgressDialog(void);

    enum { IDD = IDD_SETUP_PROGRESS_DIALOG };

    BEGIN_MSG_MAP(CSetupProgressDialog)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
        MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
    END_MSG_MAP()

    //Overrides the default ShowWindow command so that the dialog can be initialized
    //in hidden view
    BOOL ShowWindow( int nCmdShow );
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //Thread to perform progress bar update
    static DWORD WINAPI UpdateProgressBar( LPVOID lpParam );
    //Saves a local copy of the path to OpenOfficeBanner path in JRE versions' directory in APPDATA
    void setOpenOfficePath(void){
        //Get the local APPDATA path
        TCHAR szAppDataDir[BUFFER_SIZE] = {0};
        GetUserShellFolder(szAppDataDir); 
        
        //Create the correct Path, and save it in m_pszAppDataPath
        wsprintf(m_pszOpenOfficePath, "%s\\Sun\\Java\\jre%s\\%s.jpg", szAppDataDir ,VERSION,
                    SPONSOR_DEFAULT_STRING);
    }

    //The variable that represents the progress bar level
    ULONG           m_Progress;
    //The variable that represents the maximum bar level
    ULONG           m_ProgressMax;
    //A flag that needs to be set to true for
    //ShowWindow(SW_SHOW) to work
    bool            m_visible;

    private:
    HFONT CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline);
    void  FreeGDIResources ();
    //TCHAR buffers
    TCHAR   m_pszOpenOfficePath[BUFFER_SIZE];    //appdata path

    //BITMAP objects for TOP JAVA Banner
    HDC     m_hMemDC; //Handle to the GDI
    HBITMAP m_hBitmap; //Reference to the HBITMAP
    BITMAP  m_bmBannerJFX; //Reference to the actual BITMAP
    //HFONTS for the dialog box
    HFONT   m_hDialogFont;
    HFONT   m_hDialogHeaderFont;
    //BITMAP objects for the Sponsor banner
    HDC     m_hMemDCOO; //Handle to the GDI for open office
    HBITMAP m_hBitmapOO; //Reference to the HBITMAP for open ofofice
    void*  cAdvertisment; //Reference to the CImage used to open the sponsor bitmap
    //Brush for dialog box
    HBRUSH  m_hDlgBrush1;
    
};
