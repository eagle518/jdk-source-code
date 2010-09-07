/*
 * @(#)Hashtable.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "awt.h"
#include "awt_Toolkit.h"

struct HashtableEntry {
    INT_PTR hash;
    void* key;
    void* value;
    HashtableEntry* next;
};

class HashtableEnumerator {
private:
    BOOL keys;
    int index;
    HashtableEntry** table;
    HashtableEntry* entry;

public:
    HashtableEnumerator(HashtableEntry* table[], int size, BOOL keys);
    BOOL hasMoreElements();
    void* nextElement();
};

/**
 * Hashtable class. Maps keys to values. Any object can be used as
 * a key and/or value.  As you might guess, this was brazenly stolen
 * from java.util.Hashtable.
 */
class Hashtable {
protected:
    /*
     * The hash table data.
     */
    HashtableEntry** table;

    /*
     * The size of table
     */
    int capacity;

    /*
     * The total number of entries in the hash table.
     */
    int count;

    /**
     * Rehashes the table when count exceeds this threshold.
     */
    int threshold;

    /**
     * The load factor for the hashtable.
     */
    float loadFactor;

    /**
     * Our C++ synchronizer.
     */
    CriticalSection lock;

    /**
     * Element deletion routine, if any.
     */
    void (*m_deleteProc)(void*);

#ifdef DEBUG
    char* m_name;
    int m_max;
    int m_collisions;
#endif    

public:
    /**
     * Constructs a new, empty hashtable with the specified initial 
     * capacity and the specified load factor.
     */
    Hashtable(const char* name, void (*deleteProc)(void*) = NULL, 
              int initialCapacity = 29, float loadFactor = 0.75);

    virtual ~Hashtable();

    /**
     * Returns the number of elements contained in the hashtable. 
     */
    INLINE int size() {
	return count;
    }

    /**
     * Returns true if the hashtable contains no elements.
     */
    INLINE BOOL isEmpty() {
	return count == 0;
    }

    /**
     * Returns an enumeration of the hashtable's keys.
     */
    INLINE HashtableEnumerator* keys() {
        CriticalSection::Lock l(lock);
	return new HashtableEnumerator(table, capacity, TRUE);
    }

    /**
     * Returns an enumeration of the elements. Use the Enumeration methods 
     * on the returned object to fetch the elements sequentially.
     */
    INLINE HashtableEnumerator* elements() {
        CriticalSection::Lock l(lock);
	return new HashtableEnumerator(table, capacity, FALSE);
    }

    /**
     * Returns true if the specified object is an element of the hashtable.
     * This operation is more expensive than the containsKey() method.
     */
    BOOL contains(void* value); 

    /**
     * Returns true if the collection contains an element for the key.
     */
    BOOL containsKey(void* key);

    /**
     * Gets the object associated with the specified key in the 
     * hashtable.
     */
    void* get(void* key);

    /**
     * Puts the specified element into the hashtable, using the specified
     * key.  The element may be retrieved by doing a get() with the same key.
     * The key and the element cannot be null. 
     */
    virtual void* put(void* key, void* value);

    /**
     * Removes the element corresponding to the key. Does nothing if the
     * key is not present.
     */
    void* remove(void* key);

    /**
     * Clears the hash table so that it has no more elements in it.
     */
    void clear();

protected:
    /**
     * Rehashes the content of the table into a bigger table.
     * This method is called automatically when the hashtable's
     * size exceeds the threshold.
     */
    void rehash();
};

#endif // HASHTABLE_H
