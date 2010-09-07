/*
 * @(#)CNS7Adapter_Observer.cpp	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter_Observer.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter_Observer.cpp: Implementation of adapter for nsIObserver
//

#include "StdAfx.h"
#include "nsIObserver.h"
#include "IObserver.h"
#include "CNS7Adapter_Observer.h"

//nsISupports
NS_IMPL_ISUPPORTS1(CNS7Adapter_Observer, nsIObserver)

//=--------------------------------------------------------------------------=
// CNS7Adapter_Observer::CNS7Adapter_Observer
//=--------------------------------------------------------------------------=
// params:  pIObserver: reference to plug-in side's observer implementation 
// 
// notes :
//
CNS7Adapter_Observer::CNS7Adapter_Observer(IObserver* pIObserver) : m_pIObserver(pIObserver)
{
    NS_INIT_REFCNT();
    if (m_pIObserver)
	m_pIObserver->AddRef();
}

//=--------------------------------------------------------------------------=
// CNS7Adapter_Observer::~CNS7Adapter_Observer
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
CNS7Adapter_Observer::~CNS7Adapter_Observer(void)
{
    if (m_pIObserver)
    {
	m_pIObserver->Release();
	m_pIObserver = NULL;
    }
}

//=--------------------------------------------------------------------------=
// CNS7Adapter_Observer::Observe
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
NS_METHOD
CNS7Adapter_Observer::Observe(nsISupports* aSubject, const char *aTopic, const PRUnichar *aData)
{
#ifdef XP_WIN
    if (m_pIObserver == NULL)
	return NS_ERROR_NULL_POINTER;

    PRUnichar pWideStr[512];
    int len = ::MultiByteToWideChar(CP_ACP, 0, aTopic, -1, NULL, 0);
    if (len > 0 && len < 256)
        ::MultiByteToWideChar(CP_ACP, 0, aTopic, -1, pWideStr, len);
    else
        return NS_ERROR_FAILURE;
    return m_pIObserver->Observe(NULL, (const PRUnichar*)pWideStr, aData);
#else
    return NS_ERROR_NOT_IMPLEMENTED;
#endif
}
