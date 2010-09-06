#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciInstance.cpp	1.8 03/12/23 16:39:30 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciInstance.cpp.incl"

// ciInstance
//
// This class represents an instanceOop in the HotSpot virtual
// machine.

// ------------------------------------------------------------------
// ciInstance::print_impl
//
// ------------------------------------------------------------------
// ciObject::java_mirror_type
ciType* ciInstance::java_mirror_type() {
  VM_ENTRY_MARK;
  oop m = get_oop();
  // Return NULL if it is not java.lang.Class.
  if (m == NULL || m->klass() != SystemDictionary::class_klass()) {
    return NULL;
  }
  // Return either a primitive type or a klass.
  if (java_lang_Class::is_primitive(m)) {
    return ciType::make(java_lang_Class::primitive_type(m));
  } else {
    klassOop k = java_lang_Class::as_klassOop(m);
    assert(k != NULL, "");
    return CURRENT_THREAD_ENV->get_object(k)->as_klass();
  }
}

// Implementation of the print method.
void ciInstance::print_impl() {
  tty->print(" type=");
  klass()->print();
}
