#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)klassKlass.cpp	1.58 03/12/23 16:41:55 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_klassKlass.cpp.incl"

int klassKlass::oop_size(oop obj) const {
  assert (obj->is_klass(), "must be a klassOop");
  return klassOop(obj)->klass_part()->klass_oop_size();
}

klassOop klassKlass::create_klass(TRAPS) {
  KlassHandle h_this_klass;
  klassKlass o;
  // for bootstrapping, handles may not be available yet.
  klassOop k = base_create_klass_oop(h_this_klass, header_size(), o.vtbl_value(), CHECK_0);
  k->set_klass(k); // point to thyself
  // Do not try to allocate mirror, java.lang.Class not loaded at this point.
  // See Universe::fixup_mirrors()
  return k;
}

void klassKlass::oop_follow_contents(oop obj) {
  Klass* k = Klass::cast(klassOop(obj));
  // If we are alive it is valid to keep our superclass and subtype caches alive
  MarkSweep::mark_and_push(k->adr_super());
  for (juint i = 0; i < Klass::primary_super_limit(); i++)
    MarkSweep::mark_and_push(k->adr_primary_supers()+i);
  MarkSweep::mark_and_push(k->adr_secondary_super_cache());
  MarkSweep::mark_and_push(k->adr_secondary_supers());
  MarkSweep::mark_and_push(k->adr_java_mirror());
  MarkSweep::mark_and_push(k->adr_name());
  // We follow the subklass and sibling links at the end of the 
  // marking phase, since otherwise following them will prevent
  // class unloading (all classes are transitively linked from
  // java.lang.Object).
  MarkSweep::revisit_weak_klass_link(k);
  obj->follow_header();
}


int klassKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  // Get size before changing pointers
  int size = oop_size(obj);
  Klass* k = Klass::cast(klassOop(obj));
  blk->do_oop(k->adr_super());
  for (juint i = 0; i < Klass::primary_super_limit(); i++)
    blk->do_oop(k->adr_primary_supers()+i);
  blk->do_oop(k->adr_secondary_super_cache());
  blk->do_oop(k->adr_secondary_supers());
  blk->do_oop(k->adr_java_mirror());
  blk->do_oop(k->adr_name());
  // The following are in the perm gen and are treated
  // specially in a later phase of a perm gen collection; ...
  assert(oop(k)->is_perm(), "should be in perm");
  assert(oop(k->adr_subklass())->is_perm(), "should be in perm");
  assert(oop(k->adr_next_sibling())->is_perm(), "should be in perm");
  // ... don't scan them normally, but remember this klassKlass
  // for later (see, for instance, oop_follow_contents above
  // for what MarkSweep does with it.
  if (blk->should_remember_klasses()) {
    blk->remember_klass(k);
  }
  obj->oop_iterate_header(blk);
  return size;
}


int klassKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  // Get size before changing pointers
  int size = oop_size(obj);
  Klass* k = Klass::cast(klassOop(obj));
  oop* adr;
  adr = k->adr_super();
  if (mr.contains(adr)) blk->do_oop(adr);
  for (juint i = 0; i < Klass::primary_super_limit(); i++) {
    adr = k->adr_primary_supers()+i;
    if (mr.contains(adr)) blk->do_oop(adr);
  }
  adr = k->adr_secondary_super_cache();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = k->adr_secondary_supers();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = k->adr_java_mirror();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = k->adr_name();
  if (mr.contains(adr)) blk->do_oop(adr);
  // The following are "weak links" in the perm gen and are
  // treated specially in a later phase of a perm gen collection.
  assert(oop(k)->is_perm(), "should be in perm");
  assert(oop(k->adr_subklass())->is_perm(), "should be in perm");
  assert(oop(k->adr_next_sibling())->is_perm(), "should be in perm");
  if (blk->should_remember_klasses()
      && (mr.contains(k->adr_subklass())
          || mr.contains(k->adr_next_sibling()))) {
    blk->remember_klass(k);
  }
  obj->oop_iterate_header(blk, mr);
  return size;
}


int klassKlass::oop_adjust_pointers(oop obj) {
  // Get size before changing pointers
  int size = oop_size(obj);
  obj->adjust_header();

  Klass* k = Klass::cast(klassOop(obj));

  MarkSweep::adjust_pointer(k->adr_super());
  for (juint i = 0; i < Klass::primary_super_limit(); i++)
    MarkSweep::adjust_pointer(k->adr_primary_supers()+i);
  MarkSweep::adjust_pointer(k->adr_secondary_super_cache());
  MarkSweep::adjust_pointer(k->adr_secondary_supers());
  MarkSweep::adjust_pointer(k->adr_java_mirror());
  MarkSweep::adjust_pointer(k->adr_name());
  MarkSweep::adjust_pointer(k->adr_subklass());
  MarkSweep::adjust_pointer(k->adr_next_sibling());
  return size;
}

void klassKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
}

#ifndef PRODUCT

// Printing

void klassKlass::oop_print_on(oop obj, outputStream* st) {
  Klass::oop_print_on(obj, st);
}


void klassKlass::oop_print_value_on(oop obj, outputStream* st) {
  Klass::oop_print_value_on(obj, st);
}


const char* klassKlass::internal_name() const {
  return "{other class}";
}


// Verification

void klassKlass::oop_verify_on(oop obj, outputStream* st) {
  Klass::oop_verify_on(obj, st);
  guarantee(obj->is_perm(),                      "should be in permspace");
  guarantee(obj->is_klass(),                     "should be klass");

  Klass* k = Klass::cast(klassOop(obj));
  if (k->super() != NULL) {
    guarantee(k->super()->is_perm(),             "should be in permspace");
    guarantee(k->super()->is_klass(),            "should be klass");
  }
  klassOop ko = k->secondary_super_cache();
  if( ko != NULL ) {
    guarantee(ko->is_perm(),                     "should be in permspace");
    guarantee(ko->is_klass(),                    "should be klass");
  }
  for( uint i = 0; i < primary_super_limit(); i++ ) {
    oop ko = k->adr_primary_supers()[i]; // Cannot use normal accessor because it asserts
    if( ko != NULL ) {
      guarantee(ko->is_perm(),                   "should be in permspace");
      guarantee(ko->is_klass(),                  "should be klass");
    }
  }

  if (k->java_mirror() != NULL || (k->oop_is_instance() && instanceKlass::cast(klassOop(obj))->is_loaded())) {
    guarantee(k->java_mirror() != NULL,          "should be allocated");
    guarantee(k->java_mirror()->is_perm(),       "should be in permspace");
    guarantee(k->java_mirror()->is_instance(),   "should be instance");
  }
  if (k->name() != NULL) {
    guarantee(Universe::heap()->is_in_permanent(k->name()),
	      "should be in permspace");
    guarantee(k->name()->is_symbol(), "should be symbol");
  }
}

#endif
