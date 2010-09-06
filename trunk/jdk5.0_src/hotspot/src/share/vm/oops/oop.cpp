#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)oop.cpp	1.88 03/12/23 16:42:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_oop.cpp.incl"

bool always_do_update_barrier = false;

BarrierSet* oopDesc::_bs = NULL;

#ifdef PRODUCT
void oopDesc::print_on(outputStream* st) const {}
void oopDesc::print_value_on(outputStream* st) const {}
void oopDesc::print_address_on(outputStream* st) const {}
char* oopDesc::print_value_string() { return NULL; }
char* oopDesc::print_string() { return NULL; }
void oopDesc::print()         {}
void oopDesc::print_value()   {}
void oopDesc::print_address() {}
void oopDesc::verify() {}
void oopDesc::verify_old_oop(oop* p, bool allow_dirty) {}
#else
void oopDesc::print_on(outputStream* st) const {
  if (this == NULL) {
    st->print_cr("NULL");
  } else {
    blueprint()->oop_print_on(oop(this), st);
  }
}

void oopDesc::print_value_on(outputStream* st) const {
  oop obj = oop(this);
  if (this == NULL) {
    st->print("NULL");
  } else if (java_lang_String::is_instance(obj)) {
    java_lang_String::print(obj, st);
    if (PrintOopAddress) print_address_on(st);
#ifdef ASSERT
  } else if (!Universe::heap()->is_in(obj) || !Universe::heap()->is_in(klass())) {
    st->print("### BAD OOP %p ###", obj);
#endif
  } else {
    blueprint()->oop_print_value_on(obj, st);
  }
}

void oopDesc::print_address_on(outputStream* st) const {
  if (PrintOopAddress) {
    st->print("{");
    if (PrintOopAddress) {
      st->print(INTPTR_FORMAT, this);
    }
    st->print("}");
  }
}

void oopDesc::print()         { print_on(tty);         }

void oopDesc::print_value()   { print_value_on(tty);   }

void oopDesc::print_address() { print_address_on(tty); }

char* oopDesc::print_string() { 
  stringStream* st = new stringStream();
  print_on(st);
  return st->as_string();
}

char* oopDesc::print_value_string() { 
  stringStream* st = new stringStream();
  print_value_on(st);
  return st->as_string();
}


void oopDesc::verify_on(outputStream* st) {
  if (this != NULL) {
    blueprint()->oop_verify_on(this, st);
  }
}


void oopDesc::verify() { 
  verify_on(tty); 
}


void oopDesc::verify_old_oop(oop* p, bool allow_dirty) {
  blueprint()->oop_verify_old_oop(this, p, allow_dirty);
}


bool oopDesc::partially_loaded() {
  return blueprint()->oop_partially_loaded(this);
}


void oopDesc::set_partially_loaded() {
  blueprint()->oop_set_partially_loaded(this);
}
#endif // PRODUCT


intptr_t oopDesc::slow_identity_hash() {
  // slow case; we have to acquire the micro lock in order to locate the header  
  ResetNoHandleMark rnm; // Might be called from LEAF/QUICK ENTRY
  HandleMark hm;
  Handle object(this);
  assert(!is_shared_readonly(), "using identity hash on readonly object?");
  return ObjectSynchronizer::identity_hash_value_for(object);
}

NOT_PRODUCT(VerifyOopClosure VerifyOopClosure::verify_oop;)

oop oopDesc::forward_to_atomic(oop p) {
  assert(ParNewGeneration::is_legal_forward_ptr(p),
	 "illegal forwarding pointer value.");
  markOop oldMark = mark();
  markOop forwardPtrMark = markOopDesc::encode_pointer_as_mark(p);
  markOop curMark;

  assert(forwardPtrMark->decode_pointer() == p, "encoding must be reversable");
  assert(sizeof(markOop) == sizeof(intptr_t), "CAS below requires this.");

  while (!is_forwarded()) {
    curMark = (markOop)Atomic::cmpxchg_ptr(forwardPtrMark, &_mark, oldMark);
    if (curMark == oldMark) {
      assert(is_forwarded(), "the CAS should have succeeded.");
      return NULL;
    }
    oldMark = curMark;
  }
  return forwardee();
}
