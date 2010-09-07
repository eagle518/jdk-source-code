/*
 * @(#)CNSAdapter_SecurityContext.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContext.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContext.cpp is Implementation of adapter for 
// nsISecurityContext
// 
//
#include "StdAfx.h"
#include "nsISecurityContext.h"
#include "ISecurityContext.h"
#include "CNSAdapter_SecurityContext.h"

#include "Debug.h"
//nsISupports 
NS_IMPL_ISUPPORTS1(CNSAdapter_SecurityContext, nsISecurityContext);


//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContext::CNSAdapter_SecurityContext
//=--------------------------------------------------------------------------=
//
// notes :
//	
CNSAdapter_SecurityContext::CNSAdapter_SecurityContext(ISecurityContext* pSecurityContext) : 
m_pSecurityContext(pSecurityContext) 
{
    TRACE("CNSAdapter_SecurityContext::CNSAdapter_SecurityContext\n");

    NS_INIT_REFCNT();	
    if (m_pSecurityContext)
	m_pSecurityContext->AddRef();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContext::~CNSAdapter_SecurityContext
//=--------------------------------------------------------------------------=
//
// notes :
//	
CNSAdapter_SecurityContext::~CNSAdapter_SecurityContext() 
{
    TRACE("CNSAdapter_SecurityContext::~CNSAdapter_SecurityContext\n");

    if (m_pSecurityContext)
	m_pSecurityContext->Release();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContext::Implies
//=--------------------------------------------------------------------------=
// param target        -- Possible target.
//	 action        -- Possible action on the target.
// return              -- NS_OK if the target and action is permitted on the security context.
//                     -- NS_FALSE otherwise.
// notes :
//	
NS_METHOD
CNSAdapter_SecurityContext::Implies(const char* target, const char* action, PRBool *bAllowedAccess)
{
    TRACE("CNSAdapter_SecurityContext::Implies\n");

    if (m_pSecurityContext == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecurityContext->Implies(target, action, bAllowedAccess);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContext::GetOrigin
//=--------------------------------------------------------------------------=
// params:  buf    --Result buffer
//	    len	   -- buffer length
// notes:
//
NS_METHOD
CNSAdapter_SecurityContext::GetOrigin(char* buf, int len)
{
    TRACE("CNSAdapter_SecurityContext::GetOrigin\n");

    if (m_pSecurityContext == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecurityContext->GetOrigin(buf, len);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContext::GetCertificateID
//=--------------------------------------------------------------------------=
// params:  buf    --Result buffer
//	    len	   -- buffer length
// notes:
//
NS_METHOD
CNSAdapter_SecurityContext::GetCertificateID(char* buf, int len)
{
    TRACE("CNSAdapter_SecurityContext::GetCertificateID\n");

    if (m_pSecurityContext == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pSecurityContext->GetCertificateID(buf, len);
}
