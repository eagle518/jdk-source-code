#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objArrayKlassKlass.hpp	1.38 03/12/23 16:42:04 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// The objArrayKlassKlass is klass for all objArrayKlass'

class objArrayKlassKlass : public arrayKlassKlass {
 public:
  // Testing
  virtual bool oop_is_objArrayKlass() const { return true; }

  // Dispatched operation
  int oop_size(oop obj) const { return objArrayKlass::cast(klassOop(obj))->object_size(); }
  int klass_oop_size() const  { return object_size(); }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(objArrayKlassKlass);
  static klassOop create_klass(TRAPS);
  klassOop allocate_objArray_klass(int n, KlassHandle element_klass, TRAPS);
  klassOop allocate_system_objArray_klass(TRAPS); // Used for bootstrapping in Universe::genesis

  // Casting from klassOop
  static objArrayKlassKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_klass(), "cast to objArrayKlassKlass");
    return (objArrayKlassKlass*) k->klass_part(); 
  }

  // Sizing
  static int header_size()  { return oopDesc::header_size() + sizeof(objArrayKlassKlass)/HeapWordSize; }
  int object_size() const   { return align_object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int oop_adjust_pointers(oop obj);

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

 private:
  // helpers
  static klassOop allocate_objArray_klass_impl(objArrayKlassKlassHandle this_oop, int n, KlassHandle element_klass, TRAPS);

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

