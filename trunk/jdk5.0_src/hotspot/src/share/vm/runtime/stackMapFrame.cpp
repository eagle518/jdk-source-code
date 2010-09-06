#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)stackMapFrame.cpp	1.9 03/12/23 16:44:10 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_stackMapFrame.cpp.incl"

StackMapFrame::StackMapFrame(u2 max_locals, u2 max_stack, ClassVerifier* v) : 
                      _offset(0), _locals_size(0), _stack_size(0), _flags(0), 
                      _max_locals(max_locals), _max_stack(max_stack), _verifier(v) {
  _locals = NEW_C_HEAP_ARRAY(VerificationType*, max_locals);
  _stack = NEW_C_HEAP_ARRAY(VerificationType*, max_stack);
  u2 i;
  for(i = 0; i < max_locals; i++) {
    _locals[i] = VerificationType::_bogus_type;
  }
  for(i = 0; i < max_stack; i++) {
    _stack[i] = VerificationType::_bogus_type;
  }  
}

StackMapFrame::StackMapFrame(StackMapFrame* f) {
  _offset = f->offset();
  _verifier = f->verifier();
  _flags = f->flags();
  _max_locals = f->max_locals();
  _max_stack = f->max_stack();
  _locals_size = f->locals_size();
  if (_locals_size > 0) {
    _locals = NEW_C_HEAP_ARRAY(VerificationType*, _locals_size);
    copy_locals(f);
  } else {
    _locals = NULL;
  }
  _stack_size = f->stack_size();
  if (_stack_size > 0) {
    _stack = NEW_C_HEAP_ARRAY(VerificationType*, _stack_size);
    copy_stack(f);
  } else {
    _stack = NULL;
  }
}

StackMapFrame::~StackMapFrame() {
  // Don't deallocate array elements.
  // The types may be used by other frames or the next method.
  if (_locals != NULL) {
    FREE_C_HEAP_ARRAY(VerificationType*, _locals);
  }
  if (_stack != NULL) {
    FREE_C_HEAP_ARRAY(VerificationType*, _stack);
  }
}

StackMapFrame* StackMapFrame::frame_in_exception_handler(u1 flags) {
  // array size is set to _max_locals because it's part of type state
  VerificationType** locals = NEW_C_HEAP_ARRAY(VerificationType*, _max_locals);
  for (u2 i = 0; i < _max_locals; i++) {
    locals[i] = _locals[i];
  }
  VerificationType** stack = NEW_C_HEAP_ARRAY(VerificationType*, 1);
  return new StackMapFrame(_offset, flags, _locals_size, 0, _max_locals, _max_stack, locals, stack);
}

bool StackMapFrame::has_new_object() const {
  u2 i;
  for (i = 0; i < _max_locals; i++) {
    if (_locals[i]->is_uninitialized()) {
      return true;
    }
  }
  for (i = 0; i < _stack_size; i++) {
    if (_stack[i]->is_uninitialized()) {
      return true;
    }
  }
  return false;
}

void StackMapFrame::initialize_object(UninitializedType* old_object, ObjType* new_object) {
  u2 i;
  for (i = 0; i < _max_locals; i++) {
    if (_locals[i]->equals(old_object)) {
      _locals[i] = new_object;
    }
  }
  for (i = 0; i < _stack_size; i++) {
    if (_stack[i]->equals(old_object)) {
      _stack[i] = new_object;
    }
  }
  if (old_object != VerificationType::_uninitialized_this) {
    delete old_object;  // no longer exists in this method
  } else {
    // "this" has been initialized - reset flags
    _flags = 0;
  } 
}

VerificationType* StackMapFrame::set_locals_from_arg(const methodHandle m, ObjType* klass_type, TRAPS) {
  symbolHandle signature(THREAD, m->signature());
  SignatureStream ss(signature);
  int init_local_num = 0;
  if (!m->is_static()) {
    init_local_num++;
    // add one extra argument for instance method
    if (m->name() == vmSymbols::object_initializer_name() &&
        klass_type->name() != vmSymbols::java_lang_Object()) {
      _locals[0] = UninitializedType::_uninitialized_this;
      _flags |= FLAG_THIS_UNINIT;
    } else {
      _locals[0] = klass_type;
    }
  } 
  
  // local num may be greater than size of parameters because long/double occupies two slots
  while(!ss.at_return_type()) {
    init_local_num += VerificationType::change_sig_to_verificationType(&ss, &_locals[init_local_num], _verifier, CHECK_0);
    ss.next();
  }
  _locals_size = init_local_num;

  
  BasicType return_t = ss.type();
  if (return_t == T_OBJECT || return_t == T_ARRAY) {
    symbolOop sig = ss.as_symbol(CHECK_0);
    return _verifier->_local_class_type_table->get_class_type_from_name(sig, CHECK_0);
  } else {
    return VerificationType::get_primary_type(return_t);
  }
}

void StackMapFrame::copy_locals(const StackMapFrame* src) {
  u2 len = src->locals_size();
  assert(len == _locals_size, "The two stack map frames does not match");
  for (u2 i = 0; i < len; i++) {
    _locals[i] = src->locals()[i];
  }
}

void StackMapFrame::copy_stack(const StackMapFrame* src) {
  u2 len = src->stack_size();
  assert(len == _stack_size, "The two stack map frames does not match");
  for (u2 i = 0; i < len; i++) {
    _stack[i] = src->stack()[i];
  }
}


bool StackMapFrame::is_assignable_to(VerificationType** from, VerificationType** to, u2 len, TRAPS) {
  bool subtype = true;
  for (u2 i = 0; i < len; i++) {
    subtype = to[i]->is_assignable_from(from[i], CHECK_0);
    if (!subtype) {
      return false;
    }
  }
  return true;
}

bool StackMapFrame::is_assignable_to(const StackMapFrame* target, TRAPS) const {
  if (_max_locals != target->max_locals() || _stack_size != target->stack_size()) {
    return false;
  }
  // Only need to compare type elements up to target->locals() or target->stack().
  // The remaining type elements in this state can be ignored because they are
  // assignable to bogus type.
  bool match_locals = is_assignable_to(_locals, target->locals(), target->locals_size(), CHECK_0);
  bool match_stack = is_assignable_to(_stack, target->stack(), _stack_size, CHECK_0);
  bool match_flags = (_flags | target->flags()) == target->flags();
  return ( match_locals && match_stack && match_flags);
}

VerificationType* StackMapFrame::pop_stack(VerificationType* type, TRAPS) {
  if (_stack_size <= 0) {
    verify_error("Operand stack underflow in method %s at offset %d",
                  _verifier->_method, _offset, CHECK_0);
  }
  VerificationType* top = _stack[--_stack_size];
  bool subtype = type->is_assignable_from(top, CHECK_0);
  if (!subtype) {
    verify_error("Bad type on operand stack in method %s at offset %d",
                 _verifier->_method, _offset, CHECK_0);
  }
  NOT_PRODUCT( _stack[_stack_size] = VerificationType::_bogus_type; )
  return top;
}

VerificationType* StackMapFrame::get_local(u2 index, VerificationType* type, TRAPS) {
  if (index >= _max_locals) {
    verify_error("Local variable table overflow in method %s at offset %d",
                 _verifier->_method, _offset, CHECK_0);
  }
  bool subtype = type->is_assignable_from(_locals[index], CHECK_0);
  if (!subtype) {
    ResourceMark rm(THREAD);
    Exceptions::fthrow(THREAD_AND_LOCATION, vmSymbols::java_lang_VerifyError(),
                       "Bad local variable type at index %d in method %s at offset %d",
                        index, _verifier->_method->name_and_sig_as_C_string(), _offset, CHECK_0);
  }
  if(index >= _locals_size) { _locals_size = index + 1; }
  return _locals[index];
}

void StackMapFrame::set_local(u2 index, VerificationType* type, TRAPS) {
  if (index >= _max_locals) {
    verify_error("Local variable table overflow in method %s at offset %d",
                 _verifier->_method, _offset, CHECK);
  }
  // If type at index is double or long, set the next location to be unusable
  if (_locals[index]->is_double() || _locals[index]->is_long()) {
    assert((index + 1) < _locals_size, "Local variable table overflow");
    _locals[index + 1] = VerificationType::_bogus_type;
  }
  // If type at index is double_2 or long_2, set the previous location to be unusable
  if (_locals[index]->is_double2() || _locals[index]->is_long2()) {
    assert(index >= 1, "Local variable table underflow");
    _locals[index - 1] = VerificationType::_bogus_type;
  }
  _locals[index] = type;
  if(index >= _locals_size) { _locals_size = index + 1; }
}

#ifndef PRODUCT

void StackMapFrame::print() const {
  tty->print_cr("stackmap_frame[%d]:", _offset);
  tty->print_cr("flags = 0x%x", _flags);
  tty->print("locals[%d] = { ", _locals_size);
  for (u2 i = 0; i < _locals_size; i++) {
    _locals[i]->print_on(tty);
  }
  tty->print_cr(" }");
  tty->print("stack[%d] = { ", _stack_size);
  for (u2 j = 0; j < _stack_size; j++) {
    _stack[j]->print_on(tty);
  }
  tty->print_cr(" }");
}

#endif

