/*
 * %W% %E%
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

#include "StdAfx.h"
#include "CNSAdapter_BrowserAuthenticator.h"


//ISupports
JD_IMPL_ISUPPORTS1(CNSAdapter_BrowserAuthenticator, IBrowserAuthenticator);


CNSAdapter_BrowserAuthenticator::CNSAdapter_BrowserAuthenticator(nsIJVMAuthTools* pJVMAuthTools){
    JD_INIT_REFCNT();
    m_spBrowserAuth = pJVMAuthTools;
    m_pAuthInfo = NULL;
}

CNSAdapter_BrowserAuthenticator::~CNSAdapter_BrowserAuthenticator() {
    if(NULL != m_pAuthInfo) {
		m_pAuthInfo->Release();
		m_pAuthInfo = NULL;
    }
}


JD_METHOD CNSAdapter_BrowserAuthenticator::GetAuthInfo(const char* protocol, 
	const char* host, 
	int port, 
	const char* scheme,
	const char* realm, 
	char* lpszUserName,
	int   nUserNameSize,
	char* lpszPassword,
	int   nPasswordSize) {

	if(NULL == lpszUserName || NULL == lpszPassword)
		return JD_ERROR_NULL_POINTER;

	if(NULL == (nsIJVMAuthTools*)m_spBrowserAuth)
		return JD_ERROR_FAILURE;

	nsIAuthenticationInfo* pAuthInfo;

	JDresult res = m_spBrowserAuth->GetAuthenticationInfo(protocol, host, port, scheme, realm, &pAuthInfo);
	if(NS_FAILED(res))
	    return res;

	char* lpszName;
	char* lpszPasswd;
	if(NS_FAILED(pAuthInfo->GetUsername((const char**)&lpszName)) || NS_FAILED(pAuthInfo->GetPassword((const char**)&lpszPasswd))) {
	    pAuthInfo->Release();
	    return JD_ERROR_FAILURE;
	}
	
	if(nUserNameSize <= (int)strlen(lpszName) || nPasswordSize <= (int)strlen(lpszPasswd)) {
	    pAuthInfo->Release();
	    return JD_ERROR_FAILURE;
	}

	strcpy(lpszUserName, lpszName);
	strcpy(lpszPassword, lpszPasswd);

	pAuthInfo->Release();

	return res;
}
