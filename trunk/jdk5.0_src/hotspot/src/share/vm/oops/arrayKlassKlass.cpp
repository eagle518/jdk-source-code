#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)arrayKlassKlass.cpp	1.40 03/12/23 16:41:42 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_arrayKlassKlass.cpp.incl"


klassOop arrayKlassKlass::create_klass(TRAPS) {
  arrayKlassKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());
  KlassHandle k = base_create_klass(h_this_klass, header_size(), o.vtbl_value(), CHECK_0);
  // Make sure size calculation is right
  assert(k()->size() == align_object_size(header_size()), "wrong size for object");
  java_lang_Class::create_mirror(k, CHECK_0); // Allocate mirror
  return k();
}

bool arrayKlassKlass::oop_is_parsable(oop obj) const {
  assert (obj->is_klass(), "must be klass");
  arrayKlass* ak = arrayKlass::cast(klassOop(obj));
  return ak->object_is_parsable();
}

void arrayKlassKlass::oop_follow_contents(oop obj) {
  assert (obj->is_klass(), "must be klass");
  arrayKlass* ak = arrayKlass::cast(klassOop(obj));
  MarkSweep::mark_and_push(ak->adr_lower_dimension() );
  MarkSweep::mark_and_push(ak->adr_higher_dimension() );
  {
    HandleMark hm;
    ak->vtable()->oop_follow_contents();
  }
  klassKlass::oop_follow_contents(obj);
}


int arrayKlassKlass::oop_adjust_pointers(oop obj) {
  assert (obj->is_klass(), "must be klass");
  arrayKlass* ak = arrayKlass::cast(klassOop(obj));
  MarkSweep::adjust_pointer(ak->adr_lower_dimension() );
  MarkSweep::adjust_pointer(ak->adr_higher_dimension() );
  {
    HandleMark hm;
    ak->vtable()->oop_adjust_pointers();
  }
  return klassKlass::oop_adjust_pointers(obj);
}


int arrayKlassKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert (obj->is_klass(), "must be klass");
  arrayKlass* ak = arrayKlass::cast(klassOop(obj));
  blk->do_oop(ak->adr_lower_dimension());
  blk->do_oop(ak->adr_higher_dimension());
  ak->vtable()->oop_oop_iterate(blk);
  return klassKlass::oop_oop_iterate(obj, blk);
}


int arrayKlassKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert (obj->is_klass(), "must be klass");
  arrayKlass* ak = arrayKlass::cast(klassOop(obj));
  blk->do_oop(ak->adr_lower_dimension());
  blk->do_oop(ak->adr_higher_dimension());
  ak->vtable()->oop_oop_iterate_m(blk, mr);
  return klassKlass::oop_oop_iterate_m(obj, blk, mr);
}

void arrayKlassKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->blueprint()->oop_is_arrayKlass(),"must be an array klass");
}

#ifndef PRODUCT

// Printing

void arrayKlassKlass::oop_print_on(oop obj, outputStream* st) {
  assert(obj->is_klass(), "must be klass");
  klassKlass::oop_print_on(obj, st);
}


void arrayKlassKlass::oop_print_value_on(oop obj, outputStream* st) {
  assert(obj->is_klass(), "must be klass");
  arrayKlass* ak = arrayKlass::cast(klassOop(obj));
  for(int index = 0; index < ak->dimension(); index++) {
    st->print("[]");
  }
}


const char* arrayKlassKlass::internal_name() const {
  return "{array class}";
}

void arrayKlassKlass::oop_verify_on(oop obj, outputStream* st) {
  klassKlass::oop_verify_on(obj, st);

  arrayKlass* ak = arrayKlass::cast(klassOop(obj));
  if (!obj->partially_loaded()) {
    if (ak->lower_dimension())
      guarantee(ak->lower_dimension()->klass(), "should have a class");
    if (ak->higher_dimension())
      guarantee(ak->higher_dimension()->klass(), "should have a class");
  }
}

#endif
