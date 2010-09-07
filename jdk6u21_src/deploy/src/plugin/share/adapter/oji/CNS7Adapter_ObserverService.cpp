/*
 * @(#)CNS7Adapter_ObserverService.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter_ObserverService.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter_ObserverService.cpp is Implementation of adapter for nsIObserverService
// 
//
#include "StdAfx.h"
#include "nsIObserverService.h"
#include "IObserverService.h"
#include "nsIObserver.h"
#include "IObserver.h"
#include "CNS7Adapter_Observer.h"
#include "CNS7Adapter_ObserverService.h"

//ISupports
JD_IMPL_ISUPPORTS1(CNS7Adapter_ObserverService, IObserverService);

//=--------------------------------------------------------------------------=
// CNS7Adapter_ObserverService::CNS7Adapter_ObserverService
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
CNS7Adapter_ObserverService::CNS7Adapter_ObserverService(nsIObserverService* pIObserverService) :
m_pIObserverService(pIObserverService)
{
    JD_INIT_REFCNT();

    if (m_pIObserverService)
	m_pIObserverService->AddRef();
}

//=--------------------------------------------------------------------------=
// CNS7Adapter_ObserverService::~CNS7Adapter_ObserverService
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
CNS7Adapter_ObserverService::~CNS7Adapter_ObserverService(void)
{
    if (m_pIObserverService)
	m_pIObserverService->Release();
}

//=--------------------------------------------------------------------------=
// CNS7Adapter_ObserverService::AddObserver
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
JD_METHOD
CNS7Adapter_ObserverService::AddObserver(IObserver *anObserver, const JDUnichar *aTopic)
{
#ifdef XP_WIN
    if (m_pIObserverService == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsIObserver> spIObserver = new CNS7Adapter_Observer(anObserver);
    
    char pMultiBytes[256];
    int len =  ::WideCharToMultiByte(CP_ACP, 0, aTopic, -1, pMultiBytes, 0, NULL, NULL);
    if (len > 0 && len < 256)
        ::WideCharToMultiByte(CP_ACP, 0, aTopic, -1, pMultiBytes, len, NULL, NULL);
    else
        return JD_ERROR_FAILURE;
    // Netscape 7 changed the nsIObserverService interface to require the second
    // param to be const char* instead of const PRUnichar*
    return m_pIObserverService->AddObserver(spIObserver, (const char*)pMultiBytes, 
					    PR_FALSE);
#else
    return NS_ERROR_NOT_IMPLEMENTED;
#endif
}

//=--------------------------------------------------------------------------=
// CNS7Adapter_ObserverService::RemoveObserver
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
JD_METHOD
CNS7Adapter_ObserverService::RemoveObserver(IObserver *anObserver, const JDUnichar *aTopic)
{
#ifdef XP_WIN
    if (m_pIObserverService == NULL)
	return JD_ERROR_NULL_POINTER;

    JDSmartPtr<nsIObserver> spIObserver = new CNS7Adapter_Observer(anObserver);
    
    char pMultiBytes[256];
    int len =  ::WideCharToMultiByte(CP_ACP, 0, aTopic, -1, pMultiBytes, 0, NULL, NULL);
    if (len > 0 && len < 256)
        ::WideCharToMultiByte(CP_ACP, 0, aTopic, -1, pMultiBytes, len, NULL, NULL);
    else
        return JD_ERROR_FAILURE;
    
    return m_pIObserverService->RemoveObserver(spIObserver, (const char*)pMultiBytes);
#else
    return NS_ERROR_NOT_IMPLEMENTED;
#endif
}

//=--------------------------------------------------------------------------=
// CNS7Adapter_ObserverService::Notify
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
JD_METHOD
CNS7Adapter_ObserverService::Notify(ISupports* aSubject, const JDUnichar *aTopic)
{
    return JD_ERROR_NOT_IMPLEMENTED;
}
