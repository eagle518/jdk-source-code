#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)methodDataKlass.cpp	1.23 03/12/23 16:41:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_methodDataKlass.cpp.incl"

klassOop methodDataKlass::create_klass(TRAPS) {
  methodDataKlass o;
  KlassHandle h_this_klass(THREAD, Universe::klassKlassObj());  
  KlassHandle k = base_create_klass(h_this_klass, header_size(),
				    o.vtbl_value(), CHECK_0);
  // Make sure size calculation is right
  assert(k()->size() == align_object_size(header_size()),
	 "wrong size for object");
  return k();
}


int methodDataKlass::oop_size(oop obj) const {
  assert(obj->is_methodData(), "must be method data oop");
  return methodDataOop(obj)->object_size();
}


bool methodDataKlass::oop_is_parsable(oop obj) const {
  assert(obj->is_methodData(), "must be method data oop");
  return methodDataOop(obj)->object_is_parsable();
}


methodDataOop methodDataKlass::allocate(methodHandle method, TRAPS) {
  int size = methodDataOopDesc::compute_allocation_size_in_words(method());
  KlassHandle h_k(THREAD, as_klassOop());
  methodDataOop mdo =
    (methodDataOop)CollectedHeap::permanent_obj_allocate(h_k, size, CHECK_0);
  assert(!mdo->is_parsable(), "not expecting parsability yet.");
  No_GC_Verifier nogc;          // the init function should not GC
  mdo->initialize(method());

  assert(mdo->is_parsable(), "should be parsable here.");
  assert(size == mdo->object_size(), "wrong size for methodDataOop");
  return mdo;
}


void methodDataKlass::oop_follow_contents(oop obj) {
  assert (obj->is_methodData(), "object must be method data");
  methodDataOop m = methodDataOop(obj);

  obj->follow_header();
  MarkSweep::mark_and_push(m->adr_method());
  ResourceMark rm;
  for (ProfileData* data = m->first_data(); 
       m->is_valid(data); 
       data = m->next_data(data)) {
    data->follow_contents();
  }
}

int methodDataKlass::oop_oop_iterate(oop obj, OopClosure* blk) {
  assert (obj->is_methodData(), "object must be method data");
  methodDataOop m = methodDataOop(obj);
  // Get size before changing pointers
  // Don't call size() or oop_size() since that is a virtual call.
  int size = m->object_size();

  obj->oop_iterate_header(blk);
  blk->do_oop(m->adr_method());
  ResourceMark rm;
  for (ProfileData* data = m->first_data(); 
       m->is_valid(data);
       data = m->next_data(data)) {
    data->oop_iterate(blk);
  }
  return size;
}


int methodDataKlass::oop_oop_iterate_m(oop obj, OopClosure* blk, MemRegion mr) {
  assert (obj->is_methodData(), "object must be method data");
  methodDataOop m = methodDataOop(obj);
  // Get size before changing pointers
  // Don't call size() or oop_size() since that is a virtual call.
  int size = m->object_size();

  obj->oop_iterate_header(blk, mr);
  oop* adr = m->adr_method();
  if (mr.contains(adr)) {
    blk->do_oop(m->adr_method());
  }
  ResourceMark rm;
  for (ProfileData* data = m->first_data(); 
       m->is_valid(data);
       data = m->next_data(data)) {
    data->oop_iterate_m(blk, mr);
  }
  return size;
}

int methodDataKlass::oop_adjust_pointers(oop obj) {
  assert(obj->is_methodData(), "should be method data");
  methodDataOop m = methodDataOop(obj);
  // Get size before changing pointers
  // Don't call size() or oop_size() since that is a virtual call.
  int size = m->object_size();

  obj->adjust_header();
  MarkSweep::adjust_pointer(m->adr_method());
  ResourceMark rm;
  for (ProfileData* data = m->first_data();
       m->is_valid(data);
       data = m->next_data(data)) {
    data->adjust_pointers();
  }
  return size;
}


void methodDataKlass::oop_copy_contents(PSPromotionManager* pm, oop obj) {
  assert (obj->is_methodData(), "object must be method data");
  methodDataOop m = methodDataOop(obj);  
  // This should never point into the young gen.
  assert(!PSScavenge::should_scavenge(oop(*m->adr_method())), "Sanity");
}


#ifndef PRODUCT

// Printing
void methodDataKlass::oop_print_on(oop obj, outputStream* st) {
  assert(obj->is_methodData(), "should be method data");
  methodDataOop m = methodDataOop(obj);
  st->print("method data for ");
  m->method()->print_value_on(st);
  st->cr();
  m->print_data_on(st);
}

void methodDataKlass::oop_print_value_on(oop obj, outputStream* st) {
  assert(obj->is_methodData(), "should be method data");
  methodDataOop m = methodDataOop(obj);
  st->print("method data for ");
  m->method()->print_value_on(st);
}

const char* methodDataKlass::internal_name() const {
  return "{method data}";
}


// Verification
void methodDataKlass::oop_verify_on(oop obj, outputStream* st) {
  Klass::oop_verify_on(obj, st);
  guarantee(obj->is_methodData(), "object must be method data");
  methodDataOop m = methodDataOop(obj);
  guarantee(m->is_perm(), "should be in permspace");
  m->verify_data_on(st);
}

#endif // !PRODUCT
