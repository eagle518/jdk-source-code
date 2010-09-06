#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)instanceKlassKlass.hpp	1.44 03/12/23 16:41:53 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// An instanceKlassKlass is the klass of an instanceKlass

class instanceKlassKlass : public klassKlass {
 public:
  // Dispatched operation
  bool oop_is_klass() const           { return true; }
  bool oop_is_instanceKlass() const   { return true; }

  int oop_size(oop obj) const;
  int klass_oop_size() const    { return object_size(); }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(instanceKlassKlass);
  static klassOop create_klass(TRAPS);
  klassOop allocate_instance_klass(int vtable_len, 
                                   int itable_len, 
                                   int static_field_size, 
                                   int nonstatic_oop_map_size, 
                                   ReferenceType rt, 
                                   TRAPS);

  // Casting from klassOop
  static instanceKlassKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_klass(), "cast to instanceKlassKlass");
    return (instanceKlassKlass*) k->klass_part(); 
  }

  // Sizing
  static int header_size()    { return oopDesc::header_size() + sizeof(instanceKlassKlass)/HeapWordSize; }
  int object_size() const     { return align_object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);
  bool oop_is_parsable(oop obj) const;

  // Parallel Scavenge
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
  // tells whether obj is partially constructed (gc during class loading)
  bool oop_partially_loaded(oop obj) const;
  void oop_set_partially_loaded(oop obj);
#endif
};

