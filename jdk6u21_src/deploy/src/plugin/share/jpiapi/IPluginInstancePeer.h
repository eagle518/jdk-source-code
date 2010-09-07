/*
 * @(#)IPluginInstancePeer.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// IPluginInstancePeer.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
// Contains the interface for service of browser required from Java Plug-in
//
#ifndef _IPLUGININSTANCEPEER_H_
#define _IPLUGININSTANCEPEER_H_

#include "ISupports.h"

#define IPLUGININSTANCEPEER_IID                   \
{ /* {7A168FD4-A576-11d6-9A82-00B0D0A18D51} */		 \
    0x7A168FD4,                                      \
    0xA576,                                          \
    0x11d6,                                          \
    {0x9A, 0x82, 0x00, 0xB0, 0xD0, 0xA1, 0x8d, 0x51} \
}

class IPluginInstancePeer : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGININSTANCEPEER_IID)

    /**
     * Returns the MIME type of the plugin instance.
     *
     * (Corresponds to NPP_New's MIMEType argument.)
     *
     * @param result - resulting MIME type
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    GetMIMEType(JDPluginMimeType *result) = 0;

    /**
     * This operation causes status information to be displayed on the window
     * associated with the plugin instance.
     *
     * (Corresponds to NPN_Status.)
     *
     * @param message - the status message to display
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    ShowStatus(const char* message) = 0;

    /**
     * Set the desired size of the window in which the plugin instance lives.
     *
     * @param width - new window width
     * @param height - new window height
     * @result - NS_OK if this operation was successful
     */
    JD_IMETHOD
    SetWindowSize(JDUint32 width, JDUint32 height) = 0;
    
    /**
     * Get the thread ID required for creating netscape.javascript.JSObject
     * @params [out] outThreadID the out thread ID
     */
    JD_IMETHOD
    GetJSThread(JDUint32 *outThreadID) = 0;

};

////////////////////////////////////////////////////////////////////////////////

#endif /* _IPLUGININSATNCEPEER_H_*/
