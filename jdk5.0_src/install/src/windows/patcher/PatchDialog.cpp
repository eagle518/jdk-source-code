/*
 * @(#)PatchDialog.cpp	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//
// PatchDialog.cpp : Implementation of CPatchDialog
//
// By Stanley Man-Kit Ho
//

#include "stdafx.h"
#include <atlhost.h>
#include <commdlg.h>
#include <commctrl.h>
#include <windowsx.h>
#include <urlmon.h>
#include <wininet.h>
#include <stdio.h>
#include "PatchUtils.h"
#include "PatchDialog.h"


TCHAR CPatchDialog::m_szDirectory[BUFFER_SIZE];
TCHAR CPatchDialog::m_szOptions[BUFFER_SIZE];


CPatchDialog* g_pDlg = NULL;
int iNumOfFile = 1;
int iPatchedFiles = 0;

/////////////////////////////////////////////////////////////////////////////
// CPatchDialog

CPatchDialog::CPatchDialog()
{
    // Load up commctrl.dll
    InitCommonControls();

    m_hCancelEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hPatchUpdateThreadExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

    m_szDirectory[0] = NULL;
    m_szOptions[0] = NULL;
}


CPatchDialog::~CPatchDialog()
{
    ::CloseHandle(m_hCancelEvent);
    ::CloseHandle(m_hPatchUpdateThreadExitEvent);
}



//=--------------------------------------------------------------------------=
// CPatchDialog::OnInitDialog
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
LRESULT CPatchDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    // Change caption
    TCHAR szMsg[BUFFER_SIZE], szCaptionText[BUFFER_SIZE];

    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_UPDATE, szMsg, BUFFER_SIZE);
    wsprintf(szCaptionText, szMsg, "{" NEW_IMAGE_FULLVERSION "}");
    SetWindowText(szCaptionText);

    // Start animation
    Animate_Open(GetDlgItem(IDC_UPDATE_ANIMATE), MAKEINTRESOURCE(IDR_JAVAAVI));

    // Create a new thread for patch update
    DWORD dwThreadId = NULL;
    ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) PatchUpdateThreadProc, this, 0, &dwThreadId);
	
    return 1;  // Let the system set the focus
}


//=--------------------------------------------------------------------------=
// CPatchDialog::PatchUpdateThreadProc
//=--------------------------------------------------------------------------=
// Thread procedure for applying the patch
//
// Parameters:
//	lpParameter  Thread parameter
//
// Output:
//	DWORD	    
//
// Notes:
//
DWORD WINAPI CPatchDialog::PatchUpdateThreadProc(LPVOID lpParameter)
{
    CPatchDialog* pDlg = (CPatchDialog *) lpParameter;
    
    g_pDlg = pDlg;

    BOOL bSuccess = FALSE;

    do
    {
	// Check if download has been cancelled
	if (WAIT_OBJECT_0 == WaitForSingleObject(pDlg->m_hCancelEvent, 0))
	    break;

	bSuccess = ApplyPatch(m_szDirectory, m_szOptions, PatchUpdateCallBack);
    }
    while (0);


    // Post message to main thread to dis-miss dialog if users haven't hit cancel
    //
    if (WAIT_OBJECT_0 != WaitForSingleObject(pDlg->m_hCancelEvent, 0))
    {
	if (bSuccess)
	    ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDOK, NULL);
	else
	    ::PostMessage(pDlg->m_hWnd, WM_COMMAND, IDCANCEL, NULL);
    }


    // Signal main thread
    ::SetEvent(pDlg->m_hPatchUpdateThreadExitEvent);   

    return 0;
}

//=--------------------------------------------------------------------------=
// CPatchDialog::OnOK
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
LRESULT CPatchDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);

    // Signal another thread to stop patch update
    SetEvent(m_hCancelEvent);

    // Wait for the patch update thread to complete
    AtlWaitWithMessageLoop(m_hPatchUpdateThreadExitEvent);

    // Destroy dialog
    EndDialog(wID);

    return 0;
}


//=--------------------------------------------------------------------------=
// CPatchDialog::OnCancel
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
LRESULT CPatchDialog::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    // Disable window first to avoid any keyboard input
    EnableWindow(FALSE);

    // Signal another thread to stop patch update
    SetEvent(m_hCancelEvent);

    // Wait for the patch update thread to complete
    AtlWaitWithMessageLoop(m_hPatchUpdateThreadExitEvent);

    // Destroy dialog
    EndDialog(wID);

    return 0;
}


//=--------------------------------------------------------------------------=
// CPatchDialog::PatchUpdateCallBack
//=--------------------------------------------------------------------------=
// Called when patching is in progress
//
// Parameters:
//
// Output:
//
// Notes:
//
LPVOID WINAPI CPatchDialog::PatchUpdateCallBack(UINT id, LPVOID param)
{ 
    const char* szStatusText = (const char*) param;
    char szCaption[BUFFER_SIZE], szBuffer[BUFFER_SIZE], szMessage[BUFFER_SIZE];
    static char szFileEntry[BUFFER_SIZE];
    static char szWarningHeader[BUFFER_SIZE];

    // Check if download has been cancelled
    if (WAIT_OBJECT_0 == WaitForSingleObject(g_pDlg->m_hCancelEvent, 0))
	return NULL;

    HWND hWndStatus = ::GetDlgItem(g_pDlg->m_hWnd, IDC_PROGRESS_TEXT);
    HWND hWndProgress = ::GetDlgItem(g_pDlg->m_hWnd, IDC_PROGRESS_BAR);
  
    switch( id )
    {
	case 1:	  // Warning message header
	{
	    wsprintf(szWarningHeader, "%s", szStatusText);
	    break;
	}
	case 3:   // Error message header
	case 9:   // Progress message
	case 0xa: // Help message
	case 0xb: // Patch file comment
	case 0xc: // Set copyright string

	    // these are all text output...
	    break;

	case 2:
        {
	    // warning 
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_WARNING, szCaption, BUFFER_SIZE);

	    PATCH_TRACE(szStatusText);
	    PATCH_TRACE("\n");

	    // Fixed #4689257: Warning should not cause dialog to popup.
	    //
	    /*
	    if (IDCANCEL == ::MessageBox(g_pDlg->m_hWnd, szStatusText, szCaption, MB_ICONEXCLAMATION | MB_OKCANCEL))
		return NULL;
	    */		

	    // Popup warning dialog for most warnings, except
	    // wpt0015: Old File does not exist
	    // wpt0016: New File already exists
	    // wpt0024: New File already exists
	    // wpt0037: Error opening PATCH.ERR
	    //
	    if (strstr(szWarningHeader, "wpt0015") == NULL 
		&& strstr(szWarningHeader, "wpt0016") == NULL
		&& strstr(szWarningHeader, "wpt0024") == NULL
		&& strstr(szWarningHeader, "wpt0037") == NULL)
	    {
		wsprintf(szBuffer, "%s: %s", szFileEntry, szStatusText);

		::MessageBox(g_pDlg->m_hWnd, szBuffer, szCaption, MB_ICONEXCLAMATION | MB_OK);
		return NULL;
	    }
		    
	    break;		
	}

	case 4:
        {
	    // error
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ERROR, szCaption, BUFFER_SIZE);

	    ::MessageBox(g_pDlg->m_hWnd,  szStatusText, szCaption, MB_ICONSTOP | MB_OK);

	    PATCH_TRACE(szStatusText);
	    PATCH_TRACE("\n");

	    break;
	}

	case 5:
	    {
		// % complete
		//
		int iPercent = (int) ((long(0xffff & *((UINT *) param))*100L) / 0x8000L);		
		int iTotalPercent = (int) ((iPercent + iPatchedFiles * 100.0) / iNumOfFile * 0.9 + 10) ;

		::SendMessage(hWndProgress, PBM_SETPOS, (WPARAM) iTotalPercent, NULL);
		break;
	    }

	case 6:
	    // Number of patch files
	    iNumOfFile = *((int*)param);

	    break;

	case 7:
	    // File patch begin
	    {
		wsprintf(szFileEntry, "%s", szStatusText);

		::LoadString(_Module.GetResourceInstance(), IDS_STATUS_PROCESSING, szBuffer, BUFFER_SIZE);
		wsprintf(szMessage, szBuffer, szStatusText);

		::SetWindowText(hWndStatus, szMessage);

		// Set progress bar indicator to 0		    
		//::SendMessage(hWndProgress, PBM_SETPOS, 0, NULL);

		PATCH_TRACE(szMessage);
		PATCH_TRACE("\n");
	    }
	    break;

	case 8:
	    {
		// Current file patch complete
		::SetWindowText(hWndStatus, "");

		// Increment number of patched file
		iPatchedFiles = iPatchedFiles + 1;
	    }
	    break;

	case 0xd: // Patch file dialog
	case 0xe: // Invalid patch file alert
        case 0xf: // Password dialog
	case 0x10: // Invalid password alert
	case 0x11: // Next disk dialog
	case 0x12: // Invalid disk alert
	case 0x14: // Location dialog
        {
	    // this one shouldn't happen (only occurs if the patch file
	    //   file bound to the executable is invalid in some way).
	    ::LoadString(_Module.GetResourceInstance(), IDS_CAPTION_ABORT, szCaption, BUFFER_SIZE);
	    ::LoadString(_Module.GetResourceInstance(), IDS_ERROR_HANDLING, szBuffer, BUFFER_SIZE);
	    wsprintf(szMessage, szBuffer, id);

    	    PATCH_TRACE("Error: Location Dialog\n");

	    ::MessageBox(g_pDlg->m_hWnd, szMessage, szCaption, MB_ICONEXCLAMATION | MB_OK );

	    return NULL;
	}

	case 0x13:
	    // system location confirmation (always returns "Y" in this skeleton)
	    return "Y";
      
	case 0x16:
	{
	    ::LoadString(_Module.GetResourceInstance(), IDS_STATUS_SEARCHING, szMessage, BUFFER_SIZE);
	    ::SetWindowText(hWndStatus, szMessage);

    	    PATCH_TRACE(szMessage);
	    PATCH_TRACE("\n");

	    break;
	}
       
	case WM_BACKUP_BEGIN:
        {
	    ::LoadString(_Module.GetResourceInstance(), IDS_STATUS_BACKINGUP, szBuffer, BUFFER_SIZE);
	    wsprintf(szMessage, szBuffer, szStatusText);

	    ::SetWindowText(hWndStatus, szMessage);

	    break;
	}

	case WM_BACKUP_END:
        {
	    // Mark progress as 10% once backup is done
	    ::SendMessage(hWndProgress, PBM_SETPOS, (WPARAM) 10, NULL);
	    break;
	}

	case 0x15:
	default:
	    break;
    } 
  
    // do a few more messages while we're here...
  
    return "";
}
