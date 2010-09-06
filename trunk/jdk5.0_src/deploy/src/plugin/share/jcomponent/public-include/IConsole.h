/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IConsole__)
#define __IConsole__

#include "IEgo.h"

interface IConsole : public IEgo {
public:
    virtual void showConsole()=0;
};
#endif

