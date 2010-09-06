/*
 * @(#)CNS6Adapter_ObserverService.h	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

///=--------------------------------------------------------------------------=
//
// CNS6Adapter_ObserverService.h by X.Lu
//
///=--------------------------------------------------------------------------=
// CNS6Adapter_ObserverService.h is Declaration of adapter for nsIObserverService
// 
//
#ifndef __CNS6Adapter_ObserverService_H_
#define __CNS6Adapter_ObserverService_H_

#include "IObserverService.h"

class nsIObserverService;
class IObserver;

class CNS6Adapter_ObserverService : public IObserverService
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

    CNS6Adapter_ObserverService(nsIObserverService* pIObserverService);
    virtual ~CNS6Adapter_ObserverService(void);
private:
    nsIObserverService* m_pIObserverService;
};

#endif //__CNS6Adapter_ObserverService_H_
