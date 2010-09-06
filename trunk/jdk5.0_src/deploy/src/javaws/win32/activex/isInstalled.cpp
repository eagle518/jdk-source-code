/*
 * @(#)isInstalled.cpp	1.9 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// isInstalled.cpp : Implementation of CisInstalled
#include <tchar.h>
#include "StdAfx.h"
#include "JavaWebStart.h"
#include "isInstalled.h"

extern CComModule _Module;
/////////////////////////////////////////////////////////////////////////////
// CisInstalled

STDMETHODIMP CisInstalled::get_app(BSTR *pVal)
{
	*pVal = m_app.Copy();
	return S_OK;
}

STDMETHODIMP CisInstalled::put_app(BSTR newVal)
{
	m_app = newVal;
	return S_OK;
}

STDMETHODIMP CisInstalled::get_dbg(BSTR *pVal)
{
	*pVal = m_dbg.Copy();
	return S_OK;
}

STDMETHODIMP CisInstalled::put_dbg(BSTR newVal)
{
	m_dbg = newVal;
	return S_OK;
}

STDMETHODIMP CisInstalled::get_back(BSTR *pVal)
{
	*pVal = m_back.Copy();
	return S_OK;
}

STDMETHODIMP CisInstalled::put_back(BSTR newVal)
{
	m_back = newVal;
	return S_OK;
}

STDMETHODIMP CisInstalled::GetInterfaceSafetyOptions(REFIID riid, 
									 DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
	ATLTRACE(_T("IObjectSafetyImpl::GetInterfaceSafetyOptions\n"));
	if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
		return E_POINTER;
	*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER || INTERFACESAFE_FOR_UNTRUSTED_DATA;
	*pdwEnabledOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER || INTERFACESAFE_FOR_UNTRUSTED_DATA;
	return S_OK;
}

STDMETHODIMP CisInstalled::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{
	ATLTRACE(_T("IObjectSafetyImpl::SetInterfaceSafetyOptions\n"));
	return S_OK;
}

HRESULT CisInstalled::OnDraw(ATL_DRAWINFO& di)
{
//	RECT& rc = *(RECT*)di.prcBounds;
	return S_OK;
}

STDMETHODIMP CisInstalled::Close(DWORD dwSaveOption) {
	m_closed = TRUE;
	return (HRESULT)IOleObjectImpl<CisInstalled>::Close(dwSaveOption);
}

STDMETHODIMP CisInstalled::InPlaceActivate(LONG iVerb, const RECT* prcPosRect) {
	HRESULT hres;
	TCHAR javawscmd[MAX_PATH], cmdline[MAX_PATH];
	char mbsval[MAX_PATH];
	DWORD dwType, cbData = MAX_PATH;
	LPTSTR p;
	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;
	HKEY hKey;

	if (FAILED(hres = CComControl<CisInstalled>::InPlaceActivate(iVerb, prcPosRect))) return hres;

	if (m_app) {
	  // find JavaWS path registry key
	  if (SUCCEEDED(RegOpenKeyEx(HKEY_CLASSES_ROOT, JAVAWS_KEY, 0, KEY_QUERY_VALUE, &hKey))) {
	    if (SUCCEEDED(RegQueryValueEx(hKey, NULL, NULL, &dwType, (UCHAR *)javawscmd, &cbData))) {
	      // value now contains "<path to javaws.exe>" "%1"
	      // chop it at second quote
	      p = _tcschr(javawscmd, '\"');
	      if (p!= NULL) p = _tcschr(++p, '\"');
	      if (p!= NULL) {
		LPTSTR lpCmdStr = NULL;
		
		*++p = 0;
		_tcscat(javawscmd, _T(" "));
		
		_tcscpy(cmdline, javawscmd);
#ifdef UNICODE
		lpCmdStr = m_app;
#else
		WideCharToMultiByte(CP_ACP, 0, m_app, -1, mbsval, MAX_PATH, NULL, NULL);
		lpCmdStr = mbsval;
#endif
		// 4782008: disallow spaces, tabs, newlines, and leading
		// "-" in argument
		if (_tcschr(lpCmdStr, ' ') == NULL &&
		    _tcschr(lpCmdStr, '\t') == NULL &&
		    _tcschr(lpCmdStr, '\r') == NULL &&
		    _tcschr(lpCmdStr, '\n') == NULL &&
		    lpCmdStr[0] != '-') {
		  // 4782008: guard against buffer overrun
		  _tcsncat(cmdline, lpCmdStr,
			   MAX_PATH - lstrlen(cmdline) - 1);
		}
		if (m_dbg) {
		  ::MessageBox(NULL, cmdline, "Launch Command line", 0);
		}
		ZeroMemory((LPVOID)&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		// Start JavaWS
		if (SUCCEEDED(
			      CreateProcess(NULL,     // app name
					    cmdline,      
					    NULL,		// process security attributes
					    NULL,		// thread security attributes
					    FALSE,		// handle inheritance flag
					    0,			// creation flags
					    NULL,       // environment block
					    NULL,       // current directory name
					    &startupInfo, &processInfo))) {
		  // don't need to wait for process to complete
		  if (processInfo.hProcess != NULL) {
		    if (processInfo.hThread != NULL) CloseHandle(processInfo.hThread);
		    CloseHandle(processInfo.hProcess);
		  }
		}
	      }
	    }
	    RegCloseKey(hKey);
	  }
	}


	if (m_back) {
		WideCharToMultiByte(CP_ACP, 0, m_back, -1, mbsval, MAX_PATH, NULL, NULL);
	}
	if (!m_back || stricmp(mbsval, "false") != 0) {
		// don't go back if not still open because of bug in urlmon.dll
		if (m_closed == FALSE) {
			HlinkGoBack(GetUnknown());
		}
	}	

	return hres;
}
