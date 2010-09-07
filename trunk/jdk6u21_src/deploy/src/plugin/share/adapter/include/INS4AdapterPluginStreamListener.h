/*
 * @(#)INS4AdapterPluginStreamListener.h	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// nsIBwAdapterPluginStreamListener.h  by Stanley Man-Kit Ho
//
///=--------------------------------------------------------------------------=

#ifndef nsIBwAdapterPluginStreamListener_h__
#define nsIBwAdapterPluginStreamListener_h__

#include "nsplugindefs.h"

////////////////////////////////////////////////////////////////////////////////
// Plugin Stream Listener Interface for Navigator 3/4

/**
 *
 * The nsIBwAdapterPluginStreamListener interface provides a fallback to one of
 * the hack that we did in URLNotify to obtain the document base URL. Only the
 * document base stream listener should actually use it.
 *
 */
class nsIBwAdapterPluginStreamListener : public nsISupports {
public:
    NS_IMETHOD
    OnNotify(const char* url, nsresult status) = 0;
};


// {66677840-D1B4-11d2-BA21-00105A1F1DAB}
#define NS_IBWADAPTERPLUGINSTREAMLISTENER_IID			\
{	0x66677840,						\
	0xd1b4,							\
	0x11d2,							\
	{ 0xba, 0x21, 0x0, 0x10, 0x5a, 0x1f, 0x1d, 0xab } };	\

////////////////////////////////////////////////////////////////////////////////

#endif /* nsIBwAdapterPluginStreamListener_h__ */
