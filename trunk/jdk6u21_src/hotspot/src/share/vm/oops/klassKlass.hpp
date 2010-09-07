/*
 * Copyright (c) 1997, 2006, Oracle and/or its affiliates. All rights reserved.
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

// A klassKlass serves as the fix point of the klass chain.
// The klass of klassKlass is itself.

class klassKlass: public Klass {
  friend class VMStructs;
 private:
  juint    _alloc_size;        // allocation profiling support
 public:
  // Testing
  bool oop_is_klass()  const { return true; }
  bool is_leaf_class() const { return true; }

  // Sizing
  int oop_size(oop obj) const;
  int klass_oop_size() const { return object_size(); }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(klassKlass);
  static klassOop create_klass(TRAPS );

  // Casting from klassOop
  static klassKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_klass(), "cast to klassKlass");
    return (klassKlass*) k->klass_part();
  }

  // Sizing
  static int header_size()  { return oopDesc::header_size() + sizeof(klassKlass)/HeapWordSize; }
  int object_size() const   { return align_object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);

  // Parallel Scavenge and Parallel Old
  PARALLEL_GC_DECLS

  // Iterators
  int  oop_oop_iterate(oop obj, OopClosure* blk);
  int  oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

  // Allocation profiling support
  juint alloc_size() const              { return _alloc_size; }
  void set_alloc_size(juint n)          { _alloc_size = n; }

 public:
  // Printing
  void oop_print_value_on(oop obj, outputStream* st);
#ifndef PRODUCT
  void oop_print_on      (oop obj, outputStream* st);
#endif //PRODUCT

  // Verification
  const char* internal_name() const;
  void oop_verify_on(oop obj, outputStream* st);
};
