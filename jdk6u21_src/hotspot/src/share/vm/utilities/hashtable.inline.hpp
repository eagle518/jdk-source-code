/*
 * Copyright (c) 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

// Inline function definitions for hashtable.hpp.


// --------------------------------------------------------------------------
// Hash function

// We originally used hashpjw, but hash P(31) gives just as good results
// and is slighly faster. We would like a hash function that looks at every
// character, since package names have large common prefixes, and also because
// hash_or_fail does error checking while iterating.

// hash P(31) from Kernighan & Ritchie

inline unsigned int Hashtable::hash_symbol(const char* s, int len) {
  unsigned int h = 0;
  while (len-- > 0) {
    h = 31*h + (unsigned) *s;
    s++;
  }
  return h;
}


// --------------------------------------------------------------------------

// Initialize a table.

inline BasicHashtable::BasicHashtable(int table_size, int entry_size) {
  // Called on startup, no locking needed
  initialize(table_size, entry_size, 0);
  _buckets = NEW_C_HEAP_ARRAY(HashtableBucket, table_size);
  for (int index = 0; index < _table_size; index++) {
    _buckets[index].clear();
  }
}


inline BasicHashtable::BasicHashtable(int table_size, int entry_size,
                                      HashtableBucket* buckets,
                                      int number_of_entries) {
  // Called on startup, no locking needed
  initialize(table_size, entry_size, number_of_entries);
  _buckets = buckets;
}


inline void BasicHashtable::initialize(int table_size, int entry_size,
                                       int number_of_entries) {
  // Called on startup, no locking needed
  _table_size = table_size;
  _entry_size = entry_size;
  _free_list = NULL;
  _first_free_entry = NULL;
  _end_block = NULL;
  _number_of_entries = number_of_entries;
#ifdef ASSERT
  _lookup_count = 0;
  _lookup_length = 0;
#endif
}


// The following method is MT-safe and may be used with caution.
inline BasicHashtableEntry* BasicHashtable::bucket(int i) {
  return _buckets[i].get_entry();
}


inline void HashtableBucket::set_entry(BasicHashtableEntry* l) {
  // Warning: Preserve store ordering.  The SystemDictionary is read
  //          without locks.  The new SystemDictionaryEntry must be
  //          complete before other threads can be allowed to see it
  //          via a store to _buckets[index].
  OrderAccess::release_store_ptr(&_entry, l);
}


inline BasicHashtableEntry* HashtableBucket::get_entry() const {
  // Warning: Preserve load ordering.  The SystemDictionary is read
  //          without locks.  The new SystemDictionaryEntry must be
  //          complete before other threads can be allowed to see it
  //          via a store to _buckets[index].
  return (BasicHashtableEntry*) OrderAccess::load_ptr_acquire(&_entry);
}


inline void BasicHashtable::set_entry(int index, BasicHashtableEntry* entry) {
  _buckets[index].set_entry(entry);
}


inline void BasicHashtable::add_entry(int index, BasicHashtableEntry* entry) {
  entry->set_next(bucket(index));
  _buckets[index].set_entry(entry);
  ++_number_of_entries;
}

inline void BasicHashtable::free_entry(BasicHashtableEntry* entry) {
  entry->set_next(_free_list);
  _free_list = entry;
  --_number_of_entries;
}
