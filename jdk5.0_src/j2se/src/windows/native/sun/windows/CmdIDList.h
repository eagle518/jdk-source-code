/*
 * @(#)CmdIDList.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef CMDIDLIST_H
#define CMDIDLIST_H

#include "awt.h"
#include "awt_Object.h"

// Mapping from command ids to objects.
class AwtCmdIDList {
public:
    AwtCmdIDList();
    ~AwtCmdIDList();

    UINT Add(AwtObject* obj);
    AwtObject* Lookup(UINT id);
    void Remove(UINT id);

    CriticalSection    m_lock;

private:

    // next_free_index is used to build a list of free ids.  Since the
    // array index is less then 32k, we can't confuse in-use entry
    // (pointer) with an index of the next free entry.  NIL is -1.
    union CmdIDEntry {
	int next_free_index;	// index of the next entry in the free list
	AwtObject *obj;		// object that is assigned this id
    };

    CmdIDEntry *m_array;  // the vector's contents

    int m_first_free;	  // head of the free list, may be -1 (nil)
    UINT m_capacity;	  // size of currently allocated m_array

    void BuildFreeList(UINT first_index);
};


#endif // CMDIDLIST_H
