/*
 * @(#)ChangeFolder.h	1.0 00/00/2009
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// ChangeFolder.h : Declaration of the CChangeFolder
//

#ifndef __CHANGEFOLDER_H_
#define __CHANGEFOLDER_H_

#pragma once

#include "resource.h"       // main symbols


// CChangeFolder

class CChangeFolder : 
    public CAxDialogImpl<CChangeFolder>
{
public:
    CChangeFolder();

    enum { IDD = IDD_CHANGEFOLDER };

BEGIN_MSG_MAP(CChangeFolder)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_CTLCOLORDLG, OnCtlColorDlg)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
    COMMAND_ID_HANDLER(IDOK, OnNext)
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
    COMMAND_ID_HANDLER(IDC_CHANGE_BUTTON, OnBnClickedChangeButton)
    COMMAND_ID_HANDLER(IDC_BACK_BUTTON, OnBack)
END_MSG_MAP()

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNext(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnBack(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCtlColorDlg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnBnClickedChangeButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    void setBodyText(int iBodyText)
    {
        m_iBodyText = iBodyText;
    }

    LRESULT getInstallDir(LPTSTR lpszInstallDir) {
        strcpy(lpszInstallDir, m_szInstallDir);
        return ERROR_SUCCESS;
    }

    LRESULT setInstallDir(LPTSTR lpszInstallDir) {
        strcpy(m_szInstallDir, lpszInstallDir);
        return ERROR_SUCCESS;
    }

    private:
        HFONT   CreateDialogFont (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline);
        void    FreeGDIResources ();

        TCHAR   m_szInstallDir[MAX_PATH];
        int     m_iBodyText;
        HDC	m_hMemDC;
        HBITMAP m_hBitmap;
        BITMAP	m_bmBanner;
        HBRUSH	m_hDialogBrush;
        HFONT   m_hDialogFont;
        HFONT   m_hDialogFont_Directory;
        HFONT   m_hDialogHeaderFont;
};

#endif //__CHANGEFOLDER_H_
