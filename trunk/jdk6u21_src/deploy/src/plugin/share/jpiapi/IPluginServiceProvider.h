/*
 * @(#)IPluginServiceProvider.h	1.1 02/11/04
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// IPluginServiceProvider.h  by X.Lu
//
///=--------------------------------------------------------------------------=

#ifndef _IPluginServiceProvider_h___
#define _IPluginServiceProvider_h___

#include "ISupports.h"

#define IPLUGINSERVICEPROVIDER_IID			    \
{	/* {E3508FA0-A6B8-11d2-BA11-00105A1F1DAB}  */	    \
    0xe3508fa0,						    \
    0xa6b8,						    \
    0x11d2,						    \
    { 0xba, 0x11, 0x0, 0x10, 0x5a, 0x1f, 0x1d, 0xab }	    \
}
////////////////////////////////////////////////////////////////////////////////
// Plugin Service Provider Interface

/**
 *
 * The nsIPluginServiceProvider interface provides an abstraction over how the
 * actual plugin service is obtained. Since accessing the plugin service is
 * quite different between Navigator 3/4 and Mozilla, this interface basically
 * shield away this different to make writing code easier.
 *
 */
class IPluginServiceProvider : public ISupports {
public:
    JD_DEFINE_STATIC_IID_ACCESSOR(IPLUGINSERVICEPROVIDER_IID);
    /**
     * Obtain a plugin service. An nsIPluginServiceProvider will obtain the 
     * plugin service according to the current browser version.
     *
     * @param clsid - the CLSID of the requested service.
     * @param iid - the IID of the request service.
     * @param result - the interface pointer of the requested service
     * @return - NS_OK if this operation was successful.
     */
    JD_IMETHOD
    QueryService(/*[in]*/  const JDCID& clsid, 
		 /*[in]*/  const JDIID& iid,
                 /*[out]*/ ISupports* *result) = 0;


    /**
     * Release a plugin service. An nsIPluginServiceProvider will release the 
     * plugin service according to the current browser version.
     *
     * @param clsid - the CLSID of the service.
     * @param result - the interface pointer of the service
     * @return - NS_OK if this operation was successful.
     */
    JD_IMETHOD
    ReleaseService(/*[in]*/ const JDCID& clsid, 
		   /*[in]*/ ISupports* pService) = 0;
};

////////////////////////////////////////////////////////////////////////////////

#endif /* _IPluginServiceProvider_h___ */
