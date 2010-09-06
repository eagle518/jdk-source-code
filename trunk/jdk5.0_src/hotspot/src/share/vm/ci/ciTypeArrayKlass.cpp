#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciTypeArrayKlass.cpp	1.7 03/12/23 16:39:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciTypeArrayKlass.cpp.incl"

// ciTypeArrayKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part in a TypeArrayKlass.

// ------------------------------------------------------------------
// ciTypeArrayKlass::ciTypeArrayKlass
ciTypeArrayKlass::ciTypeArrayKlass(KlassHandle h_k) : ciArrayKlass(h_k) {
  assert(get_Klass()->oop_is_typeArray(), "wrong type");
  _element_type = get_typeArrayKlass()->type();
}

// ------------------------------------------------------------------
// ciTypeArrayKlass::make_impl
//
// Implementation of make.
ciTypeArrayKlass* ciTypeArrayKlass::make_impl(BasicType t) {
  klassOop k = Universe::typeArrayKlassObj(t);
  return CURRENT_ENV->get_object(k)->as_type_array_klass();
}

// ------------------------------------------------------------------
// ciTypeArrayKlass::make
//
// Make an array klass corresponding to the specified primitive type.
ciTypeArrayKlass* ciTypeArrayKlass::make(BasicType t) {
  GUARDED_VM_ENTRY(return make_impl(t);)
}

