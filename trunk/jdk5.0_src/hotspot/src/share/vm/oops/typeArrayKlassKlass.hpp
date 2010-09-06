#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)typeArrayKlassKlass.hpp	1.17 03/12/23 16:42:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
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

#ifndef PRODUCT
 public:
  // Printing
  void oop_print_on(oop obj, outputStream* st);
  void oop_print_value_on(oop obj, outputStream* st);
  const char* internal_name() const;
#endif
};

