/*
 * @(#)CNS4Adapter_PluginServiceProvider.cpp	1.6 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS4Adapter_PluginServiceProvider.cpp  by X.Lu
//
///=--------------------------------------------------------------------------=
//
// The service handle get from browser 
//

#include "StdAfx.h"
#include "IPluginManager.h"
#include "IPluginServiceProvider.h"
#include "CNS4Adapter_PluginServiceProvider.h"

//ISupports
JD_IMPL_ISUPPORTS1(CPluginServiceProvider, IPluginServiceProvider);

//=--------------------------------------------------------------------------=
// CPluginServiceProvider::CPluginServiceProvider
//=--------------------------------------------------------------------------=
//
CPluginServiceProvider::CPluginServiceProvider(ISupports* pProvider)
: mMgr(pProvider)
{
    JD_INIT_REFCNT();

    if (mMgr)
	mMgr->AddRef();
}

//=--------------------------------------------------------------------------=
// CPluginServiceProvider::~CPluginServiceProvider
//=--------------------------------------------------------------------------=
//
CPluginServiceProvider::~CPluginServiceProvider(void)
{
    if (mMgr)
	mMgr->Release();
}

//=--------------------------------------------------------------------------=
// CPluginServiceProvider::QueryService
//=--------------------------------------------------------------------------=
//param: cid     the Class ID of the service
//       iid     the interface id
//       result  the resultant service reference
//
JD_METHOD
CPluginServiceProvider::QueryService(const JDCID& cid,
				     const JDIID& iid,
				     ISupports* *result)
{
    if (mMgr == NULL)
	return JD_ERROR_NULL_POINTER;

    return mMgr->QueryInterface(iid, (void**)result);
}

//=--------------------------------------------------------------------------=
// CPluginServiceProvider::ReleaseService
//=--------------------------------------------------------------------------=
//params:   cid        the class id of the service
//          pService   the service reference
//
JD_METHOD
CPluginServiceProvider::ReleaseService(const JDCID& cid, /*[in]*/ISupports* pService)
{
    if (mMgr == NULL || pService == NULL)
	return JD_ERROR_NULL_POINTER;

    pService->Release();

    return JD_OK;
}
