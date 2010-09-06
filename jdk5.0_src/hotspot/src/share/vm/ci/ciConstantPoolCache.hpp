#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciConstantPoolCache.hpp	1.5 03/12/23 16:39:27 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// ciConstantPoolCache
//
// The class caches indexed constant pool lookups.
//
// Usage note: this klass has nothing to do with constantPoolCacheOop.
class ciConstantPoolCache : public ResourceObj {
private:
  GrowableArray<intptr_t>*   _keys;
  GrowableArray<void*>* _elements;

  int find(int index);

public:
  ciConstantPoolCache(Arena* arena, int expected_size);

  // Get the element associated with some index.
  void* get(int index);
	
  // Associate an element with an index.
  void insert(int index, void* element);

  void print();
};

