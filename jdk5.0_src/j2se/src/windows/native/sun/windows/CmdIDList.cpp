/*
 * @(#)CmdIDList.cpp	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "CmdIDList.h"

// How much space to allocate initially
static const UINT ARRAY_INITIAL_SIZE = 1024;

// Array expansion increment when need more free space
static const UINT ARRAY_SIZE_INCREMENT = 1024;

// It seems that Win95 can not handle ids greater than 2**16
static const UINT ARRAY_MAXIMUM_SIZE = 32768;


AwtCmdIDList::AwtCmdIDList()
{
    m_capacity = ARRAY_INITIAL_SIZE;
    m_first_free = -1;
    m_array = (CmdIDEntry *)safe_Malloc(m_capacity * sizeof(AwtObject*));
    BuildFreeList(0);
}

AwtCmdIDList::~AwtCmdIDList()
{
    free(m_array);
}


// Build a new free list from a newly allocated memory.  This only
// happens after malloc/realloc, and new free entries are contiguous
// from first_index to m_capacity-1
INLINE void AwtCmdIDList::BuildFreeList(UINT first_index)
{
    DASSERT(m_first_free == -1);
    for (UINT i = first_index; i < m_capacity-1; ++i)
	m_array[i].next_free_index = i+1;
    m_array[m_capacity-1].next_free_index = -1;	// nil
    m_first_free = first_index;	// head of the free list
}

// Assign an id to the object.  Recycle the first free entry from the
// head of the free list or allocate more memory for a new free list.
UINT AwtCmdIDList::Add(AwtObject* obj)
{
    CriticalSection::Lock l(m_lock);

    if (m_first_free == -1) {	// out of free ids
	if (m_capacity == ARRAY_MAXIMUM_SIZE) {
	    // Really bad - out of ids.  Since we hardly can have *so*
	    // many items simultaneously in existence, we have an id
	    // leak somewhere.
	    DASSERT(FALSE);
	    return 0;
	}
	else {			// snarf a bigger arena
	    UINT old_capacity = m_capacity; // will be the first free entry
	    m_capacity += ARRAY_SIZE_INCREMENT;
	    if (m_capacity > ARRAY_MAXIMUM_SIZE)
		m_capacity = ARRAY_MAXIMUM_SIZE;
	    m_array = (CmdIDEntry *)safe_Realloc(m_array,
				        m_capacity * sizeof(CmdIDEntry*));
	    BuildFreeList(old_capacity);
	}
    }

    DASSERT(m_first_free != -1);
    UINT newid = m_first_free;	// use the entry from the head of the list
    m_first_free = m_array[newid].next_free_index; // advance free pointer
    m_array[newid].obj = obj;

    return newid;
}

// Return the object associated with this id..
AwtObject* AwtCmdIDList::Lookup(UINT id)
{
    CriticalSection::Lock l(m_lock);
    DASSERT(id < m_capacity);
    if (m_array[id].next_free_index <= ARRAY_MAXIMUM_SIZE) {
        return NULL;
    }
    return m_array[id].obj;
}

// Return this id to the head of the free list.
void AwtCmdIDList::Remove(UINT id)
{
    CriticalSection::Lock l(m_lock);
    DASSERT(id < m_capacity);
    DASSERT(m_array[id].next_free_index > ARRAY_MAXIMUM_SIZE); // it's a pointer
    m_array[id].next_free_index = m_first_free;
    m_first_free = id;
}
