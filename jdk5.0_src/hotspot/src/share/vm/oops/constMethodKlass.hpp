#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)constMethodKlass.hpp	1.2 03/12/23 16:41:45 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// an constMethodKlass is the klass of a constMethodOop

class constMethodKlass : public Klass {
  friend class VMStructs;
private:
  juint    _alloc_size;        // allocation profiling support
public:
  // Testing
  bool oop_is_constMethod() const { return true; }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(constMethodKlass);
  constMethodOop allocate(int byte_code_size, int compressed_line_number_size,
                          int localvariable_table_length,
                          int checked_exceptions_length, TRAPS);
  static klassOop create_klass(TRAPS);

  // Sizing
  int oop_size(oop obj) const;
  int klass_oop_size() const     { return object_size(); }

  // Casting from klassOop
  static constMethodKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_constMethod(), "cast to constMethodKlass");
    return (constMethodKlass*) k->klass_part(); 
  }

  // Sizing
  static int header_size() {
    return oopDesc::header_size() + sizeof(constMethodKlass)/HeapWordSize;
  }
  int object_size() const {
    return align_object_size(header_size());
  }

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);

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

