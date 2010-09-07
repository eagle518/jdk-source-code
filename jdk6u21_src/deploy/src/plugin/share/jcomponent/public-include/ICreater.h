/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__ICreater__)
#define __ICreater__

#include "IEgo.h"
#include "IJavaInstance.h"
#include "IJavaInstanceCB.h"
#include "IFDMonitor.h"

interface ICreater : public IEgo {
public:
    virtual JRESULT createJavaInstance(const char*, 
                                       int, const char**, const char**,
				       IJavaInstanceCB *, IJavaInstance **)=0;
    virtual void setFDMonitor(IFDMonitor *)=0;
};
#endif

