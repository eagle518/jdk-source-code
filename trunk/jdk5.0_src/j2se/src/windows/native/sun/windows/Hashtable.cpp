/*
 * @(#)Hashtable.cpp	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "Hashtable.h"

Hashtable::Hashtable(const char* name, void (*deleteProc)(void*), 
                     int initialCapacity, float loadFactor) {
    DASSERT ((initialCapacity > 0) && (loadFactor > 0.0));

    table = (HashtableEntry**)
        safe_Calloc(initialCapacity, sizeof(HashtableEntry*));
        
    capacity = initialCapacity;
    count = 0;
    threshold = (int)(capacity * loadFactor);
    this->loadFactor = loadFactor;
    m_deleteProc = deleteProc;

#ifdef DEBUG
    m_name = (char*)name;
    m_max = 0;
    m_collisions = 0;
#else
    name;  // suppress "unused parameter" warning
#endif
}

Hashtable::~Hashtable()
{
#ifdef DEBUG
    DTRACE_PRINTLN3("%s: %d entries, %d maximum entries\n", m_name, count, m_max);
#endif
    clear();
    free(table);
}

BOOL Hashtable::contains(void* value) {
    DASSERT(value != NULL);

    CriticalSection::Lock l(lock);

    for (int i = capacity; i-- > 0;) {
        for (HashtableEntry* e = table[i] ; e != NULL ; e = e->next) {
            if (e->value == value) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

BOOL Hashtable::containsKey(void* key) {
    CriticalSection::Lock l(lock);
    int index = static_cast<int>(((reinterpret_cast<INT_PTR>(key) << 1) >> 1)
        % capacity);
    for (HashtableEntry* e = table[index]; e != NULL; e = e->next) {
        if (e->hash == (INT_PTR)key && e->key == key) {
            return TRUE;
        }
    }
    return FALSE;
}

void* Hashtable::get(void* key) {
    CriticalSection::Lock l(lock);
    int index = static_cast<int>(((reinterpret_cast<INT_PTR>(key) << 1) >> 1)
        % capacity);
    for (HashtableEntry* e = table[index]; e != NULL; e = e->next) {
        if (e->hash == (INT_PTR)key && e->key == key) {
            return e->value;
        }
    }
    return NULL;
}

void Hashtable::rehash() {
    int oldCapacity = capacity;
    HashtableEntry** oldTable = table;

    int newCapacity = oldCapacity * 2 + 1;
    HashtableEntry** newTable = (HashtableEntry**)safe_Calloc(
        newCapacity, sizeof(HashtableEntry*));

    threshold = (int)(newCapacity * loadFactor);
    table = newTable;
    capacity = newCapacity;

    for (int i = 0; i < oldCapacity; i++) {
        for (HashtableEntry* old = oldTable[i] ; old != NULL ; ) {
            HashtableEntry* e = old;
            old = old->next;
            int index = static_cast<int>(((e->hash << 1) >> 1) % newCapacity);
            e->next = newTable[index];
            newTable[index] = e;
        }
    }

    free(oldTable);
}

void* Hashtable::put(void* key, void* value) {
    DASSERT(value != NULL);
    CriticalSection::Lock l(lock);

    // Makes sure the key is not already in the hashtable.
    int index = (int)(((INT_PTR)key << 1) >> 1) % capacity;
    for (HashtableEntry* e = table[index]; e != NULL; e = e->next) {
#ifdef DEBUG
        m_collisions++;
#endif
        if (e->hash == (INT_PTR)key && e->key == key) {
            void* old = e->value;
            e->value = value;
            return old;
	}
    }

    if (count >= threshold) {
        // Rehash the table if the threshold is exceeded
        rehash();
        return put(key, value);
    } 

    // Creates the new entry.
    e = new HashtableEntry();
    e->hash = (INT_PTR)key;
    e->key = key;
    e->value = value;
    e->next = table[index];
    table[index] = e;
    count++;
#ifdef DEBUG
    if (count > m_max) {
        m_max = count;
    }
#endif
    return NULL;
}

void* Hashtable::remove(void* key) {
    CriticalSection::Lock l(lock);
    int index = (int)(((INT_PTR)key << 1) >> 1) % capacity;
    HashtableEntry* prev = NULL;
    for (HashtableEntry* e = table[index]; e != NULL ; prev = e, e = e->next) {
        if (e->key == key) {
            void* value = e->value;
            if (prev != NULL) {
                prev->next = e->next;
            } else {
                table[index] = e->next;
            }
            count--;
            delete e;
            return value;
        }
    }
    return NULL;
}

void Hashtable::clear() {
    CriticalSection::Lock l(lock);
    for (int index = capacity; --index >= 0; ) {
        HashtableEntry* e = table[index];
        while (e != NULL) {
            HashtableEntry* next = e->next;
            if (m_deleteProc) {
                (*m_deleteProc)(e->value);
            }
            delete e;
            e = next;
        }
        table[index] = NULL;
    }
    count = 0;
}

HashtableEnumerator::HashtableEnumerator(HashtableEntry* table[], int size, 
                                         BOOL keys) 
{
    this->table = table;
    this->keys = keys;
    this->index = size;
    this->entry = NULL;
}
	
BOOL HashtableEnumerator::hasMoreElements() {
    if (entry != NULL) {
        return TRUE;
    }
    while (index-- > 0) {
        if ((entry = table[index]) != NULL) {
            return TRUE;
        }
    }
    return FALSE;
}

void* HashtableEnumerator::nextElement() {
    if (entry == NULL) {
        while ((index-- > 0) && ((entry = table[index]) == NULL));
    }
    if (entry != NULL) {
        HashtableEntry* e = entry;
        entry = e->next;
        return keys ? e->key : e->value;
    }
    DASSERT(FALSE);  // shouldn't get here
    return NULL;
}
