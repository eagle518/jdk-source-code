#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)compiledICHolderKlass.hpp	1.21 03/12/23 16:41:44 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class CMSIsAliveClosure;

// a compiledICHolderKlass is the klass of a compiledICHolderOop

class compiledICHolderKlass : public Klass {
  friend class VMStructs;
 private:
  juint    _alloc_size;        // allocation profiling support
 public:
  // Testing
  bool oop_is_compiledICHolder() const { return true; }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(compiledICHolderKlass);
  compiledICHolderOop allocate(TRAPS);
  static klassOop create_klass(TRAPS);

  // Sizing
  int oop_size(oop obj) const;
  int klass_oop_size() const     { return object_size(); }

  // Casting from klassOop
  static compiledICHolderKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_compiledICHolder(), "cast to compiledICHolderKlass");
    return (compiledICHolderKlass*) k->klass_part(); 
  }

  // Sizing
  static int header_size()       { return oopDesc::header_size() + sizeof(compiledICHolderKlass)/HeapWordSize; }
  int object_size() const        { return align_object_size(header_size()); }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);
  bool oop_being_unloaded(BoolObjectClosure* is_alive, oop obj);

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  // Allocation profiling support
  juint alloc_size() const              { return _alloc_size; }
  void set_alloc_size(juint n)          { _alloc_size = n; }

  // Iterators
  int  oop_oop_iterate(oop obj, OopClosure* blk);
  int  oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on      (oop obj, outputStream* st);
  void oop_print_value_on(oop obj, outputStream* st);
  const char* internal_name() const;

  // Verification
  void oop_verify_on(oop obj, outputStream* st);
#endif
};

