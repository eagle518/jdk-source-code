/*
 * @(#)CNSAdapter_SecurityContextPeer.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContextPeer.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContextPeer.cpp is Implementation of adapter for 
// ISecurityContext
// 
//
#include "StdAfx.h"
#include "ISecurityContext.h"
#include "nsISecurityContext.h"
#include "CNSAdapter_SecurityContextPeer.h"
#include "Debug.h"
//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContextPeer::CNSAdapter_SecurityContextPeer
//=--------------------------------------------------------------------------=
//
// notes :
//	
CNSAdapter_SecurityContextPeer::CNSAdapter_SecurityContextPeer(nsISecurityContext* pSecurityContext) : 
m_pSecurityContext(pSecurityContext) 
{
    TRACE("CNSAdapter_SecurityContextPeer::CNSAdapter_SecurityContextPeer\n");

    JD_INIT_REFCNT();

    if (m_pSecurityContext)
	m_pSecurityContext->AddRef();
}

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContextPeer::~CNSAdapter_SecurityContextPeer
//=--------------------------------------------------------------------------=
//
// notes :
//	
CNSAdapter_SecurityContextPeer::~CNSAdapter_SecurityContextPeer() 
{
    TRACE("CNSAdapter_SecurityContextPeer::~CNSAdapter_SecurityContextPeer\n");

    if (m_pSecurityContext)
	m_pSecurityContext->Release();
}

//ISupports
JD_IMPL_ISUPPORTS1(CNSAdapter_SecurityContextPeer, ISecurityContext);

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContextPeer::Implies
//=--------------------------------------------------------------------------=
// param target        -- Possible target.
//	 action        -- Possible action on the target.
// return              -- NS_OK if the target and action is permitted on the security context.
//                     -- NS_FALSE otherwise.
// notes :
//	
JD_METHOD
CNSAdapter_SecurityContextPeer::Implies(const char* target, const char* action, JDBool *bAllowedAccess)
{
    TRACE("CNSAdapter_SecurityContextPeer::Implies\n");

    if (m_pSecurityContext == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pSecurityContext->Implies(target, action, bAllowedAccess);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContextPeer::GetOrigin
//=--------------------------------------------------------------------------=
// param target        -- Possible target.
//	 action        -- Possible action on the target.
// return              -- NS_OK if the target and action is permitted on the security context.
//                     -- NS_FALSE otherwise.
// notes :
//	
JD_METHOD
CNSAdapter_SecurityContextPeer::GetOrigin(char* buf, int len)
{
    TRACE("CNSAdapter_SecurityContextPeer::GetOrigin\n");

    if (m_pSecurityContext == NULL)
	return JD_ERROR_NULL_POINTER;

    return m_pSecurityContext->GetOrigin(buf, len);
}

//=--------------------------------------------------------------------------=
// CNSAdapter_SecurityContextPeer::GetCertificateID
//=--------------------------------------------------------------------------=
// param target        -- Possible target.
//	 action        -- Possible action on the target.
// return              -- NS_OK if the target and action is permitted on the security context.
//                     -- NS_FALSE otherwise.
// notes :
//	
JD_METHOD
CNSAdapter_SecurityContextPeer::GetCertificateID(char* buf, int len)
{
    TRACE("CNSAdapter_SecurityContextPeer::GetCertificateID\n");

    if (m_pSecurityContext == NULL)
	return JD_ERROR_NULL_POINTER;
    
    return m_pSecurityContext->GetCertificateID(buf, len);
}



