/*
 * @(#)isInstalled.cpp	1.18 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// isInstalled.cpp : Implementation of CisInstalled
#include <tchar.h>
#include <stdio.h>
#include "StdAfx.h"
#include "wsdetect.h"
#include "isInstalled.h"

#define DNS_HOSTNAME_MAX_LENGTH 255
#define IP_ADDRESS_MAX_LENGTH 256

//=--------------------------------------------------------------------------=
//  WarmupWinsock() 
//=--------------------------------------------------------------------------=
// Called to load the winsock library and its suppporting naming libraries
// This is a workaround for bug 4241800 on Windows 98.
// Under some circumstances windows 98 deadlocks internally if we initialize
// winsock inside IE4 from inside the depths of the Java system.
// To avoid the deadlock, we initialize winsock before starting Java.
typedef int (PASCAL FAR *WSAStartupType)(WORD, LPWSADATA);
typedef int (PASCAL FAR *gethostnameType)(char FAR *, int); 
typedef hostent FAR* (PASCAL FAR *gethostbynameType)(const char FAR*);

WSACleanupType do_WSACleanup = NULL;

gethostbynameType do_gethostbyname = NULL;

extern CComModule _Module;

void WarmupWinsock()
{
    // Load wsock32.dll
    HINSTANCE wsock = LoadLibrary("wsock32.dll");
    if (wsock == NULL) {
	return;
    }

    // Initialize winsock
    WSAStartupType do_WSAStartup = (WSAStartupType)GetProcAddress(wsock, "WSAStartup");
    if (do_WSAStartup == NULL) {
	return;
    }

    // Get the WSACleanup function pointer used when the process get detached
    // the function pinter of cleanup needs to get before WSAStartup called
    do_WSACleanup = (WSACleanupType)GetProcAddress(wsock, "WSACleanup");
    if (do_WSACleanup == NULL) {
	return;
    }

    WSADATA wsa;
    (*do_WSAStartup)(MAKEWORD(1,1), &wsa);
    
    // Call gethostname.  This initializes the naming libraries
    // Note: we ignore the return value.
    gethostnameType do_gethostname = (gethostnameType)GetProcAddress(wsock, "gethostname");
    if (do_gethostname == NULL) {
	return;
    }
    char hostname[256];
    (*do_gethostname)(hostname, sizeof hostname);

    // Get the function pointer of gethostbyname for later use by AxControl::DnsResolve
    // This provides the capablity to do dns look up for PAC file. 
    do_gethostbyname = (gethostbynameType)GetProcAddress(wsock, "gethostbyname");
}

char* ConvertBSTRToLPSTR(BSTR bstrIn) {
    char* pszOut = NULL;
    
    if (bstrIn == NULL) {
        return NULL;
    }
        
    int nInputStrLen = SysStringLen(bstrIn);
    
   
    int nOutputStrLen = WideCharToMultiByte(CP_ACP, 0, bstrIn, nInputStrLen, 
            NULL, 0, 0, 0);
    
    if (nOutputStrLen == 0) {
        return NULL;
    }
    
    // Double NULL Termination
    nOutputStrLen += 2;
    
    pszOut = (char*)calloc(nOutputStrLen, sizeof(char));
    
    if (pszOut == NULL) {
        return NULL;
    }
    
    int ret = WideCharToMultiByte(CP_ACP, 0, bstrIn, nInputStrLen, pszOut,
            nOutputStrLen, 0, 0);
    
    if (ret == 0) {
        return NULL;
    }
    
    return pszOut;
}

OLECHAR* ConvertAnsiToUnicode(char * strIn) {
    if (strIn == NULL) {
        return NULL;
    }
    
    int nInputStrLen = strlen(strIn);
       
    int nOutputStrLen = MultiByteToWideChar(CP_ACP, 0, strIn, nInputStrLen, 
            NULL, 0);
    
    if (nOutputStrLen == 0) {
        return NULL;
    }
    
    // Double NULL Termination
    nOutputStrLen += 2;

    unsigned short *bufU = (unsigned short*)calloc(nOutputStrLen, 
            sizeof(unsigned short));
    
    if (bufU == NULL) {
        return NULL;
    }
    
    int ret = MultiByteToWideChar(CP_ACP, 0, strIn, nInputStrLen, bufU, 
            nOutputStrLen);
    
    if (ret == 0) {
        return NULL;
    }
    
    return bufU;
}

//=--------------------------------------------------------------------------=
// CisInstalled::DnsResolve
//=--------------------------------------------------------------------------=
//
// Parameters:
//    DISPPARAMS* pParams : pointer to host name string
//    LPVARIANT   pVarRes : pointer to the IP address of that host name passed in
// Return Value:		  
//    HRESULT indicate whether DnsResolve success or not
//    
// Remarks: 
//    The only reason of failure is the "WarmupWinsock()" call may
//    fail. If the host name can not resolved, we still return S_OK due to 
//    Jscript may not catch the exception (usual case).
//
STDMETHODIMP CisInstalled::dnsResolve(BSTR hostName, BSTR* ipAddress) 
{
	USES_CONVERSION;

    UINT hostNameLen = SysStringByteLen(hostName);
    // make sure the hostname length is valid
    if (hostNameLen > DNS_HOSTNAME_MAX_LENGTH) {
        return E_FAIL;
    }

	// Load Windows socks library
    WarmupWinsock();

    // do_gethostbyname is a function pointer initialized from DllServer::WarmpWinsock
    if (do_gethostbyname == NULL)
	return E_FAIL; // fail to start up windows socket
    
    char * lpHostName = NULL;
    
    lpHostName = ConvertBSTRToLPSTR(hostName);
    
    if (lpHostName == NULL) {
        return E_FAIL;
    }

    hostent* result = do_gethostbyname(lpHostName);
    
    free(lpHostName);
    
    if (result)
    {
        char  ipAddr[IP_ADDRESS_MAX_LENGTH]; // containning final IP address
        char  temp[IP_ADDRESS_MAX_LENGTH];
        int ret = 0;
        int templen = 0;
	ipAddr[0] = '\0';
	int length = result->h_length;
	// Using 'for' loop is because the IP may be IPv6
        for(int i = 0; i < length - 1; i++) {
            ret = _snprintf(temp, IP_ADDRESS_MAX_LENGTH, "%u.", 
                    (unsigned char)(*result->h_addr_list)[i]);
            // if ret == IP_ADDRESS_MAX_LENGTH, string is not NULL terminated
            if (ret < 0 || ret == IP_ADDRESS_MAX_LENGTH) {
                return E_FAIL;
            }
            templen = strlen(temp);
            // include NULL terminator
            if ((strlen(ipAddr) + templen + 1) > IP_ADDRESS_MAX_LENGTH) {
                return E_FAIL;
            }
            strncat(ipAddr, temp, templen);
        }
     
        ret = _snprintf(temp, IP_ADDRESS_MAX_LENGTH, "%u",
                (unsigned char)(*result->h_addr_list)[length - 1]);
        // if ret == IP_ADDRESS_MAX_LENGTH, string is not NULL terminated
        if (ret < 0 || ret == IP_ADDRESS_MAX_LENGTH) {
            return E_FAIL;
        }
        
        templen = strlen(temp);
        // include NULL terminator
        if ((strlen(ipAddr) + templen + 1) > IP_ADDRESS_MAX_LENGTH) {
            return E_FAIL;
        }
        
        strncat(ipAddr, temp, templen);

        OLECHAR* wIpAddr = NULL;
        
        wIpAddr = ConvertAnsiToUnicode(ipAddr);
        
        if (wIpAddr == NULL) {
            return E_FAIL;
        }

	*ipAddress = SysAllocString(wIpAddr);
        
        free(wIpAddr);
    }
    
    //Can't resolve, the result will be NULL.
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

//=--------------------------------------------------------------------------=
// CisInstalled::IPersistPropertyBag_Load
//=--------------------------------------------------------------------------=
// IPersistPropertyBag::Load. Called when container wants to load the control
// from property bags.
//
// Parameters:
//	pPropBag    Property Bag
//	pErrorLog   Error log
//	pMap	    ATL's property map
//
// Output:
//	HRESULT	    S_OK if succeeded
//
HRESULT CisInstalled::IPersistPropertyBag_Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog, ATL_PROPMAP_ENTRY* pMap)
{

    CisInstalled* pT = static_cast<CisInstalled*>(this);
    HRESULT hr = AtlIPersistPropertyBag_Load(pPropBag, pErrorLog, pMap, pT, pT->GetUnknown());

    if (!SUCCEEDED(hr)) {
        return hr;
    }

    // Load properties
    CComVariant v((BSTR) NULL);

    if (SUCCEEDED(pPropBag->Read(L"back", &v, pErrorLog)))
    {
        m_back = v.bstrVal;
        v.Clear();
        ZeroMemory(&v,sizeof(v));
    }

    if (SUCCEEDED(pPropBag->Read(L"app", &v, pErrorLog)))
    {
        m_app = v.bstrVal;
        v.Clear();
        ZeroMemory(&v,sizeof(v));
    }

    if (SUCCEEDED(pPropBag->Read(L"dbg", &v, pErrorLog)))
    {
        m_dbg = v.bstrVal;
        v.Clear();
        ZeroMemory(&v,sizeof(v));
    }

    return S_OK;
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
