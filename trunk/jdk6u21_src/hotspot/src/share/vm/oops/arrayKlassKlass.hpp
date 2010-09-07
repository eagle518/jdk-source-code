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

// arrayKlassKlass is the abstract baseclass for all array class classes

class arrayKlassKlass : public klassKlass {
 public:
   // Testing
  bool oop_is_arrayKlass() const { return true; }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(arrayKlassKlass);
  static klassOop create_klass(TRAPS);

  // Casting from klassOop
  static arrayKlassKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_klass(), "cast to arrayKlassKlass");
    return (arrayKlassKlass*) k->klass_part();
  }

  // Sizing
  static int header_size()      { return oopDesc::header_size() + sizeof(arrayKlassKlass)/HeapWordSize; }
  int object_size() const       { return align_object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int oop_adjust_pointers(oop obj);
  bool oop_is_parsable(oop obj) const;

  // Parallel Scavenge and Parallel Old
  PARALLEL_GC_DECLS

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

 public:
  // Printing
  void oop_print_value_on(oop obj, outputStream* st);
#ifndef PRODUCT
  void oop_print_on(oop obj, outputStream* st);
#endif //PRODUCT

  // Verification
  const char* internal_name() const;
  void oop_verify_on(oop obj, outputStream* st);
};
