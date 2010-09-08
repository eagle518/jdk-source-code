/*
 * @(#)ObjectList.h	1.17 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef OBJECTLIST_H
#define OBJECTLIST_H

#include "awt.h"
#include "awt_Toolkit.h"
#include "awt_Object.h"

class AwtObjectListItem {
public:
    INLINE AwtObjectListItem(AwtObject* c) {
	obj = c;
	next = NULL;
    }

    AwtObject* obj;
    AwtObjectListItem* next;
};

class AwtObjectList {
public:
    AwtObjectList(); 

    void Add(AwtObject* obj);
    void Remove(AwtObject* obj);
#ifdef DEBUG
    /* Used for sanity checks only. */
    AwtObject* LookUp(AwtObject* obj);
#endif /* DEBUG */
    static void Cleanup(void);

    AwtObjectListItem* m_head;
    CriticalSection    m_lock;
};

extern AwtObjectList theAwtObjectList;

#endif // OBJECTLIST_H
