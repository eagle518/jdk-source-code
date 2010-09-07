/*
 * Copyright (c) 1997, 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

# include "incls/_precompiled.incl"
# include "incls/_privilegedStack.cpp.incl"


void PrivilegedElement::initialize(vframeStream* vfst, oop context, PrivilegedElement* next, TRAPS) {
  methodOop method      = vfst->method();
  _klass                = method->method_holder();
  _privileged_context   = context;
#ifdef CHECK_UNHANDLED_OOPS
  THREAD->allow_unhandled_oop(&_klass);
  THREAD->allow_unhandled_oop(&_privileged_context);
#endif // CHECK_UNHANDLED_OOPS
  _frame_id             = vfst->frame_id();
  _next                 = next;
  assert(_privileged_context == NULL || _privileged_context->is_oop(), "must be an oop");
  assert(protection_domain() == NULL || protection_domain()->is_oop(), "must be an oop");
}

void PrivilegedElement::oops_do(OopClosure* f) {
  PrivilegedElement *cur = this;
  do {
    f->do_oop((oop*) &cur->_klass);
    f->do_oop((oop*) &cur->_privileged_context);
    cur = cur->_next;
  } while(cur != NULL);
}

//-------------------------------------------------------------------------------
#ifndef PRODUCT

void PrivilegedElement::print_on(outputStream* st) const {
  st->print("   0x%lx ", _frame_id);
  _klass->print_value_on(st);
  if (protection_domain() != NULL) {
    st->print("   ");
    protection_domain()->print_value_on(st);
  }
  st->cr();
}

bool PrivilegedElement::contains(address addr) {
  PrivilegedElement *e = (PrivilegedElement *)addr;
  if (e >= this && e < this+1) return true;

  if (_next != NULL) {
    return _next->contains(addr);
  } else {
    return false;
  }
}

#endif
