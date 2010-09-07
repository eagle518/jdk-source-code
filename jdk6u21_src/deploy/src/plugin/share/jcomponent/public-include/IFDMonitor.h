/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

