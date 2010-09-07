/*
 * @(#)INS4PluginInstance.h	1.4 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// INS4PluginInstance.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
// Contains the interface for service from NS4 Java Plug-in
//
#ifndef _INS4PLUGININSTANCE_H_
#define _INS4PLUGININSTANCE_H_

#include "ISupports.h"

class IPluginStreamInfo;
class IPluginStream;

// {76DDA694-A299-4783-9059-2AA02C132FEF}
#define INS4PLUGININSTANCE_IID                              \
{0x76dda694, 0xa299, 0x4783, {0x90, 0x59, 0x2a, 0xa0, 0x2c, 0x13, 0x2f, 0xef}}

class INS4PluginInstance : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(INS4PLUGININSTANCE_IID);

    JD_IMETHOD
    NewStream(IPluginStreamInfo* peer, IPluginStream** stream) = 0;

    JD_IMETHOD
    URLNotify(const char* url, const char* target, JDPluginReason reason, void* notifyData) = 0;

    JD_IMETHOD
    SetDocbase(const char *url) = 0;

    JD_IMETHOD_(void) JavascriptReply(const char *reply) = 0;
};

#endif //_INS4PLUGININSTANCE_H_

