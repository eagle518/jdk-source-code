/*
 * @(#)IPluginStreamInfo.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// IPluginStreamInfo.h  by X.Lu
//
///=--------------------------------------------------------------------------=
#ifndef _IPLUGINSTREAMINFO_H_
#define _IPLUGINSTREAMINFO_H_

#include "ISupports.h"

#define IPLUGINSTREAMINFO_IID			     \
{ /* {7A168FD5-A576-11d6-9A82-00B0D0A18D51} */       \
    0x7A168FD5,                                      \
    0xA576,                                          \
    0x11d6,                                          \
    {0x9A, 0x82, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51} \
}

class IPluginStreamInfo : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGINSTREAMINFO_IID);

    JD_IMETHOD
    GetContentType(JDPluginMimeType* result) = 0;

    JD_IMETHOD
    IsSeekable(JDBool* result) = 0;

    JD_IMETHOD
    GetLength(JDUint32* result) = 0;

    JD_IMETHOD
    GetLastModified(JDUint32* result) = 0;

    JD_IMETHOD
    GetURL(const char** result) = 0;

    JD_IMETHOD
    GetNotifyData(void** result) = 0;
};

////////////////////////////////////////////////////////////////////////////////

#endif /* _IPLUGINSTREAMINFO_H_ */
