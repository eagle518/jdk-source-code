/*
 * @(#)ErrorHelpDialog.h	1.1 06/01/19
 *
 * Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// ErrorHelpDialog.h : Declaration of the CErrorHelpDialog
//

#ifndef __ERRORHELP_DIALOG_H_
#define __ERRORHELP_DIALOG_H_

#include "resource.h"       // main symbols

// IDC_HAND is defined in winUser.h, but only if (WINVER >= 0x0500),
// which is not the case in our builds.

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

/////////////////////////////////////////////////////////////////////////////
// CErrorHelpDialog
//
class CErrorHelpDialog : 
	public CAxDialogImpl<CErrorHelpDialog>
{
public:
	CErrorHelpDialog();

	enum { IDD = IDD_ERROR_HELP_DIALOG };

BEGIN_MSG_MAP(CErrorHelpDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnCtlColorStatic)
	MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	COMMAND_ID_HANDLER(IDC_MORE_INFO, ShowMoreInfo)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCtlColorStatic(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT ShowMoreInfo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void setBodyText(int iBodyText)
	{
            m_iBodyText = iBodyText;
	}
	
    private:
	HFONT CreateDialogFonts (HDC hdc, TCHAR *szFaceName, int ptSize, DWORD dwWeight, BOOL bUnderline);
	void  FreeGDIResources ();
	TCHAR	m_szDescription[1024];
	TCHAR	m_szUrl[256];
	int                     	m_iBodyText;
	HDC	m_hMemDC;
	BITMAP	m_bmJavaCup;
	HBRUSH	m_hDlgBrush1;
	HFONT   m_hDialogFont;
	HFONT	m_hDialogUndelineFont;
	HFONT   m_hDialogBoldFont;
};

#endif //__ERRORHELP_DIALOG_H_
