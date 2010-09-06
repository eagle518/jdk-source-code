/*
 * @(#)CNS6Adapter_ObserverService.cpp	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter_ObserverService.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS6Adapter_ObserverService.cpp is Implementation of adapter for nsIObserverService
// 
//
#include "StdAfx.h"
#include "nsIObserverService.h"
#include "IObserverService.h"
#include "nsIObserver.h"
#include "IObserver.h"
#include "CNS6Adapter_Observer.h"
#include "CNS6Adapter_ObserverService.h"

//ISupports
JD_IMPL_ISUPPORTS1(CNS6Adapter_ObserverService, IObserverService);

//=--------------------------------------------------------------------------=
// CNS6Adapter_ObserverService::CNS6Adapter_ObserverService
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
CNS6Adapter_ObserverService::CNS6Adapter_ObserverService(nsIObserverService* pIObserverService) :
m_pIObserverService(pIObserverService)
{
    JD_INIT_REFCNT();

    if (m_pIObserverService)
	m_pIObserverService->AddRef();
}

//=--------------------------------------------------------------------------=
// CNS6Adapter_ObserverService::~CNS6Adapter_ObserverService
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
CNS6Adapter_ObserverService::~CNS6Adapter_ObserverService(void)
{
    if (m_pIObserverService)
	m_pIObserverService->Release();
}

//=--------------------------------------------------------------------------=
// CNS6Adapter_ObserverService::AddObserver
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
JD_METHOD
CNS6Adapter_ObserverService::AddObserver(IObserver *anObserver, const JDUnichar *aTopic)
{
    if (m_pIObserverService == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsIObserver> spIObserver = new CNS6Adapter_Observer(anObserver);

    return m_pIObserverService->AddObserver(spIObserver, aTopic);
}

//=--------------------------------------------------------------------------=
// CNS6Adapter_ObserverService::RemoveObserver
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
JD_METHOD
CNS6Adapter_ObserverService::RemoveObserver(IObserver *anObserver, const JDUnichar *aTopic)
{
    if (m_pIObserverService == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsIObserver> spIObserver = new CNS6Adapter_Observer(anObserver);

    return m_pIObserverService->RemoveObserver(spIObserver, aTopic);
}

//=--------------------------------------------------------------------------=
// CNS6Adapter_ObserverService::Notify
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
JD_METHOD
CNS6Adapter_ObserverService::Notify(ISupports* aSubject, const JDUnichar *aTopic)
{
    return JD_ERROR_NOT_IMPLEMENTED;
}
