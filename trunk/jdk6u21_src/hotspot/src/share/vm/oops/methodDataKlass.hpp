/*
 * Copyright (c) 2000, 2006, Oracle and/or its affiliates. All rights reserved.
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

// a methodDataKlass is the klass of a methodDataOop

class methodDataKlass : public Klass {
  friend class VMStructs;
 private:
  juint _alloc_size; // allocation profiling support
 public:
  // Testing
  bool oop_is_methodData() const { return true; }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(methodDataKlass);
  methodDataOop allocate(methodHandle method, TRAPS);
  static klassOop create_klass(TRAPS);

  // Sizing
  int oop_size(oop obj) const;
  int klass_oop_size() const { return object_size(); }

  // Casting from klassOop
  static methodDataKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_methodData(), "cast to methodDataKlass");
    return (methodDataKlass*) k->klass_part();
  }

  // Sizing
  static int header_size() {
    return oopDesc::header_size() + sizeof(methodDataKlass)/wordSize;
  }
  int object_size() const {
    return align_object_size(header_size());
  }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);
  bool oop_is_parsable(oop obj) const;

  // Parallel Scavenge and Parallel Old
  PARALLEL_GC_DECLS

  // Allocation profiling support
  juint alloc_size() const { return _alloc_size; }
  void  set_alloc_size(juint n) { _alloc_size = n; }

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

 public:
  // Printing
  void oop_print_value_on(oop obj, outputStream* st);
#ifndef PRODUCT
  void oop_print_on      (oop obj, outputStream* st);
#endif //PRODUCT

  // Verify operations
  const char* internal_name() const;
  void oop_verify_on(oop obj, outputStream* st);
};
