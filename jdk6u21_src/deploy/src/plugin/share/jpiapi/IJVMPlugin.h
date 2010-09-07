/*
 * @(#)IJVMPlugin.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IJVMPlugin.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for functionality of JVM Plugin browser required from JPI.
//
#ifndef _IJVMPLUGIN_H_
#define _IJVMPLUGIN_H_

#include "ISupports.h"
#include "ISecureEnv.h"

//{389E0ABF-9840-11d6-9A73-00B0D0A18D51}
#define IJVMPLUGIN_IID \
    {0x389E0ABF, 0x9840, 0x11d6, {0x9A, 0x73, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

//ISupports interface (A replicate of nsISupports)
class IJVMPlugin : public ISupports {
public:

    JD_DEFINE_STATIC_IID_ACCESSOR(IJVMPLUGIN_IID);

    /**
     * @params returns the current classpath in use by the JVM.
     */
    JD_IMETHOD
    GetClassPath(const char* *result) = 0;
    
    /**
     * @params jenv the JNI enviroment variable
     * @params [in] obj the opaque handle to the java script object
     * @params [out] the netscape.javascript.JSObject
     */
    JD_IMETHOD
    GetJavaWrapper(JNIEnv* jenv, jint obj, jobject *jobj) = 0;

    /**
     * This creates a new secure communication channel with Java. The second parameter,
     * nativeEnv, if non-NULL, will be the actual thread for Java communication.
     * Otherwise, a new thread should be created.
     * @param	proxyEnv		the env to be used by all clients on the browser side
     * @return	outSecureEnv	the secure environment used by the proxyEnv
     */
    JD_IMETHOD
    CreateSecureEnv(JNIEnv* proxyEnv, ISecureEnv* *outSecureEnv) = 0;

    /**
     * @params jenv the JNI enviroment variable
     * @params [in] jobj the java object
     * @params [out] the opaque handle to javascript object
     */
    JD_IMETHOD
    UnwrapJavaWrapper(JNIEnv* jenv, jobject jobj, jint* obj) = 0;
};

#endif //_IJVMPLUGIN_H_
