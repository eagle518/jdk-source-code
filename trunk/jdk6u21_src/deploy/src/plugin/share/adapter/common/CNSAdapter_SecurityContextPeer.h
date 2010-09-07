/*
 * @(#)CNSAdapter_SecurityContextPeer.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContextPeer.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNSAdapter_SecurityContextPeer.h is Declaration of adapter for ISecurityContext
// 
//
#ifndef __CNSAdapter_SecurityContextPeer_H_
#define __CNSAdapter_SecurityContextPeer_H_

#include "ISecurityContext.h"

class nsISecurityContext;
class CNSAdapter_SecurityContextPeer : public ISecurityContext
{
public:
    CNSAdapter_SecurityContextPeer(nsISecurityContext* pSecurityContext);
    virtual ~CNSAdapter_SecurityContextPeer();

    // nsISupports
    JD_DECL_ISUPPORTS

    // nsISecurityContext
    JD_IMETHOD
    Implies(const char* target, const char* action, JDBool *bAllowedAccess);

    JD_IMETHOD
    GetOrigin(char* buf, int len);

    JD_IMETHOD
    GetCertificateID(char* buf, int len);
private:
    nsISecurityContext* m_pSecurityContext;
};

#endif //__CNSAdapter_SecurityContextPeer_H_
