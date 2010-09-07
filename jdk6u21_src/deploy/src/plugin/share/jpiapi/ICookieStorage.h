/*
 * @(#)ICookieStorage.h	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

//=---------------------------------------------------------------------------=
//
// ICookieStorage.h  by X.Lu 
//
//=---------------------------------------------------------------------------=
//
// Contains interface for Store cookie
//

#ifndef _ICOOKIESTORAGE_H_
#define _ICOOKIESTORAGE_H_

#include "ISupports.h"

//{EFD74BE1-99B7-11d6-9A76-00B0D0A18D51}
#define ICOOKIESTORAGE_IID \
    {0xEFD74BE1, 0x99B7, 0x11d6, {0x9A, 0x76, 0x00, 0xB0, 0xD0, 0xA1, 0x8D, 0x51}}

class ICookieStorage : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(ICOOKIESTORAGE_IID);

    /**
     * Get the cookie from browser and store in the buffer with a certain size
     * @param inCookieURL       URL string to get the cookie from
     * @param inCookieBuffer    The buffer to store the result
     * @param inOutCookieSize   The size of the cookie buffer
     */
    JD_IMETHOD
    GetCookie(const char* inCookieURL, void* inOutCookieBuffer, JDUint32& inOutCookieSize) = 0;


    /**
     * Stores a cookie in the browser's persistent cookie store.
     * @param inCookieURL        URL string store cookie with.
     * @param inCookieBuffer     buffer containing cookie data.
     * @param inCookieSize       specifies  size of cookie data.
     */
    JD_IMETHOD
    SetCookie(const char* inCookieURL, const void* inCookieBuffer, JDUint32 inCookieSize) = 0;
};

#endif // _ICOOKIESTORAGE_H_
