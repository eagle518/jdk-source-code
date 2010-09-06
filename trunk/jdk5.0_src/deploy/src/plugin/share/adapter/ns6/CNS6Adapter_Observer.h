/*
 * @(#)CNS6Adapter_Observer.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter_Observer.h by X.Lu
//
///=--------------------------------------------------------------------------=
//
// CNS6Adapter_Observer.h is Declaration of adapter for nsIObserver
// 
//
#ifndef __CNS6Adapter_Observer_H_
#define __CNS6Adapter_Observer_H_

#include "nsIObserver.h"

class IObserver;

class CNS6Adapter_Observer : public nsIObserver
{
public:
    //nsISupports
    NS_DECL_ISUPPORTS

    // nsIObserver
    NS_IMETHOD
    Observe(nsISupports* aSubject, const PRUnichar *aTopic, const PRUnichar *aData);

    // CNS6Adapter_Observer methods
    CNS6Adapter_Observer(IObserver* pIObserver);

    virtual ~CNS6Adapter_Observer(void);
private:
    IObserver* m_pIObserver;
};

#endif // __CNS6Adapter_Observer_H_
