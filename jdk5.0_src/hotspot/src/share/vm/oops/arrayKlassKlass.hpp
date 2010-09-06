#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)arrayKlassKlass.hpp	1.29 03/12/23 16:41:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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

  // ParallelScavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on(oop obj, outputStream* st);
  void oop_print_value_on(oop obj, outputStream* st);
  const char* internal_name() const;

  // Verification
  void oop_verify_on(oop obj, outputStream* st);
#endif
};

