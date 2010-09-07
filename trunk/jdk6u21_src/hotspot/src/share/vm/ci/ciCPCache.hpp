/*
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
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

// ciCPCache
//
// This class represents a constant pool cache.
//
// Note: This class is called ciCPCache as ciConstantPoolCache is used
// for something different.
class ciCPCache : public ciObject {
public:
  ciCPCache(constantPoolCacheHandle cpcache) : ciObject(cpcache) {}

  // What kind of ciObject is this?
  bool is_cpcache() const { return true; }

  // Get the offset in bytes from the oop to the f1 field of the
  // requested entry.
  size_t get_f1_offset(int index);

  void print();
};
