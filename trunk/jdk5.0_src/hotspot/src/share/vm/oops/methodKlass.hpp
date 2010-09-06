#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)methodKlass.hpp	1.39 04/07/13 10:07:14 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// a methodKlass is the klass of a methodOop

class methodKlass : public Klass {
  friend class VMStructs;
 private:
  juint    _alloc_size;        // allocation profiling support
 public:
  // Testing
  bool oop_is_method() const { return true; }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(methodKlass);
  methodOop allocate(constMethodHandle xconst, AccessFlags access_flags,
                     TRAPS);
  static klassOop create_klass(TRAPS);

  // Sizing
  int oop_size(oop obj) const;
  int klass_oop_size() const     { return object_size(); }

  // Casting from klassOop
  static methodKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_method(), "cast to methodKlass");
    return (methodKlass*) k->klass_part(); 
  }

  // Sizing
  static int header_size()       { return oopDesc::header_size() + sizeof(methodKlass)/HeapWordSize; }
  int object_size() const        { return align_object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);
  bool oop_is_parsable(oop obj) const;

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  // Allocation profiling support
  juint alloc_size() const              { return _alloc_size; }
  void set_alloc_size(juint n)          { _alloc_size = n; }

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on      (oop obj, outputStream* st);
  void oop_print_value_on(oop obj, outputStream* st);
  const char* internal_name() const;

  // Verify operations
  void oop_verify_on(oop obj, outputStream* st);
  bool oop_partially_loaded(oop obj) const;
  void oop_set_partially_loaded(oop obj);
#endif
};

