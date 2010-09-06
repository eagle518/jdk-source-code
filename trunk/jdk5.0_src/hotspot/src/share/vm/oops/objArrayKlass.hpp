#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)objArrayKlass.hpp	1.72 03/12/23 16:42:03 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// objArrayKlass is the klass for objArrays

class objArrayKlass : public arrayKlass {
  friend class VMStructs;
 private:
  klassOop _element_klass;            // The klass of the elements of this array type
  klassOop _bottom_klass;             // The one-dimensional type (instanceKlass or typeArrayKlass)
 public:
  // Instance variables
  klassOop element_klass() const      { return _element_klass; }
  void set_element_klass(klassOop k)  { oop_store_without_check((oop*) &_element_klass, (oop) k); }
  oop* element_klass_addr()           { return (oop*)&_element_klass; }

  klassOop bottom_klass() const       { return _bottom_klass; }
  void set_bottom_klass(klassOop k)   { oop_store_without_check((oop*) &_bottom_klass, (oop) k); }
  oop* bottom_klass_addr()            { return (oop*)&_bottom_klass; }

  // Compiler/Interpreter offset
  static int element_klass_offset_in_bytes() { return (intptr_t)&((objArrayKlass*)NULL)->_element_klass; }

  // Dispatched operation
  bool can_be_primary_super_slow() const;
  objArrayOop compute_secondary_supers(int num_extra_slots, TRAPS);
  bool compute_is_subtype_of(klassOop k);
  bool oop_is_objArray()  const       { return true; }
  int oop_size(oop obj) const;
  int klass_oop_size() const          { return object_size(); }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(objArrayKlass);
  objArrayOop allocate(int length, TRAPS);
  oop multi_allocate(int rank, jint* sizes, int next_dim_step, TRAPS);

  // Copying
  void  copy_array(arrayOop s, int src_pos, arrayOop d, int dst_pos, int length, TRAPS);

  // Compute protection domain
  oop protection_domain() { return Klass::cast(bottom_klass())->protection_domain(); }

 protected:
  // Returns the objArrayKlass for n'th dimension.
  virtual klassOop array_klass_impl(bool or_null, int n, TRAPS);

  // Returns the array class with this class as element type.
  virtual klassOop array_klass_impl(bool or_null, TRAPS);

 public:
  // Casting from klassOop
  static objArrayKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_objArray(), "cast to objArrayKlass");
    return (objArrayKlass*) k->klass_part(); 
  }

  // Sizing
  static int header_size()                { return oopDesc::header_size() + sizeof(objArrayKlass)/HeapWordSize; }
  int object_size() const                 { return arrayKlass::object_size(header_size()); }

  // Initialization (virtual from Klass)
  void initialize(TRAPS);

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

  // Iterators
  int oop_oop_iterate(oop obj, OopClosure* blk) {
    return oop_oop_iterate_v(obj, blk);
  }
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
    return oop_oop_iterate_v_m(obj, blk, mr);
  }
#define ObjArrayKlass_OOP_OOP_ITERATE_DECL(OopClosureType, nv_suffix)   \
  int oop_oop_iterate##nv_suffix(oop obj, OopClosureType* blk);         \
  int oop_oop_iterate##nv_suffix##_m(oop obj, OopClosureType* blk,      \
                                     MemRegion mr);

  ALL_OOP_OOP_ITERATE_CLOSURES_1(ObjArrayKlass_OOP_OOP_ITERATE_DECL)
  ALL_OOP_OOP_ITERATE_CLOSURES_2(ObjArrayKlass_OOP_OOP_ITERATE_DECL)
  ALL_OOP_OOP_ITERATE_CLOSURES_3(ObjArrayKlass_OOP_OOP_ITERATE_DECL)

  // JVM support
  jint compute_modifier_flags(TRAPS) const;

 private:
   static klassOop array_klass_impl   (objArrayKlassHandle this_oop, bool or_null, int n, TRAPS);

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on      (oop obj, outputStream* st);
  void oop_print_value_on(oop obj, outputStream* st);
  const char* internal_name() const;
  // Verification
  void oop_verify_on(oop obj, outputStream* st);
  void oop_verify_old_oop(oop obj, oop* p, bool allow_dirty);
#endif
};

