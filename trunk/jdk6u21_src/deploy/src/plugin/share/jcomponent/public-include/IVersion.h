/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IVersion__)
#define __IVersion__

#include "IEgo.h"

interface IVersion : public IEgo {

    virtual void supportedVersions(const char *** )=0;
    virtual void containingVersions(const char ***)=0;
};
#endif

