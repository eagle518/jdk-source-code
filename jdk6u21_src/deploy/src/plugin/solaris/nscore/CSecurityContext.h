/*
 * @(#)CSecurityContext.h	1.14 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=-----------------------------------------------------------------------=
// CSecurityContext.h   Based on win32 version by Stanley Man-Kit Ho
///=------------------------------------------------------------------------=
/* 
   This implementation of the security context is a wrapper around the
   Java protection domain in a Java to JS call. It calls back into
   Java to perform the "implies"

   */
#ifndef CSecurityContext_h___
#define CSecurityContext_h___

#include "jni.h"
#include "JDSupportUtils.h"
#include "ISecurityContext.h"

class CSecurityContext : public ISecurityContext
{
public:

    //////////////////////////////////////////////////////////////////////////
    // from ISupports and AggregatedQueryInterface:

    JD_DECL_AGGREGATED

    static JD_METHOD Create(ISupports* outer, 
			    const char* lpszURL, JDBool isAllPermission,
                            const JDIID& aIID, 
			    void* *aInstancePtr);

    ////////////////////////////////////////////////////////////////////////
    // from ISecurityContext:

    /**
     * Get the security context to be used in LiveConnect.
     * This is used for JavaScript <--> Java.
     *
     * @param target        -- Possible target.
     * @param action        -- Possible action on the target.
     * @return              -- NS_OK if the target and action is permitted 
     *                      -- NS_FALSE otherwise.
     */
    JD_IMETHOD Implies(const char* target, const char* action, 
		       JDBool* bActionAllowed);

    /**
     * Get the origin associated with the context.
     *
     * @param buf        -- Result buffer (managed by the caller.)
     * @param len        -- Buffer length.
     * @return           -- NS_OK if the codebase string was obtained.
     *                   -- NS_FALSE otherwise.
     */
    JD_IMETHOD GetOrigin(char* buf, int len);

    /**
     * Get the certificate associated with the context.
     *
     * @param buf        -- Result buffer (managed by the caller.)
     * @param len        -- Buffer length.
     * @return           -- NS_OK if the codebase string was obtained.
     *                   -- NS_FALSE otherwise.
     */
    JD_IMETHOD GetCertificateID(char* buf, int len);

   
    //////////////////////////////////////////////////////////////////////
    // from nsISecureJNI:

    CSecurityContext(ISupports *aOuter, 
		     const char* lpszURL, JDBool isAllPermission);
    virtual ~CSecurityContext(void);

protected:
    char*	 m_lpszURL;
    JDBool       m_isAllPermission;
};

#endif // CSecurityContext_h___





