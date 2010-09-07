/*
 * @(#)INS4AdapterPeer.h	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// INS4AdapterPeer.h  by Stanley Man-Kit Ho
//
//=---------------------------------------------------------------------------=
//
// object definition for Netscape 4.x adapter peer implemetation in Java Plug-in.
//

#ifndef __INS4ADAPTERPEER_H_
#define __INS4ADAPTERPEER_H_

#include "ISupports.h"

#ifdef XP_WIN
typedef int NPNVariable;
#endif

// {ADC80210-B7C2-11d2-BA19-00105A1F1DAB}
static const JDIID IID_INS4AdapterPeer = 
{ 0xadc80210, 0xb7c2, 0x11d2, { 0xba, 0x19, 0x0, 0x10, 0x5a, 0x1f, 0x1d, 0xab } };

// INS4AdapterPeer interface
class INS4AdapterPeer : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IID_INS4AdapterPeer);

    JD_IMETHOD_(void) NPN_Version(int*, int*, int*, int*) = 0;

    JD_IMETHOD_(NPError) NPN_GetURLNotify(NPP, const char*, const char*, void*) = 0;

    JD_IMETHOD_(NPError) NPN_GetURL(NPP, const char*, const char*) = 0;

    JD_IMETHOD_(NPError) NPN_PostURLNotify(NPP, const char*, const char*, uint32,
					   const char*, NPBool, void*) = 0;

    JD_IMETHOD_(NPError) NPN_PostURL(NPP, const char*, const char*, uint32, const char*, NPBool) = 0;

    JD_IMETHOD_(NPError) NPN_RequestRead(NPStream*, 
					 NPByteRange*) = 0;

    JD_IMETHOD_(NPError) NPN_NewStream(NPP, NPMIMEType, const char*, NPStream**) = 0;

    JD_IMETHOD_(int32) NPN_Write(NPP, NPStream*, int32, void*) = 0;

    JD_IMETHOD_(NPError) NPN_DestroyStream(NPP, NPStream*, NPReason) = 0;

    JD_IMETHOD_(void) NPN_Status(NPP, const char*) = 0;

    JD_IMETHOD_(const char*) NPN_UserAgent(NPP) = 0;

    JD_IMETHOD_(void*) NPN_MemAlloc(uint32) = 0;
    
    JD_IMETHOD_(void) NPN_MemFree(void*) = 0;

    JD_IMETHOD_(uint32) NPN_MemFlush(uint32) = 0;

    JD_IMETHOD_(void) NPN_ReloadPlugins(NPBool) = 0;

    JD_IMETHOD_(JRIEnv*) NPN_GetJavaEnv() = 0;

    JD_IMETHOD_(jref) NPN_GetJavaPeer(NPP) = 0;

    JD_IMETHOD_(NPError) NPN_GetValue(NPP instance, NPNVariable variable, void* value) = 0;
};

#endif // __INS4ADAPTERPEER_H_
