/*
 * @(#)CSecurityContext.cpp	1.20 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=------------------------------------------------------------------------=
//
// CSecurityContext.cpp  by Stanley Man-Kit Ho
//
///=------------------------------------------------------------------------=
//
// These functions create object, invoke method, get/set field in JNI with
// security context.
//
 
#include "commonhdr.h"
#include "ISecurityContext.h"
#include "CSecurityContext.h"

// Mozilla changes : Added a semicolon at the end
JD_DEFINE_IID(jISecurityContextIID, ISECURITYCONTEXT_IID);
JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);

// This macro expands to the aggregated query interface scheme.
JD_IMPL_AGGREGATED(CSecurityContext);

JD_METHOD
CSecurityContext::AggregatedQueryInterface(const JDIID& aIID, void** aInstancePtr)
{
    if (aIID.Equals(jISupportsIID)) {
      *aInstancePtr = GetInner();
      AddRef();
      return JD_OK;
    }
    if (aIID.Equals(jISecurityContextIID)) {
        *aInstancePtr = (ISecurityContext *)this;
        AddRef();
        return JD_OK;
    }
    return JD_NOINTERFACE;
}



// Implements the CSecurityContext object for encapsulating security context.
// This security context object encapuslates the security associated with
// a particular JS call from Java. 
CSecurityContext::CSecurityContext(ISupports *aOuter, 
				   const char* lpszURL,
                                   JDBool isAllPermission) : m_isAllPermission(isAllPermission)
{
    trace("CSecurityContext::CSecurityContext\n");

    JD_INIT_AGGREGATED(aOuter);
	m_lpszURL = NULL;

    if (lpszURL != NULL) {
	m_lpszURL = new char[strlen(lpszURL) + 1];
        strcpy(m_lpszURL, lpszURL);
	}
}


// Destroy the security context. Who does this?
CSecurityContext::~CSecurityContext()  
{
    trace("CSecurityContext::~CSecurityContext\n");
    if(NULL != m_lpszURL)
	delete[] m_lpszURL;
}


// Create the CSecurityContext object for creating object, invoking method, 
// getting/setting field in JNI with security context.
JD_METHOD
CSecurityContext::Create(ISupports* outer, 
			 const char* lpszURL, JDBool isAllPermission, 
			 const JDIID& aIID, void* *aInstancePtr)
{
    /* If this is an aggregated object creation (i.e. outer exists)
       then we expect to return the special object Internal, which
       provides the nondelegating functions */

    if (outer && !aIID.Equals(jISupportsIID))
        return JD_NOINTERFACE;   // XXX right error?
    CSecurityContext* context = new CSecurityContext(outer, lpszURL, isAllPermission);
    if (context == NULL)
        return JD_ERROR_OUT_OF_MEMORY;
    context->AddRef();
    *aInstancePtr = (outer != NULL)
	?(void *)context->GetInner()
	:(void *)context;
    return JD_OK;
}


////////////////////////////////////////////////////////////////////////////
// from ISecurityContext:
//

// Get the security context to be used in LiveConnect.
// This is used for JavaScript <--> Java.
//
// @param target        -- Possible target.
// @param action        -- Possible action on the target.
// @return              -- JD_OK if the action is permitted 
//                      -- JD_FALSE otherwise.
JD_IMETHODIMP CSecurityContext::Implies(const char* target, 
					const char* action, 
					JDBool* bActionAllowed)
{
    trace("CSecurityContext::Implies\n");

    UNUSED(action);

    if (target == NULL || bActionAllowed == NULL)
        return JD_ERROR_NULL_POINTER;

    *bActionAllowed = m_isAllPermission;

    return JD_OK;
}


////////////////////////////////////////////////////////////////////////////
// from ISecurityContext:
//

// Get the origin associated with the context.
//
// @param buf        -- Result buffer (managed by the caller.)
// @param len        -- Buffer length.
// @return           -- JD_OK if the origin string was obtained.
//                   -- JD_FALSE otherwise.

JD_IMETHODIMP CSecurityContext::GetOrigin(char* buf, int len)
{
    trace("CSecurityContext::GetOrigin\n");
    
    if (buf == NULL)
        return JD_ERROR_NULL_POINTER;

    if (((unsigned int) len) <= strlen(m_lpszURL))
        return JD_ERROR_FAILURE;

    // Copy origin
    strcpy(buf, m_lpszURL);


    return JD_OK;
}


////////////////////////////////////////////////////////////////////////////
// from ISecurityContext:
//

// Get the certificate associated with the context.
// 
//  @param buf        -- Result buffer (managed by the caller.)
//  @param len        -- Buffer length.
//  @return           -- JD_OK if the certificate string was obtained.
//                    -- JD_FALSE otherwise.

JD_IMETHODIMP CSecurityContext::GetCertificateID(char* buf, int len)

{
    trace("CSecurityContext::GetCertificateID\n");

    UNUSED(buf);
    UNUSED(len);

    return JD_OK;
}

   

