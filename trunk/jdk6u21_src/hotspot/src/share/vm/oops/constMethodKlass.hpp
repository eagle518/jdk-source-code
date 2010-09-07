/*
 * Copyright (c) 2003, 2009, Oracle and/or its affiliates. All rights reserved.
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

// A constMethodKlass is the klass of a constMethodOop

class constMethodKlass : public Klass {
  friend class VMStructs;
private:
  juint    _alloc_size;        // allocation profiling support
public:
  // Testing
  bool oop_is_constMethod() const { return true; }
  virtual bool oop_is_parsable(oop obj) const;
  virtual bool oop_is_conc_safe(oop obj) const;


  // Allocation
  DEFINE_ALLOCATE_PERMANENT(constMethodKlass);
  constMethodOop allocate(int byte_code_size, int compressed_line_number_size,
                          int localvariable_table_length,
                          int checked_exceptions_length,
                          bool is_conc_safe,
                          TRAPS);
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

  // Parallel Scavenge and Parallel Old
  PARALLEL_GC_DECLS

  // Allocation profiling support
  juint alloc_size() const              { return _alloc_size; }
  void set_alloc_size(juint n)          { _alloc_size = n; }

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
  bool oop_partially_loaded(oop obj) const;
  void oop_set_partially_loaded(oop obj);
};
