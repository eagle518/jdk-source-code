/*
 * @(#)ISecurityContext.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// ISecurityContext.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Contains interface called by Plugin to get addtional security info
//
#ifndef _ISECURITYCONTEXT_H_
#define _ISECURITYCONTEXT_H_

#include "ISupports.h"

//{389E0AC1-9840-11d6-9A73-00B0D0A18D51}
#define ISECURITYCONTEXT_IID \
    {0x389E0AC1, 0x9840, 0x11d6, {0x9A, 0x73, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51} }

//ISupports interface (A replicate of nsISupports)
class ISecurityContext : public ISupports {
public:

    JD_DEFINE_STATIC_IID_ACCESSOR(ISECURITYCONTEXT_IID);

    /**
     * Get the security context to be used in LiveConnect.
     * This is used for JavaScript <--> Java.
     *
     * @param target        -- Possible target.
     * @param action        -- Possible action on the target.
     * @return              -- NS_OK if the target and action is permitted on the security context.
     *                      -- NS_FALSE otherwise.
     */
    JD_IMETHOD Implies(const char* target, const char* action, JDBool *bAllowedAccess) = 0;

    /**
     * Get the origin associated with the context.
     *
     * @param buf        -- Result buffer (managed by the caller.)
     * @param len        -- Buffer length.
     * @return           -- NS_OK if the codebase string was obtained.
     *                   -- NS_FALSE otherwise.
     */
    JD_IMETHOD GetOrigin(char* buf, int len) = 0;

    /**
     * Get the certificate associated with the context.
     *
     * @param buf        -- Result buffer (managed by the caller.)
     * @param len        -- Buffer length.
     * @return           -- NS_OK if the codebase string was obtained.
     *                   -- NS_FALSE otherwise.
     */
    JD_IMETHOD GetCertificateID(char* buf, int len) = 0;
};

#endif //_ISECURITYCONTEXT_H_
