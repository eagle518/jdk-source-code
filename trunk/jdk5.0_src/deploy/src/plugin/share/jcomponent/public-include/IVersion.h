/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(__IVersion__)
#define __IVersion__

#include "IEgo.h"

interface IVersion : public IEgo {

    virtual void supportedVersions(const char *** )=0;
    virtual void containingVersions(const char ***)=0;
};
#endif

