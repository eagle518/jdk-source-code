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

class constantPoolCacheKlass: public Klass {
  juint    _alloc_size;        // allocation profiling support
 public:
  // Dispatched klass operations
  bool oop_is_constantPoolCache() const          { return true; }
  int  oop_size(oop obj) const;
  int  klass_oop_size() const                    { return object_size(); }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(constantPoolCacheKlass);
  constantPoolCacheOop allocate(int length, bool is_conc_safe, TRAPS);
  static klassOop create_klass(TRAPS);

  // Casting from klassOop
  static constantPoolCacheKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_constantPoolCache(), "cast to constantPoolCacheKlass");
    return (constantPoolCacheKlass*)k->klass_part();
  }

  // Sizing
  static int header_size()       { return oopDesc::header_size() + sizeof(constantPoolCacheKlass)/HeapWordSize; }
  int object_size() const        { return align_object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int oop_adjust_pointers(oop obj);
  virtual bool oop_is_conc_safe(oop obj) const;

  // Parallel Scavenge and Parallel Old
  PARALLEL_GC_DECLS

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

  // Allocation profiling support
  juint alloc_size() const              { return _alloc_size; }
  void set_alloc_size(juint n)          { _alloc_size = n; }

 public:
  // Printing
  void oop_print_value_on(oop obj, outputStream* st);
#ifndef PRODUCT
  void oop_print_on(oop obj, outputStream* st);
#endif

 public:
  // Verification
  const char* internal_name() const;
  void oop_verify_on(oop obj, outputStream* st);
};
