/*
 * @(#)IPluginStreamListener.h	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// IPluginStreamListener.h  by X.Lu
//
///=--------------------------------------------------------------------------=
#ifndef _IPLUGINSTREAMLISTENER_H_
#define _IPLUGINSTREAMLISTENER_H_

#include "ISupports.h"
#include "IPluginStreamInfo.h"
#include "IInputStream.h"

#define IPLUGINSTREAMLISTENER_IID		     \
{ /*{7A168FD6-A576-11d6-9A82-00B0D0A18D51}*/	     \
    0x7A168FD6,                                      \
    0xA576,                                          \
    0x11d6,                                          \
    {0x9A, 0x82, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51} \
}

class IPluginStreamListener : public ISupports {
public:

    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGINSTREAMLISTENER_IID)

    /**
     * Notify the observer that the URL has started to load.  This method is
     * called only once, at the beginning of a URL load.<BR><BR>
     *
     * @return The return value is currently ignored.  In the future it may be
     * used to cancel the URL load..
     */
    JD_IMETHOD
    OnStartBinding(IPluginStreamInfo* pluginInfo) = 0;

    /**
     * Notify the client that data is available in the input stream.  This
     * method is called whenver data is written into the input stream by the
     * networking library...<BR><BR>
     * 
     * @param aIStream  The input stream containing the data.  This stream can
     * be either a blocking or non-blocking stream.
     * @param length    The amount of data that was just pushed into the stream.
     * @return The return value is currently ignored.
     */
    JD_IMETHOD
    OnDataAvailable(IPluginStreamInfo* pluginInfo, JDint32 offset, JDint32 len, const char* buffer, JDint32* count) = 0;
    
    JD_IMETHOD
    OnFileAvailable(IPluginStreamInfo* pluginInfo, const char* fileName) = 0;
 
    /**
     * Notify the observer that the URL has finished loading.  This method is 
     * called once when the networking library has finished processing the 
     * URL transaction initiatied via the nsINetService::Open(...) call.<BR><BR>
     * 
     * This method is called regardless of whether the URL loaded successfully.<BR><BR>
     * 
     * @param status    Status code for the URL load.
     * @param msg   A text string describing the error.
     * @return The return value is currently ignored.
     */
    JD_IMETHOD
    OnStopBinding(IPluginStreamInfo* pluginInfo, JDresult status) = 0;


    JD_IMETHOD
    GetStreamType(JDPluginStreamType *result) = 0;
};

////////////////////////////////////////////////////////////////////////////////
#endif /* _IPLUGINSTREAMLISTENER_H_ */
