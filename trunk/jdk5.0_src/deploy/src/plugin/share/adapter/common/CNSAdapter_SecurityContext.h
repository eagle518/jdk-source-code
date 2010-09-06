/*
 * @(#)CNSAdapter_SecurityContext.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContext.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContext.h is Declaration of adapter for nsISecurityContext
// 
//
#ifndef __CNSAdapter_SecurityContext_H_
#define __CNSAdapter_SecurityContext_H_

#include "nsISecurityContext.h"

class ISecurityContext;

class CNSAdapter_SecurityContext : public nsISecurityContext
{
public:
    CNSAdapter_SecurityContext(ISecurityContext* pSecurityContext);
    virtual ~CNSAdapter_SecurityContext(void);

    // nsISupports
    NS_DECL_ISUPPORTS

    // nsISecurityContext
    NS_IMETHOD
    Implies(const char* target, const char* action, PRBool *bAllowedAccess);

    NS_IMETHOD
    GetOrigin(char* buf, int len);

    NS_IMETHOD
    GetCertificateID(char* buf, int len);
private:
    ISecurityContext* m_pSecurityContext;
};

#endif //__CNSAdapter_SecurityContext_H_
