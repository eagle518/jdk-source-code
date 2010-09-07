/*
 * @(#)MTSafeArray.h	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


#ifndef _MTSAFEARRAY_H_
#define _MTSAFEARRAY_H_

#include "awt.h"
#include "awt_Toolkit.h"

/**
 * Virtual base class used for elements of the MTSafeArray.  This allows
 * MTSaeArray to clean up array by calling delete on the objects in the
 * array upon destruction.
 */
class MTSafeArrayElement {
public:
    virtual ~MTSafeArrayElement() {};
};

class MTSafeArray {

private:
    MTSafeArrayElement	**safeArray;
    CriticalSection	arrayLock;
    int			refCount;
    int			numElements;

public:
			MTSafeArray(int numElements);
    void		AddElement(MTSafeArrayElement *element, int index);
    void		AddReference();
    MTSafeArrayElement	**Lock();
    void		Unlock();
    MTSafeArrayElement	*GetElementReference(int index);
    void		RemoveReference();

};


#endif _MTSAFEARRAY_H_

