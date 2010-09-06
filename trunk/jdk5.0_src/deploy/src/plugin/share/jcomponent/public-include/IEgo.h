/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IEgo__)
#define __IEgo__

#include "jpom.h"

interface IEgo {
    virtual JRESULT QI(const IID&, void **) = 0;
    virtual unsigned long add() = 0;
    virtual unsigned long release() = 0;
};
#endif

