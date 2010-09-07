/*
 * Copyright (c) 1998, 2009, Oracle and/or its affiliates. All rights reserved.
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

// The Rewriter adds caches to the constant pool and rewrites bytecode indices
// pointing into the constant pool for better interpreter performance.

class Rewriter: public StackObj {
 private:
  instanceKlassHandle _klass;
  constantPoolHandle  _pool;
  objArrayHandle      _methods;
  intArray            _cp_map;
  intStack            _cp_cache_map;

  void init_cp_map(int length) {
    _cp_map.initialize(length, -1);
    // Choose an initial value large enough that we don't get frequent
    // calls to grow().
    _cp_cache_map.initialize(length / 2);
  }
  int  cp_entry_to_cp_cache(int i) { assert(has_cp_cache(i), "oob"); return _cp_map[i]; }
  bool has_cp_cache(int i) { return (uint)i < (uint)_cp_map.length() && _cp_map[i] >= 0; }
  int maybe_add_cp_cache_entry(int i) { return has_cp_cache(i) ? _cp_map[i] : add_cp_cache_entry(i); }
  int add_cp_cache_entry(int cp_index) {
    assert((cp_index & _secondary_entry_tag) == 0, "bad tag");
    assert(_cp_map[cp_index] == -1, "not twice on same cp_index");
    int cache_index = _cp_cache_map.append(cp_index);
    _cp_map.at_put(cp_index, cache_index);
    assert(cp_entry_to_cp_cache(cp_index) == cache_index, "");
    return cache_index;
  }
  int add_secondary_cp_cache_entry(int main_cpc_entry) {
    assert(main_cpc_entry < _cp_cache_map.length(), "must be earlier CP cache entry");
    int cache_index = _cp_cache_map.append(main_cpc_entry | _secondary_entry_tag);
    return cache_index;
  }

  // All the work goes in here:
  Rewriter(instanceKlassHandle klass, constantPoolHandle cpool, objArrayHandle methods, TRAPS);

  void compute_index_maps();
  void make_constant_pool_cache(TRAPS);
  void scan_method(methodOop m);
  methodHandle rewrite_jsrs(methodHandle m, TRAPS);
  void rewrite_Object_init(methodHandle m, TRAPS);
  int  rewrite_member_reference(address bcp, int offset);
  void rewrite_invokedynamic(address bcp, int offset, int cp_index);

 public:
  // Driver routine:
  static void rewrite(instanceKlassHandle klass, TRAPS);
  static void rewrite(instanceKlassHandle klass, constantPoolHandle cpool, objArrayHandle methods, TRAPS);

  enum {
    _secondary_entry_tag = nth_bit(30)
  };
};
