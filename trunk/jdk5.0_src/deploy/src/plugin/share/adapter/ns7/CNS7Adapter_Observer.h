/*
 * @(#)CNS7Adapter_Observer.h	1.1 02/11/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter_Observer.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS7Adapter_Observer.h is Declaration of adapter for nsIObserver
// 
//
#ifndef __CNS7Adapter_Observer_H_
#define __CNS7Adapter_Observer_H_

#include "nsIObserver.h"

class IObserver;

class CNS7Adapter_Observer : public nsIObserver
{
public:
    //nsISupports
    NS_DECL_ISUPPORTS

    // nsIObserver
    NS_IMETHOD
    Observe(nsISupports* aSubject, const char *aTopic, const PRUnichar *aData);

    // CNS7Adapter_Observer methods
    CNS7Adapter_Observer(IObserver* pIObserver);

    virtual ~CNS7Adapter_Observer(void);
private:
    IObserver* m_pIObserver;
};

#endif // __CNS7Adapter_Observer_H_
