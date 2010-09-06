/*
 * @(#)PatchDialog.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// PatchDialog.h : Declaration of the CPatchDialog
//
// By Stanley Man-Kit Ho
//

#ifndef __PATCHDIALOG_H_
#define __PATCHDIALOG_H_

#include "resource.h"       // main symbols


/////////////////////////////////////////////////////////////////////////////
// CPatchDialog
class CPatchDialog : 
	public CAxDialogImpl<CPatchDialog>
{
public:
	CPatchDialog();
	~CPatchDialog();

	enum { IDD = IDD_PROGRESS_DIALOG };

BEGIN_MSG_MAP(CDownloadDialog)
	MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	COMMAND_ID_HANDLER(IDOK, OnOK)
	COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	void setCommandLine(LPCTSTR pszDirectory, LPCTSTR pszOptions)
	{
	    m_szDirectory[0] = NULL;
	    m_szOptions[0] = NULL;
	    
	    if (pszDirectory)
		wsprintf(m_szDirectory, "%s", pszDirectory);

	    if (pszOptions)
		wsprintf(m_szOptions, "%s", pszOptions);
	}

	static DWORD WINAPI PatchUpdateThreadProc(LPVOID lpParameter);
	static LPVOID WINAPI PatchUpdateCallBack(UINT Id, LPVOID Parm);

private:
	static TCHAR m_szDirectory[BUFFER_SIZE];
	static TCHAR m_szOptions[BUFFER_SIZE];
	HANDLE	m_hCancelEvent;
	HANDLE	m_hPatchUpdateThreadExitEvent;
};

#endif //__PATCHDIALOG_H_
