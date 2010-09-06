#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)typeArrayKlassKlass.cpp	1.21 03/12/23 16:42:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_typeArrayKlassKlass.cpp.incl"

klassOop typeArrayKlassKlass::create_klass(TRAPS) {
  typeArrayKlassKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());  
  KlassHandle k = base_create_klass(h_this_klass, header_size(), o.vtbl_value(), CHECK_0);
  assert(k()->size() == align_object_size(header_size()), "wrong size for object");
  java_lang_Class::create_mirror(k, CHECK_0); // Allocate mirror
  return k();
}


#ifndef PRODUCT

// Printing

void typeArrayKlassKlass::oop_print_on(oop obj, outputStream* st) {
  assert(obj->is_klass(), "must be klass");
  oop_print_value_on(obj, st);
  Klass:: oop_print_on(obj, st); 
}


void typeArrayKlassKlass::oop_print_value_on(oop obj, outputStream* st) {
  assert(obj->is_klass(), "must be klass");
  st->print("{type array ");
  switch (typeArrayKlass::cast(klassOop(obj))->type()) {
    case T_BOOLEAN: st->print("bool");    break;
    case T_CHAR:    st->print("char");    break;
    case T_FLOAT:   st->print("float");   break;
    case T_DOUBLE:  st->print("double");  break;
    case T_BYTE:    st->print("byte");    break;
    case T_SHORT:   st->print("short");   break;
    case T_INT:     st->print("int");     break;
    case T_LONG:    st->print("long");    break;
    default: ShouldNotReachHere();
  }
  st->print("}");
}


const char* typeArrayKlassKlass::internal_name() const {
  return "{type array class}";
}


#endif
