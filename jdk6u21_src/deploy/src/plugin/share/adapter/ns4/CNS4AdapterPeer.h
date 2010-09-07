/*
 *  @(#)CNS4AdapterPeer.h	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=--------------------------------------------------------------------------=
// CNS4AdapterPeer.h    by Stanley Man-Kit Ho
//=--------------------------------------------------------------------------=
//
// CNS4AdapterPeer.h : Declaration of the CNS4AdapterPeer
//
// CNS4AdapterPeer is an abstraction of the Netsacpe Plug-in API on the
// browser side for Navigator 3/4.
//

#ifndef __CNS4ADAPTERPEER_H_
#define __CNS4ADAPTERPEER_H_

#include "INS4AdapterPeer.h"
#include "INS4AdapterPeerInit.h"

#ifdef XP_WIN
typedef int NPNVariable;
#endif
/////////////////////////////////////////////////////////////////////////////
// CNS4AdapterPeer
class CNS4AdapterPeer : 
	public INS4AdapterPeer,
	public INS4AdapterPeerInit
{
public:
    CNS4AdapterPeer(void);

    JD_DECL_ISUPPORTS
    // INS4AdapterPeer
    JD_IMETHOD_(void) NPN_Version(int*, int*, int*, int*);

    JD_IMETHOD_(NPError) NPN_GetURLNotify(NPP, const char*, 
			                  const char*, void*);

    JD_IMETHOD_(NPError) NPN_GetURL(NPP, const char*, const char*);

    JD_IMETHOD_(NPError) NPN_PostURLNotify(NPP, const char*, 
			                   const char*, uint32,
		                           const char*, NPBool, void*);

    JD_IMETHOD_(NPError) NPN_PostURL(NPP, const char*,
			             const char*, uint32, const char*, NPBool);

    JD_IMETHOD_(NPError) NPN_RequestRead(NPStream*, NPByteRange*);

    JD_IMETHOD_(NPError) NPN_NewStream(NPP, NPMIMEType, const char*, NPStream**);

    JD_IMETHOD_(int32) NPN_Write(NPP, NPStream*, int32, void*);

    JD_IMETHOD_(NPError) NPN_DestroyStream(NPP, NPStream*, NPReason);

    JD_IMETHOD_(void) NPN_Status(NPP, const char*);

    JD_IMETHOD_(const char*) NPN_UserAgent(NPP);

    JD_IMETHOD_(void*) NPN_MemAlloc(uint32);

    JD_IMETHOD_(void) NPN_MemFree(void*);

    JD_IMETHOD_(uint32) NPN_MemFlush(uint32);

    JD_IMETHOD_(void) NPN_ReloadPlugins(NPBool);

    JD_IMETHOD_(JRIEnv*) NPN_GetJavaEnv(void);

    JD_IMETHOD_(jref) NPN_GetJavaPeer(NPP);

    JD_IMETHOD_(NPError) NPN_GetValue(NPP instance,
                                      NPNVariable variable,
                                      void* value);
    // INS4AdapterPeerInit
    JD_IMETHOD Initialize(NPNetscapeFuncs*);

private:
    NPNetscapeFuncs* m_pNavigatorFuncs;
};

#endif //__CNS4ADAPTERPEER_H_
