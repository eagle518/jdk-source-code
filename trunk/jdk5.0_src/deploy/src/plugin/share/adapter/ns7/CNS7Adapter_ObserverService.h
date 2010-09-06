/*
 * @(#)CNS7Adapter_ObserverService.h	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS7Adapter_ObserverService.h by X.Lu
//
///=--------------------------------------------------------------------------=
// CNS7Adapter_ObserverService.h is Declaration of adapter for nsIObserverService
// 
//
#ifndef __CNS7Adapter_ObserverService_H_
#define __CNS7Adapter_ObserverService_H_

#include "IObserverService.h"

class nsIObserverService;
class IObserver;

class CNS7Adapter_ObserverService : public IObserverService
{
public:
    JD_DECL_ISUPPORTS

    // Add an observer
    JD_IMETHOD
    AddObserver(IObserver *anObserver, const JDUnichar *aTopic);

    // Remove an observer
    JD_IMETHOD
    RemoveObserver(IObserver *anObserver, const JDUnichar *aTopic);

    // Notify the observer
    JD_IMETHOD
    Notify(ISupports* aSubject, const JDUnichar *aTopic);

    CNS7Adapter_ObserverService(nsIObserverService* pIObserverService);
    virtual ~CNS7Adapter_ObserverService(void);
private:
    nsIObserverService* m_pIObserverService;
};

#endif //__CNS7Adapter_ObserverService_H_
