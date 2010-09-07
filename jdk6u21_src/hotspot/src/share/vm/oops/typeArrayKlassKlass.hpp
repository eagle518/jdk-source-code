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

// A typeArrayKlassKlass is the klass of a typeArrayKlass

class typeArrayKlassKlass : public arrayKlassKlass {
 public:
  // Testing
  bool oop_is_typeArrayKlass() const { return true; }

  // Dispatched operation
  int oop_size(oop obj) const { return typeArrayKlass::cast(klassOop(obj))->object_size(); }
  int klass_oop_size() const  { return object_size(); }

  // Allocation
  DEFINE_ALLOCATE_PERMANENT(typeArrayKlassKlass);
  static klassOop create_klass(TRAPS);

  // Casting from klassOop
  static typeArrayKlassKlass* cast(klassOop k) {
    assert(k->klass_part()->oop_is_klass(), "cast to typeArrayKlassKlass");
    return (typeArrayKlassKlass*) k->klass_part();
  }

  // Sizing
  static int header_size() { return oopDesc::header_size() + sizeof(typeArrayKlassKlass)/HeapWordSize; }
  int object_size() const  { return align_object_size(header_size()); }

 public:
  // Printing
  void oop_print_value_on(oop obj, outputStream* st);
#ifndef PRODUCT
  void oop_print_on(oop obj, outputStream* st);
#endif //PRODUCT

  const char* internal_name() const;
};
