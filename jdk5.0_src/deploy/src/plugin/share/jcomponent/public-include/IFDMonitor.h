/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IFDMonitor__)
#define __IFDMonitor__

#include "IEgo.h"



interface IFDMonitor : public IEgo {
public:
    virtual void connectFD(int fd, void (*)(void *), void *)=0;
    virtual void disconnectFD(void * data)=0;
};
#endif

