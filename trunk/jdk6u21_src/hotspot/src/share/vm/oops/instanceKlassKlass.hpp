/*
 * Copyright (c) 1997, 2009, Oracle and/or its affiliates. All rights reserved.
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
                                   unsigned int nonstatic_oop_map_count,
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

  // Parallel Scavenge and Parallel Old
  PARALLEL_GC_DECLS

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

private:
  // Apply closure to the InstanceKlass oops that are outside the java heap.
  inline void iterate_c_heap_oops(instanceKlass* ik, OopClosure* closure);

 public:
  // Printing
  void oop_print_value_on(oop obj, outputStream* st);
#ifndef PRODUCT
  void oop_print_on(oop obj, outputStream* st);
#endif

  // Verification
  const char* internal_name() const;
  void oop_verify_on(oop obj, outputStream* st);
  // tells whether obj is partially constructed (gc during class loading)
  bool oop_partially_loaded(oop obj) const;
  void oop_set_partially_loaded(oop obj);
};
