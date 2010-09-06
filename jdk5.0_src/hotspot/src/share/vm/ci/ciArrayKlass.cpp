#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciArrayKlass.cpp	1.7 03/12/23 16:39:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciArrayKlass.cpp.incl"

// ciArrayKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in an arrayKlass.

// ------------------------------------------------------------------
// ciArrayKlass::ciArrayKlass
//
// Loaded array klass.
ciArrayKlass::ciArrayKlass(KlassHandle h_k) : ciKlass(h_k) {
  assert(get_Klass()->oop_is_array(), "wrong type");
  _dimension = get_arrayKlass()->dimension();
}

// ------------------------------------------------------------------
// ciArrayKlass::ciArrayKlass
//
// Unloaded array klass.
ciArrayKlass::ciArrayKlass(ciSymbol* name, int dimension, ciKlass* klass)
  : ciKlass(name, klass) {
  _dimension = dimension;
}

// ------------------------------------------------------------------
// ciArrayKlass::element_type
//
// What type is obtained when this array is indexed once?
ciType* ciArrayKlass::element_type() {
  if (is_type_array_klass()) {
    return ciType::make(as_type_array_klass()->element_type());
  } else {
    return as_obj_array_klass()->element_klass()->as_klass();
  }
}


// ------------------------------------------------------------------
// ciArrayKlass::base_element_type
//
// What type is obtained when this array is indexed as many times as possible?
ciType* ciArrayKlass::base_element_type() {
  if (is_type_array_klass()) {
    return ciType::make(as_type_array_klass()->element_type());
  } else {
    ciKlass* ek = as_obj_array_klass()->base_element_klass();
    if (ek->is_type_array_klass()) {
      return ciType::make(ek->as_type_array_klass()->element_type());
    }
    return ek;
  }
}


// ------------------------------------------------------------------
// ciArrayKlass::base_element_type
//
// What type is obtained when this array is indexed as many times as possible?
ciArrayKlass* ciArrayKlass::make(ciType* element_type) {
  if (element_type->is_primitive_type()) {
    return ciTypeArrayKlass::make(element_type->basic_type());
  } else {
    return ciObjArrayKlass::make(element_type->as_klass());
  }
}
