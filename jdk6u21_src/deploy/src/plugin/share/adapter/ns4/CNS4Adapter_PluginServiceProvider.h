/*
 * @(#)CNS4Adapter_PluginServiceProvider.h	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginServiceProvider.h  by X.Lu
//
///=--------------------------------------------------------------------------=
//
// The service handle get from browser 
//
#ifndef __CPLUGINSERVICEPROVIDER_H_
#define __CPLUGINSERVICEPROVIDER_H_

#include "IPluginServiceProvider.h"

class ISupports;

class CPluginServiceProvider : public IPluginServiceProvider {
public:

    JD_DECL_ISUPPORTS

    ////////////////////////////////////////////////////////////////////////////
    // from IPluginServiceProvider:

    /**
     * Obtain a plugin service. An nsIPluginServiceProvider will obtain the
     * plugin service according to the current browser version.
     *
     * @param clsid - the CLSID of the requested service.
     * @param iid - the IID of the request service.
     * @param result - the interface pointer of the requested service
     * @return - JD_OK if this operation was successful.
     */
    JD_IMETHOD
    QueryService(/*[in]*/ const JDCID& cid,
		/*[in]*/  const JDIID& iid,
                /*[out]*/ ISupports* *result);

    /**
     * Release a plugin service. An nsIPluginServiceProvider will release the
     * plugin service according to the current browser version.
     *
     * @param clsid - the CLSID of the service.
     * @param result - the interface pointer of the service
     * @return - JD_OK if this operation was successful.
     */
    JD_IMETHOD
    ReleaseService(/*[in]*/const JDCID& cid, /*[in]*/ISupports* pService);

    ////////////////////////////////////////////////////////////////////////////
    // CPluginServiceProvider specific methods:

    CPluginServiceProvider(ISupports* pProvoder);
    virtual ~CPluginServiceProvider(void);

protected:
    ISupports* mMgr;
};

#endif // __CPLUGINSERVICEPROVIDER_H_
