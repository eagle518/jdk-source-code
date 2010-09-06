#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)typeArrayKlass.hpp	1.57 03/12/23 16:42:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// A typeArrayKlass is the klass of a typeArray
// It contains the type and size of the elements

class typeArrayKlass : public arrayKlass {
  friend class VMStructs;
 private:
  int _type;                  // type of the elements
  int _scale;                 // size of the elements
  int _max_length;            // maximum number of elements allowed in an array
 public:
  // instance variables
  BasicType  type() const               { return (BasicType) _type; }
  void set_type(BasicType t)            { _type = (int) t; }

  int  scale() const                    { return _scale; }
  void set_scale(int s)                 { _scale = s;    }

  int  max_length()                     { return _max_length; }
  void set_max_length(int m)            { _max_length = m;    }

  // testers
  bool oop_is_typeArray() const         { return true; }

  // klass allocation
  DEFINE_ALLOCATE_PERMANENT(typeArrayKlass);
  static klassOop create_klass(BasicType type, int scale, TRAPS);

  int oop_size(oop obj) const;
  int klass_oop_size() const  { return object_size(); }

  bool compute_is_subtype_of(klassOop k);

  // Allocation
  typeArrayOop allocate(int length, TRAPS);
  typeArrayOop allocate_permanent(int length, TRAPS);  // used for class file structures
  oop multi_allocate(int rank, jint* sizes, int next_dim_step, TRAPS);

  // Copying
  void  copy_array(arrayOop s, int src_pos, arrayOop d, int dst_pos, int length, TRAPS);

  // Iteration
  int oop_oop_iterate(oop obj, OopClosure* blk);
  int oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr);

  // Garbage collection
  void oop_follow_contents(oop obj);
  int  oop_adjust_pointers(oop obj);

  // Parallel Scavenge
  void oop_copy_contents(PSPromotionManager* pm, oop obj);

 protected:
  // Find n'th dimensional array
  virtual klassOop array_klass_impl(bool or_null, int n, TRAPS);

  // Returns the array class with this class as element type
  virtual klassOop array_klass_impl(bool or_null, TRAPS);

 public:
  // Casting from klassOop
  static typeArrayKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_typeArray(), "cast to typeArrayKlass");
    return (typeArrayKlass*) k->klass_part(); 
  }

  // Naming
  static const char* external_name(BasicType type);

  // Sizing
  static int header_size()  { return oopDesc::header_size() + sizeof(typeArrayKlass)/HeapWordSize; }
  int object_size() const   { return arrayKlass::object_size(header_size()); }

  // Initialization (virtual from Klass)
  void initialize(TRAPS);

 private:
   // Helpers
   static klassOop array_klass_impl(typeArrayKlassHandle h_this, bool or_null, int n, TRAPS);

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on(oop obj, outputStream* st);
  const char* internal_name() const;
#endif
};
