/*
 * CNS7Adapter_BrowserAuthenticator.h	 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef __CNSADAPTER_BROWSERAUTHENTICATOR_H
#define __CNSADAPTER_BROWSERAUTHENTICATOR_H

#include "JDSmartPtr.h"
#include "IBrowserAuthenticator.h"
#include "nsIJVMAuthTools.h"

class CNSAdapter_BrowserAuthenticator: public IBrowserAuthenticator {

public:
	JD_DECL_ISUPPORTS

	CNSAdapter_BrowserAuthenticator(nsIJVMAuthTools* pIJVMAuthTools);
	~CNSAdapter_BrowserAuthenticator();

	JD_IMETHOD GetAuthInfo(const char* protocol, 
			       const char* host, 
			       int port, 
			       const char* scheme,
			       const char* realm,
			       char*  lpszUserName,
			       int    nUserNameSize,
			       char*  lpszPassword,
			       int    nPasswordSize);

private:
	JDSmartPtr<nsIJVMAuthTools>	m_spBrowserAuth;
	nsIAuthenticationInfo*		m_pAuthInfo;
};



#endif __CNSADAPTER_BROWSERAUTHENTICATOR_H
