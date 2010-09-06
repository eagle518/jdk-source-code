#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciObjArrayKlass.cpp	1.15 03/12/23 16:39:36 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_ciObjArrayKlass.cpp.incl"

// ciObjArrayKlass
//
// This class represents a klassOop in the HotSpot virtual machine
// whose Klass part is an objArrayKlass.

// ------------------------------------------------------------------
// ciObjArrayKlass::ciObjArrayKlass
//
// Constructor for loaded object array klasses.
ciObjArrayKlass::ciObjArrayKlass(KlassHandle h_k) : ciArrayKlass(h_k) {
  assert(get_Klass()->oop_is_objArray(), "wrong type");
  klassOop element_klassOop = get_objArrayKlass()->bottom_klass();
  _base_element_klass = CURRENT_ENV->get_object(element_klassOop)->as_klass();
  assert(_base_element_klass->is_instance_klass() ||
         _base_element_klass->is_type_array_klass(), "bad base klass");
  if (dimension() == 1) {
    _element_klass = _base_element_klass;
  } else {
    _element_klass = NULL;
  }
  if (!ciObjectFactory::shared_is_initialized()) {
    assert(_element_klass->is_java_lang_Object(), "only arrays of object are shared");
  }
}

// ------------------------------------------------------------------
// ciObjArrayKlass::ciObjArrayKlass
//
// Constructor for unloaded object array klasses.
ciObjArrayKlass::ciObjArrayKlass(ciSymbol* array_name,
                                 ciKlass* base_element_klass,
                                 int dimension)
  : ciArrayKlass(array_name,
                 dimension,
                 ciObjArrayKlassKlass::make()) {
    _base_element_klass = base_element_klass;
    assert(_base_element_klass->is_instance_klass() ||
           _base_element_klass->is_type_array_klass(), "bad base klass");
    if (dimension == 1) {
      _element_klass = base_element_klass;
    } else {
      _element_klass = NULL;
    }
}

// ------------------------------------------------------------------
// ciObjArrayKlass::element_klass
//
// What is the one-level element type of this array?
ciKlass* ciObjArrayKlass::element_klass() {
  if (_element_klass == NULL) {
    assert(dimension() > 1, "_element_klass should not be NULL");
    // Produce the element klass.
    if (is_loaded()) {
      VM_ENTRY_MARK;
      klassOop element_klassOop = get_objArrayKlass()->element_klass();
      _element_klass = CURRENT_THREAD_ENV->get_object(element_klassOop)->as_klass();
    } else {
      VM_ENTRY_MARK;
      // We are an unloaded array klass.  Attempt to fetch our
      // element klass by name.
      _element_klass = CURRENT_THREAD_ENV->get_klass_by_name_impl(
                          this,
                          construct_array_name(base_element_klass()->name(),
                                               dimension() - 1),
                          false);
    }
  }
  return _element_klass;
}

// ------------------------------------------------------------------
// ciObjArrayKlass::construct_array_name
//
// Build an array name from an element name and a dimension.
ciSymbol* ciObjArrayKlass::construct_array_name(ciSymbol* element_name,
                                                int dimension) {
  EXCEPTION_CONTEXT;
  int element_len = element_name->utf8_length();

  symbolOop base_name_sym = element_name->get_symbolOop();
  char* name;

  if (base_name_sym->byte_at(0) == '[' ||
      (base_name_sym->byte_at(0) == 'L' &&  // watch package name 'Lxx'
       base_name_sym->byte_at(element_len-1) == ';')) {

    int new_len = element_len + dimension + 1; // for the ['s and '\0'
    name = CURRENT_THREAD_ENV->name_buffer(new_len);

    int pos = 0;
    for ( ; pos < dimension; pos++) {
      name[pos] = '[';
    }
    strncpy(name+pos, (char*)element_name->base(), element_len);
    name[new_len-1] = '\0';
  } else {
    int new_len =   3                       // for L, ;, and '\0'
                  + dimension               // for ['s
                  + element_len;

    name = CURRENT_THREAD_ENV->name_buffer(new_len);
    int pos = 0;
    for ( ; pos < dimension; pos++) {
      name[pos] = '[';
    }
    name[pos++] = 'L';
    strncpy(name+pos, (char*)element_name->base(), element_len);
    name[new_len-2] = ';';
    name[new_len-1] = '\0';
  }
  return ciSymbol::make(name);
}

// ------------------------------------------------------------------
// ciObjArrayKlass::make_impl
//
// Implementation of make.
ciObjArrayKlass* ciObjArrayKlass::make_impl(ciKlass* element_klass) {

  if (element_klass->is_loaded()) {
    EXCEPTION_CONTEXT;
    // The element klass is loaded
    klassOop array = element_klass->get_Klass()->array_klass(THREAD);
#ifndef PRODUCT
    CURRENT_THREAD_ENV->maybe_fire_out_of_memory(THREAD);
#endif
    if (HAS_PENDING_EXCEPTION) {
      CLEAR_PENDING_EXCEPTION;
      CURRENT_THREAD_ENV->record_out_of_memory_failure();
      return ciEnv::unloaded_ciobjarrayklass();
    }
    return CURRENT_THREAD_ENV->get_object(array)->as_obj_array_klass();
  }

  // The array klass was unable to be made or the element klass was
  // not loaded.
  ciSymbol* array_name = construct_array_name(element_klass->name(), 1);
  if (array_name == ciEnv::unloaded_cisymbol()) {
    return ciEnv::unloaded_ciobjarrayklass();
  }
  return
    CURRENT_ENV->get_unloaded_klass(element_klass, array_name)
                        ->as_obj_array_klass();
}

// ------------------------------------------------------------------
// ciObjArrayKlass::make
//
// Make an array klass corresponding to the specified primitive type.
ciObjArrayKlass* ciObjArrayKlass::make(ciKlass* element_klass) {
  assert(element_klass->is_java_klass(), "wrong kind of klass");
  GUARDED_VM_ENTRY(return make_impl(element_klass);)
}
