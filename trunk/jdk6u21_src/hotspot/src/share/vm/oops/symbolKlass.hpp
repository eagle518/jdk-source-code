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

// a symbolKlass is the klass for a symbolOop

class symbolKlass : public Klass {
  friend class VMStructs;
 private:
  juint    _alloc_size;        // allocation profiling support
 public:
  // Allocation
  DEFINE_ALLOCATE_PERMANENT(symbolKlass);
  static klassOop create_klass(TRAPS);
  symbolOop allocate_symbol(u1* name, int len, TRAPS);   // Assumes no characters larger than 0x7F
  bool allocate_symbols(int names_count, const char** names, int* lengths, symbolOop* sym_oops, TRAPS);

  // Test operation
  bool oop_is_symbol() const { return true; }

  // Casting from klassOop
  static symbolKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_symbol(), "cast to symbolKlass");
    return (symbolKlass*) k->klass_part();
  }

  static int header_size()       { return oopDesc::header_size() + sizeof(symbolKlass)/HeapWordSize; }
  int oop_size(oop obj) const;
  int klass_oop_size() const     { return object_size(); }
  int object_size() const        { return align_object_size(header_size()); }

    // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);
  bool oop_is_parsable(oop obj) const;

  // Parallel Scavenge and Parallel Old
  PARALLEL_GC_DECLS

  // Allocation profiling support
  juint alloc_size() const              { return _alloc_size; }
  void set_alloc_size(juint n)          { _alloc_size = n; }

  // Iterators
  int  oop_oop_iterate(oop obj, OopClosure* blk);
  int  oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

  // Printing
  void oop_print_value_on(oop obj, outputStream* st);
#ifndef PRODUCT
  void oop_print_on(oop obj, outputStream* st);
#endif //PRODUCT
  const char* internal_name() const;
};
