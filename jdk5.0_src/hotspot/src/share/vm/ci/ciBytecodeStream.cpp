#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)ciBytecodeStream.cpp	1.12 03/12/23 16:39:24 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_ciBytecodeStream.cpp.incl"


ciBytecodeStream::ciBytecodeStream(ciMethod* method) {
  _method    = method;
  _code_base = method->code();
  _code_size = method->code_size();
  set_interval(0, _code_size);
}


void ciBytecodeStream::set_interval(int beg_bci, int end_bci) {
  assert(0 <= beg_bci && beg_bci <= _code_size, "illegal beg_bci");
  assert(0 <= end_bci && end_bci <= _code_size, "illegal end_bci");
  // setup of iteration pointers
  _bci      = beg_bci;
  _next_bci = beg_bci;
  _end_bci  = end_bci;
}


int ciBytecodeStream::get_constant_index() const {
  switch(code()) {
    case Bytecodes::_ldc   : return get_index();
    case Bytecodes::_ldc_w : // fall through
    case Bytecodes::_ldc2_w: return get_index_big();
  }
  ShouldNotReachHere();
  return 0;
}


int ciBytecodeStream::get_field_index() const {
  return get_index_big();
}


int ciBytecodeStream::get_klass_index() const {
  return get_index_big();
}


int ciBytecodeStream::get_method_index() const {
  return get_index_big();
}


ciConstant ciBytecodeStream::get_constant() const {
  return CURRENT_ENV->get_constant_by_index(method()->holder(), get_constant_index());
}


ciField* ciBytecodeStream::get_field() const {
  return CURRENT_ENV->get_field_by_index(method()->holder(), get_field_index());
}


ciKlass* ciBytecodeStream::get_klass() const {
  bool will_link;
  ciKlass* k = CURRENT_ENV->get_klass_by_index(method()->holder(), get_klass_index(), will_link);
  if (!will_link && k->is_loaded()) {
    GUARDED_VM_ENTRY(
      k = CURRENT_ENV->get_unloaded_klass(method()->holder(), k->name());
    )
  }
  return k;
}


ciMethod* ciBytecodeStream::get_method() const {
  return CURRENT_ENV->get_method_by_index(method()->holder(), get_method_index(), code());
}


ciKlass* ciBytecodeStream::get_declared_method_holder() {
  bool ignore;
  // There is no "will_link" result passed back.  The user is responsible
  // for checking linkability when retrieving the associated method.
  return CURRENT_ENV->get_klass_by_index(method()->holder(), get_method_holder_index(), ignore);
}


int ciBytecodeStream::get_method_holder_index() {
  VM_ENTRY_MARK;
  constantPoolOop cpool = method()->holder()->get_instanceKlass()->constants();
  return cpool->klass_ref_index_at(get_method_index());
}

