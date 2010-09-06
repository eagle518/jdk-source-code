#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)constantPoolKlass.hpp	1.41 03/12/23 16:41:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A constantPoolKlass is the klass of a constantPoolOop

class constantPoolKlass : public arrayKlass {
 public:
  // Dispatched klass operations
  bool oop_is_constantPool() const  { return true; }
  int oop_size(oop obj) const;
  int klass_oop_size() const        { return object_size(); }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(constantPoolKlass);
  constantPoolOop allocate(int length, TRAPS); 
  static klassOop create_klass(TRAPS);

  // Casting from klassOop
  static constantPoolKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_constantPool(), "cast to constantPoolKlass");
    return (constantPoolKlass*) k->klass_part(); 
  }

  // Sizing
  static int header_size()        { return oopDesc::header_size() + sizeof(constantPoolKlass)/HeapWordSize; }
  int object_size() const         { return arrayKlass::object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int oop_adjust_pointers(oop obj);

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on(oop obj, outputStream* st);
  const char* internal_name() const;

  // Verification
  void oop_verify_on(oop obj, outputStream* st);
  // tells whether obj is partially constructed (gc during class loading)
  bool oop_partially_loaded(oop obj) const;
  void oop_set_partially_loaded(oop obj);

  // Compile the world support
  static void preload_and_initialize_all_classes(oop constant_pool, TRAPS);
#endif
};

