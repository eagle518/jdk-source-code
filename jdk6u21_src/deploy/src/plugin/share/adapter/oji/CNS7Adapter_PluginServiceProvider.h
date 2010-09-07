/*
 * @(#)CNS7Adapter_PluginServiceProvider.h	1.1 02/11/06
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter_PluginServiceProvider.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter_PluginServiceProvider.h is Declaration of adapter for 
// IPluginServiceProvider, it provides the browser service to adapter module
// 
//
#ifndef __CNS7Adapter_PluginServiceProvider_h__
#define __CNS7Adapter_PluginServiceProvider_h__

#include "nsIServiceManager.h"
#include "IPluginServiceProvider.h"

class nsIPluginManager;
class nsIJVMManager;
class nsIComponentManager;

class IPluginManager;
class IJVMManager;
 
class CNS7Adapter_PluginServiceProvider : public IPluginServiceProvider {
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
    // CNS7Adapter_PluginServiceProvider specific methods:

    CNS7Adapter_PluginServiceProvider(nsISupports* pProvoder);
    virtual ~CNS7Adapter_PluginServiceProvider(void);

protected:
    nsIServiceManager* mService;
 
    // Cache the service in Mozilla side
    nsIPluginManager*	    m_pPluginManager;
    nsIJVMManager*	    m_pJVMManager;
    // Cacahe this to create liveconnect instance
    nsIComponentManager*    m_pComponentManager;

    // These two objects represent the service adapter in JPI side
    // they are seen by JPI code to get the service done
    IPluginManager*	    m_pPluginManagerAdapter;
    IJVMManager*	    m_pJVMManagerAdapter;
};

#endif // __CNS7Adapter_PluginServiceProvider_h__
