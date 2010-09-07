/*
 * @(#)IPluginTagInfo2.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// IPluginTagInfo2.h  by X.Lu
//
///=--------------------------------------------------------------------------=
// Contains additional interface for Plugin Tag information called by Plugin.
//
#ifndef _IPLUGINTAGINFO2_H_
#define _IPLUGINTAGINFO2_H_

#include "ISupports.h"
#include "IPluginTagInfo.h"

#define IPLUGINTAGINFO2_IID				\
{   /*{7A168FD9-A576-11d6-9A82-00B0D0A18D51}*/		\
    0x7A168FD9,						\
    0xA576,						\
    0x11d6,						\
    {0x9A,0x82,0x00,0xB0,0xD0,0xA1,0x8D, 0x51}		\
};


enum JDPluginTagType {
    JDPluginTagType_Unknown,
    JDPluginTagType_Embed,
    JDPluginTagType_Object,
    JDPluginTagType_Applet
};


// An Image of nsIPluginTagInfo2 of Mozilla
// See http://lxr.mozilla.org/seamonkey/source/modules/plugin/base/public/nsIPluginTagInfo2.h
//
class IPluginTagInfo2 : public IPluginTagInfo
{
public:

    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGINTAGINFO2_IID)

    // Get the type of the HTML tag that was used ot instantiate this
    // plugin.  Currently supported tags are EMBED, OBJECT and APPLET.
    JD_IMETHOD
    GetTagType(JDPluginTagType *result) = 0;

    // Get the complete text of the HTML tag that was
    // used to instantiate this plugin
    JD_IMETHOD
    GetTagText(const char* *result) = 0;

    // Get a ptr to the paired list of parameter names and values,
    // returns the length of the array.
    //
    // Each name or value is a null-terminated string.
    JD_IMETHOD
    GetParameters(JDUint16& n, const char*const*& names, const char*const*& values) = 0;

    // Get the value for the named parameter.  Returns null
    // if the parameter was not set.
    // @result - NS_OK if this operation was successful, NS_ERROR_FAILURE if
    // this operation failed. result is set to NULL if the attribute is not found
    // else to the found value.
    JD_IMETHOD
    GetParameter(const char* name, const char* *result) = 0;

    JD_IMETHOD
    GetDocumentBase(const char* *result) = 0;

    // Return an encoding whose name is specified in:
    // http://java.sun.com/products/jdk/1.1/docs/guide/intl/intl.doc.html#25303
    JD_IMETHOD
    GetDocumentEncoding(const char* *result) = 0;

    JD_IMETHOD
    GetWidth(JDUint32 *result) = 0;

    JD_IMETHOD
    GetHeight(JDUint32 *result) = 0;

    // Returns a unique id for the current document on which the
    // plugin is displayed.
    JD_IMETHOD
    GetUniqueID(JDUint32 *result) = 0;
};

#endif //_IPLUGINTAGINFO_H_
