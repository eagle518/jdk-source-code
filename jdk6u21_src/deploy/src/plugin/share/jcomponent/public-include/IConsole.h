/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IConsole__)
#define __IConsole__

#include "IEgo.h"

interface IConsole : public IEgo {
public:
    virtual void showConsole()=0;
};
#endif

