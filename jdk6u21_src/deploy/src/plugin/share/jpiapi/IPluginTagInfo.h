/*
 * @(#)IPluginTagInfo.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// IPluginTagInfo.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Contains interface for Plugin Tag information called by Plugin.
//
#ifndef _IPLUGINTAGINFO_H_
#define _IPLUGINTAGINFO_H_

#include "ISupports.h"

#define IPLUGINTAGINFO_IID				    \
{	/* 5A2E31B7-AF17-11d6-9A8D-00B0D0A18D51 */	    \
	0x5A2E31B7,					    \
	0xAF17,						    \
	0x11d6,						    \
	{0x9A,0x8D,0x00,0xB0,0xD0,0xA1,0x8D, 0x51}	    \
};

// An Image of IPluginTagInfo of Mozilla
// See http://lxr.mozilla.org/seamonkey/source/modules/plugin/base/public/nsIPluginTagInfo.h
//
class IPluginTagInfo : public ISupports
{
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGINTAGINFO_IID)

    JD_IMETHOD
    GetAttributes(JDUint16& n, const char*const*& names, const char*const*& values) = 0;

    JD_IMETHOD
    GetAttribute(const char* name, const char* *result) = 0;

};

#endif //_IPLUGINTAGINFO_H_
