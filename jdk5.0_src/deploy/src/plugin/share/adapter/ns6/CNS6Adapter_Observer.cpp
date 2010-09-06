/*
 * @(#)CNS6Adapter_Observer.cpp	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter_Observer.cpp by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS6Adapter_Observer.cpp: Implementation of adapter for nsIObserver
//

#include "StdAfx.h"
#include "nsIObserver.h"
#include "IObserver.h"
#include "CNS6Adapter_Observer.h"

//nsISupports
NS_IMPL_ISUPPORTS1(CNS6Adapter_Observer, nsIObserver)

//=--------------------------------------------------------------------------=
// CNS6Adapter_Observer::CNS6Adapter_Observer
//=--------------------------------------------------------------------------=
// params:  pIObserver: reference to plug-in side's observer implementation 
// 
// notes :
//
CNS6Adapter_Observer::CNS6Adapter_Observer(IObserver* pIObserver) : m_pIObserver(pIObserver)
{
    NS_INIT_REFCNT();
    if (m_pIObserver)
	m_pIObserver->AddRef();
}

//=--------------------------------------------------------------------------=
// CNS6Adapter_Observer::~CNS6Adapter_Observer
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
CNS6Adapter_Observer::~CNS6Adapter_Observer(void)
{
    if (m_pIObserver)
    {
	m_pIObserver->Release();
	m_pIObserver = NULL;
    }
}

//=--------------------------------------------------------------------------=
// CNS6Adapter_Observer::Observe
//=--------------------------------------------------------------------------=
// params:
// 
// notes :
//
NS_METHOD
CNS6Adapter_Observer::Observe(nsISupports* aSubject, const PRUnichar *aTopic, const PRUnichar *aData)
{
    if (m_pIObserver == NULL)
	return NS_ERROR_NULL_POINTER;

    return m_pIObserver->Observe(NULL, aTopic, aData);
}
