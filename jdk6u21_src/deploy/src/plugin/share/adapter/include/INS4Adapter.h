/*
 * @(#)INS4Adapter.h	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// INS4Adapter.h  by Stanley Man-Kit Ho
//
//=---------------------------------------------------------------------------=
//
// object definition for Netscape 4.x adapter implemetation in Java Plug-in.
//

#ifndef __INS4ADAPTER_H_
#define __INS4ADAPTER_H_

// INS4Adapter interface
DECLARE_INTERFACE_(INS4Adapter, IUnknown)
{
    // IUnknown memebers
    STDMETHOD(QueryInterface) (THIS_ REFIID, void**) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // INS4Adapter
    STDMETHOD_(NPError, NPP_Initialize) (THIS) PURE;
    STDMETHOD_(void, NPP_Shutdown) (THIS) PURE;
    STDMETHOD_(NPError, NPP_New) (THIS_ NPMIMEType, NPP, uint16, 
				  int16, char**, char**, NPSavedData*) PURE;
    STDMETHOD_(NPError, NPP_Destroy) (THIS_ NPP, NPSavedData**) PURE;
    STDMETHOD_(NPError, NPP_SetWindow) (THIS_ NPP, NPWindow*) PURE;
    STDMETHOD_(NPError, NPP_NewStream) (THIS_ NPP, NPMIMEType, 
					NPStream*, NPBool, uint16*) PURE;
    STDMETHOD_(NPError, NPP_DestroyStream) (THIS_ NPP,  NPStream*,
					    NPReason) PURE;
    STDMETHOD_(int32, NPP_WriteReady) (THIS_ NPP, NPStream*) PURE;
    STDMETHOD_(int32, NPP_Write) (THIS_ NPP, NPStream*, int32, int32, void*) PURE;
    STDMETHOD_(void, NPP_StreamAsFile) (THIS_ NPP, NPStream*, const char*) PURE;
    STDMETHOD_(void, NPP_Print) (THIS_ NPP, NPPrint*) PURE;
    STDMETHOD_(int16, NPP_HandleEvent) (THIS_ NPP, void*) PURE;
    STDMETHOD_(void, NPP_URLNotify) (THIS_ NPP, const char*, 
				     NPReason, void*) PURE;
    STDMETHOD_(jref, NPP_GetJavaClass) (THIS) PURE;
};

// {ADC80211-B7C2-11d2-BA19-00105A1F1DAB}
static const IID IID_INS4Adapter = 
{ 0xadc80211, 0xb7c2, 0x11d2, { 0xba, 0x19, 0x0, 0x10, 0x5a, 0x1f, 0x1d, 0xab } };


#endif // __INS4ADAPTER_H_
