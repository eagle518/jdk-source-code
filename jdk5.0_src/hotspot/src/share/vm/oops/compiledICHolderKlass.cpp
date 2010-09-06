#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)compiledICHolderKlass.cpp	1.29 03/12/23 16:41:43 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_compiledICHolderKlass.cpp.incl"

klassOop compiledICHolderKlass::create_klass(TRAPS) {
  compiledICHolderKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());  
  KlassHandle k = base_create_klass(h_this_klass, header_size(), o.vtbl_value(), CHECK_0);
  // Make sure size calculation is right
  assert(k()->size() == align_object_size(header_size()), "wrong size for object");
  java_lang_Class::create_mirror(k, CHECK_0); // Allocate mirror
  return k();
}


compiledICHolderOop compiledICHolderKlass::allocate(TRAPS) {
  KlassHandle h_k(THREAD, as_klassOop());
  int size = compiledICHolderOopDesc::object_size();
  compiledICHolderOop c = (compiledICHolderOop)
    CollectedHeap::permanent_obj_allocate(h_k, size, CHECK_0);
  c->set_holder_method(NULL);
  c->set_holder_klass(NULL);
  return c;
}


int compiledICHolderKlass::oop_size(oop obj) const {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  return compiledICHolderOop(obj)->object_size();
}

void compiledICHolderKlass::oop_follow_contents(oop obj) {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  compiledICHolderOop c = compiledICHolderOop(obj);

  obj->follow_header();
  MarkSweep::mark_and_push(c->adr_holder_method());
  MarkSweep::mark_and_push(c->adr_holder_klass());
}


int compiledICHolderKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  compiledICHolderOop c = compiledICHolderOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = c->object_size();

  obj->oop_iterate_header(blk);
  blk->do_oop(c->adr_holder_method());
  blk->do_oop(c->adr_holder_klass());
  return size;
}

int compiledICHolderKlass::oop_oop_iterate_m(oop obj, OopClosure* blk,
					      MemRegion mr) {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  compiledICHolderOop c = compiledICHolderOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = c->object_size();

  obj->oop_iterate_header(blk, mr);

  oop* adr;
  adr = c->adr_holder_method();
  if (mr.contains(adr)) blk->do_oop(adr);
  adr = c->adr_holder_klass();
  if (mr.contains(adr)) blk->do_oop(adr);
  return size;
}


int compiledICHolderKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  compiledICHolderOop c = compiledICHolderOop(obj);
  // Get size before changing pointers.
  // Don't call size() or oop_size() since that is a virtual call.
  int size = c->object_size();

  MarkSweep::adjust_pointer(c->adr_holder_method());
  MarkSweep::adjust_pointer(c->adr_holder_klass());
  obj->adjust_header();
  return size;
}

void compiledICHolderKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
}

bool compiledICHolderKlass::oop_being_unloaded(
  BoolObjectClosure* is_alive, oop obj) {
  // Delegate to the method and klass
  assert(!is_alive->do_object_b(obj), "should not be live");
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  compiledICHolderOop c = compiledICHolderOop(obj);
  return c->holder_method()->method_holder()->being_unloaded(is_alive)
         || c->holder_klass()->being_unloaded(is_alive);
}

#ifndef PRODUCT

// Printing

void compiledICHolderKlass::oop_print_on(oop obj, outputStream* st) {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  Klass::oop_print_on(obj, st);
  compiledICHolderOop c = compiledICHolderOop(obj);
  st->print(" - method: "); c->holder_method()->print_value_on(st); st->cr();
  st->print(" - klass:  "); c->holder_klass()->print_value_on(st); st->cr();
}


void compiledICHolderKlass::oop_print_value_on(oop obj, outputStream* st) {
  assert(obj->is_compiledICHolder(), "must be compiledICHolder");
  Klass::oop_print_value_on(obj, st);
}

const char* compiledICHolderKlass::internal_name() const {
  return "{compiledICHolder}";
}

// Verification

void compiledICHolderKlass::oop_verify_on(oop obj, outputStream* st) {
  Klass::oop_verify_on(obj, st);
  guarantee(obj->is_compiledICHolder(), "must be compiledICHolder");
  compiledICHolderOop c = compiledICHolderOop(obj);
  guarantee(c->is_perm(),             "should be in permspace");
  guarantee(c->holder_method()->is_perm(),   "should be in permspace");
  guarantee(c->holder_method()->is_method(), "should be method");
  guarantee(c->holder_klass()->is_perm(),    "should be in permspace");
  guarantee(c->holder_klass()->is_klass(),   "should be klass");
}
#endif
