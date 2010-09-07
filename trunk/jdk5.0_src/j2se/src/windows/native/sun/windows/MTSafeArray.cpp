/*
 * @(#)MTSafeArray.cpp	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/**
 * This class exists as a utility for anyone that needs an array
 * which is safe for Multi-Threaded access.  For example, the
 * list of current "devices" (monitors) on a system is a global,
 * so that various classes can find out the current state of the
 * devices.  But since this list can be altered at any time, simply
 * using a global array is not sufficient.  
 *
 * The MT-safeness of the array is assured in two ways:
 *	- hide the actual array being used so that access to 
 *	it can only be made from this class within a lock on
 *	that array (using a CriticalSection).
 *	- Do not delete the array until all references to the
 *	array have released it.  That way, anyone that happens
 *	to have a pointer to an element of the array can still
 *	safely refer to that item, even if the situation has
 *	changed and the array is out of date.
 *
 * See awt_Win32GraphicsDevice.* for examples of usage.
 */

#include "MTSafeArray.h"

/**
 * Create a new MTSafeArray object with numElements elements.
 */
MTSafeArray::MTSafeArray(int numElements) 
{
    this->numElements = numElements;
    this->refCount = 0;
    safeArray = (MTSafeArrayElement**)safe_Malloc
	(numElements * sizeof(MTSafeArrayElement *));
}

/**
 * Add a new element to the array.
 */
void MTSafeArray::AddElement(MTSafeArrayElement *element, int index) 
{
    arrayLock.Enter();
    safeArray[index] = element;
    arrayLock.Leave();
}

/**
 * Add a reference to the array.  This could be someone that wants
 * to register interest in the array, versus someone that actually
 * holds a reference to an array item (in which case they would
 * call GetElementReference() instead).  This mechanism can keep 
 * the array from being deleted when it has no elements being
 * referenced but is still a valid array to use for new elements
 * or references.
 */
void MTSafeArray::AddReference() 
{
    arrayLock.Enter();
    refCount++;
    arrayLock.Leave();
}

/**
 * Lock the array for access.  This should only be called for very
 * temporary operations since the arrayLock CriticalSection will be 
 * held until Unlock() is called.  Accesses to elements of the array
 * directly are safe in between Lock and Unlock calls.
 */
MTSafeArrayElement **MTSafeArray::Lock() 
{
    arrayLock.Enter();
    return safeArray;
}

/**
 * Unlocks the array.  This releases the CriticalSection held since
 * the last call to Lock().  Accesses to the array directly are
 * not allowed after calls to Unlock().
 */
void MTSafeArray::Unlock() 
{
    arrayLock.Leave();
}

/**
 * Retrieve a pointer to an item in the array and register a
 * reference to the array.  This increases the refCount variable
 * used to track when the array can be deleted.
 */
MTSafeArrayElement *MTSafeArray::GetElementReference(int index) 
{
    MTSafeArrayElement *element;
    arrayLock.Enter();
    ++refCount;
    element = safeArray[index];
    arrayLock.Leave();
    return element;
}

/**
 * Removes reference from the array.  This decreases the refCount
 * of the array.  If the refCount goes to 0, then there are no more
 * references to the array and all of the array elements, the
 * array itself, and this object can be destroyed.
 */
void MTSafeArray::RemoveReference() 
{
    arrayLock.Enter();
    --refCount;
    if (refCount <= 0) {
	for (int i = 0; i < numElements; ++i) {
	    delete safeArray[i];
	}
	free(safeArray);
	arrayLock.Leave();
	delete this;
    } else {
	arrayLock.Leave();
    }
}


