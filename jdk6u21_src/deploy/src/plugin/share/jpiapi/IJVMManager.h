/*
 * @(#)IJVMManager.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IJVMManager.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for JVM Manager, this interface provides the hooks for browser
// to Create JNI. In the future release, we should get rid of this function because
// JNI is too low level C-style interface.
//
#ifndef _IJVMMANAGER_H_
#define _IJVMMANAGER_H_

#include "ISupports.h"
#include "ISecureEnv.h"
#include "jni.h"

//{3BB20CB2-9B7D-11d6-9A7D-00B0D0A18D51}
#define CJVMMANAGER_CID \
	{0x3BB20CB2, 0x9B7D, 0x11d6, {0x9A, 0x7D, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

//{EFD74BDE-99B7-11d6-9A76-00B0D0A18D51}
#define IJVMMANAGER_IID \
    {0xEFD74BDE, 0x99B7, 0x11d6, {0x9A, 0x76, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

//ISupports interface (A replicate of nsISupports)
class IJVMManager : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IJVMMANAGER_IID);
    JD_DEFINE_STATIC_CID_ACCESSOR(CJVMMANAGER_CID);

    /**
     * This method creates a JNIEnv
     * @params [out] outProxyEnv the JNIEnv returned
     */
    JD_IMETHOD CreateProxyJNI(ISecureEnv* secureEnv, JNIEnv * *outProxyEnv) = 0;

    /**
     * This has been deprecated since JPI won't relie on browser
     * to do athentication
     */
    JD_IMETHOD IsAllPermissionGranted(const char *lastFingerprint, const char *lastCommonName, const char *rootFingerprint, const char *rootCommonName, JDBool *_retval) = 0;
};

#endif // _IJVMMANAGER_H_
