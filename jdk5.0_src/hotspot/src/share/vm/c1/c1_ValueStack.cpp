#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_ValueStack.cpp	1.57 04/04/14 17:27:23 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_ValueStack.cpp.incl"


// Implementation of ValueStack

ValueStack::ValueStack(IRScope* scope, int locals_size, int max_stack_size)
: _scope(scope)
, _locals(locals_size, NULL)
, _stores(locals_size, NULL)
, _stack(max_stack_size)
, _lock_stack(false)
, _locks(1)
{
  assert(scope != NULL, "scope must exist");
}


ValueStack::ValueStack(ValueStack* s)
: _scope(s->scope())
, _locals(s->locals_size(), NULL)
, _stores(s->locals_size(), NULL)
, _stack(s->max_stack_size())
, _lock_stack(s->is_lock_stack())
, _locks(s->locks_size())
{
  init(s);
}


ValueStack* ValueStack::copy() {
  ValueStack* s = new ValueStack(scope(), locals_size(), max_stack_size());
  s->_stack.appendAll(&_stack);
  s->_locks.appendAll(&_locks);
  return s;
}


ValueStack* ValueStack::copy_locks() {
  int sz = scope()->lock_stack_size();
  if (stack_size() == 0) {
    sz = 0;
  }
  ValueStack* s = new ValueStack(scope(), 0, sz);
  s->_lock_stack = true;
  s->_locks.appendAll(&_locks);
  if (sz > 0) {
    assert(sz <= stack_size(), "lock stack underflow");
    for (int i = 0; i < sz; i++) {
      s->_stack.append(_stack[i]);
    }
  }
  return s;
}


void ValueStack::init(ValueStack* s) {
  assert(s != NULL, "state must exist");
  assert(scope      () == s->scope      (), "scopes       must correspond");
  assert(locals_size() == s->locals_size(), "locals sizes must correspond");
  // introduce a stack phi for each stack element of s
  for (int i = 0; i < s->stack_size();) {
    ValueType* t = s->stack_at_inc(i)->type();
    push(t, new Phi(t));
  }
  assert(stack_size() == s->stack_size(), "stack sizes must correspond");
  // introduce a lock element for each lock element of s - simply
  // copy them for now as we are only interested in the locks size
  _locks.appendAll(&s->_locks);
}


bool ValueStack::is_same(ValueStack* s) {
  assert(s != NULL, "state must exist");
  assert(scope      () == s->scope      (), "scopes       must correspond");
  assert(locals_size() == s->locals_size(), "locals sizes must correspond");
  return is_same_across_scopes(s);
}


bool ValueStack::is_same_across_scopes(ValueStack* s) {
  assert(s != NULL, "state must exist");
  assert(stack_size () == s->stack_size (), "stack  sizes must correspond");
  assert(locks_size () == s->locks_size (), "locks  sizes must correspond");
  // compare each stack element with the corresponding stack element of s
  for (int i = 0; i < s->stack_size();) {
    int i0 = i; // i is modified by stack_at_inc(...)
    Value x = stack_at_inc(i0);
    Value y = s->stack_at_inc(i);
    if (x->type()->tag() != y->type()->tag()) {
      return false;
    }
  }
  // compare each locks element with the corresponding locks element of s
  for (int j = 0; j < s->locks_size(); j++) {
    if (_locks.at(j) != s->_locks.at(j)) {
      return false;
    }
  }
  return true;
}


ValueStack* ValueStack::caller_state() const {
  return scope()->caller_state();
}


void ValueStack::clear_locals() {
  ValueStack* stack = this;
  while (stack != NULL) {
    for (int i = 0; i < stack->_locals.length(); i++) stack->_locals.at_put(i, NULL);
    stack = stack->caller_state();
  }
  clear_stores();
}


void ValueStack::store_local(StoreLocal* x, int bci) {
  int i = x->java_index();
  Value instr = _stores.at(i);
  if (instr != NULL) {
    StoreLocal* store = instr->as_StoreLocal();
    assert(store != NULL, "just checking");
    store->set_eliminated(true);
    if (PrintStoreElimination) {
      tty->print_cr("store local %d (index %d) eliminated %d @ %d",
                    x->id(),
                    x->java_index(),
                    instr->id(),
                    bci);
    }
  }
  // NOTE: currently we only allow store elimination for arguments of
  // inlined methods -- which is where most of the inefficiency was
  // being generated anyway. Running Java2Demo with
  // PrintStoreElimination and
  // CompileOnly=GradientPaintContext.\<init\> (at least the JDK 1.4
  // version) shows a situation where the store elimination algorithm
  // is incorrect. In general we can not know whether the right hand
  // side of the store either has side effects or must be evaluated
  // (i.e., it is a LoadLocal which must be executed before subsequent
  // instructions overwrite the local, and unpinning the StoreLocal
  // causes it to be skipped).
  if (x->is_inline_argument()) {
    _stores.at_put(i, x);
  }
}


void ValueStack::clear_store(int index) {
  _stores.at_put(index, NULL);
}


void ValueStack::clear_stores() {
  ValueStack* stack = this;
  while (stack != NULL) {
    for (int i = 0; i < stack->_stores.length(); i++) stack->_stores.at_put(i, NULL);
    stack = stack->caller_state();
  }
}


void ValueStack::eliminate_stores(int bci) {
  for (int i = 0; i < _stores.length(); i++) {
    Value instr = _stores.at(i);
    if (instr != NULL) {
      StoreLocal* store = instr->as_StoreLocal();
      assert(store != NULL, "just checking");
      store->set_eliminated(true);
      _stores.at_put(i, NULL);
      if (PrintStoreElimination) {
        tty->print_cr("store local %d (index %d) eliminated @ %d",
                      store->id(),
                      store->java_index(),
                      bci);
      }
    }
  }
}


void ValueStack::eliminate_all_scope_stores(int bci) {
  ValueStack* stack = this;
  while (stack != NULL) {
    stack->eliminate_stores(bci);
    stack = stack->caller_state();
  }
}

void ValueStack::pin_stack_locals(int index) {
  if (ValueStackPinStackAll) {
    pin_stack_all(Instruction::PinStackLocals);
    return;
  }

  // this function could be made more selective by taking the type into account
  for (int i = 0; i < stack_size();) {
    AccessLocal* t = stack_at_inc(i)->as_AccessLocal();
    if (t != NULL && t->java_index() == index) {
      t->pin(Instruction::PinStackLocals);
    }
  }
}


void ValueStack::pin_stack_fields(ciField* field) {
  if (ValueStackPinStackAll) {
    pin_stack_all(Instruction::PinStackFields);
    return;
  }

  // this function could be made more selective by taking the type into account
  for (int i = 0; i < stack_size();) {
    AccessField* t = stack_at_inc(i)->as_AccessField();
    if (t != NULL && t->field() == field) t->pin(Instruction::PinStackFields);
  }
}


void ValueStack::pin_stack_indexed(ValueType* type) {
  if (ValueStackPinStackAll) {
    pin_stack_all(Instruction::PinStackIndexed);
    return;
  }

  // this function could be made more selective by taking the type into account
  for (int i = 0; i < stack_size();) {
    AccessIndexed* t = stack_at_inc(i)->as_AccessIndexed();
    if (t != NULL && t->type() == type) t->pin(Instruction::PinStackIndexed);
  }
}


void ValueStack::pin_stack_for_state_split() {
  // extend this later to walk the trees and pin all array and field accesses
  for (int i = 0; i < stack_size();) {
    Value v = stack_at_inc(i);
    if (v->as_LoadLocal() == NULL && v->as_Constant() == NULL) v->pin(Instruction::PinStackForStateSplit);
  }
}


void ValueStack::pin_stack_all(Instruction::PinReason reason) {
  for (int i = 0; i < stack_size();) stack_at_inc(i)->pin(reason);
}


void ValueStack::values_do(void f(Value*)) {
  for (int i = 0; i < stack_size();) {
#ifdef ASSERT
    // in debug mode, make sure that we keep track of
    // a potential HiWord and update it if necessary
    Value* ta = _stack.adr_at(i);
    Value t0 = *ta; // value before application of f
    assert(t0->as_HiWord() == NULL, "address points to hi word");
    assert(t0->type()->is_single_word() || t0 == _stack.at(i+1)->as_HiWord()->lo_word(), "stack inconsistent");
    f(ta);
    Value t1 = *ta; // value after application of f - may have changed (substitution)
    assert(t1->as_HiWord() == NULL, "address points to hi word");
    // if value changed we need to adjust HiWord if any
    if (t1 != t0) {
      assert(t1->type()->tag() == t0->type()->tag(), "types must match");
      if (t1->type()->is_double_word()) _stack.at_put(i+1, new HiWord(t1));
    }
#else
    // in optimized mode, ignore the 2nd word
    // of double word values since it is NULL
    f(_stack.adr_at(i));
#endif // ASSERT
    stack_at_inc(i);
  }
}


Values* ValueStack::pop_arguments(int argument_size) {
  assert(stack_size() >= argument_size, "stack too small or too many arguments");
  int base = stack_size() - argument_size;
  Values* args = new Values(argument_size);
  for (int i = base; i < stack_size();) args->push(stack_at_inc(i));
  truncate_stack(base);
  return args;
}


int ValueStack::lock(IRScope* scope, Value obj) {
  _locks.push(obj);
  scope->set_min_number_of_locks(locks_size());
  return locks_size() - 1;
}


int ValueStack::unlock() {
  _locks.pop();
  return locks_size();
}


ValueStack* ValueStack::push_scope(IRScope* scope) {
  assert(scope->caller() == _scope, "scopes must have caller/callee relationship");
  ValueStack* res = new ValueStack(scope,
                                   scope->method()->max_locals(),
                                   max_stack_size() + scope->method()->max_stack());
  // Preserves stack and monitors.
  res->_stack.appendAll(&_stack);
  res->_locks.appendAll(&_locks);
  assert(res->_stack.size() <= res->max_stack_size(), "stack overflow");
  return res;
}


ValueStack* ValueStack::pop_scope(bool should_eliminate_stores, int bci) {
  assert(_scope->caller() != NULL, "scope must have caller");
  IRScope* scope = _scope->caller();
  int max_stack = max_stack_size() - _scope->method()->max_stack();
  assert(max_stack >= 0, "stack underflow");
  ValueStack* res = new ValueStack(scope,
                                   scope->method()->max_locals(),
                                   max_stack);
  // Preserves stack and monitors. Restores local and store state from caller scope.
  res->_stack.appendAll(&_stack);
  res->_locks.appendAll(&_locks);
  ValueStack* caller = caller_state();
  if (caller != NULL) {
    for (int i = 0; i < caller->_locals.length(); i++) {
      res->_locals.at_put(i, caller->_locals.at(i));
      res->_stores.at_put(i, caller->_stores.at(i));
    }
    assert(res->_locals.length() == res->scope()->method()->max_locals(), "just checking");
    assert(res->_stores.length() == res->scope()->method()->max_locals(), "just checking");
  }
  assert(res->_stack.size() <= res->max_stack_size(), "stack overflow");
  if (EliminateStores && should_eliminate_stores) eliminate_stores(bci);
  return res;
}


#ifndef PRODUCT
void ValueStack::print() {
  if (stack_is_empty()) {
    tty->print_cr("empty stack");
  } else {
    InstructionPrinter ip;
    for (int i = stack_size(); i > 0;) {
      Value t = stack_at_dec(i);
      tty->print("%2d  ", i);
      ip.print_instr(t);
      tty->cr();
    }
  }
  if (!no_active_locks()) {
    InstructionPrinter ip;
    for (int i = locks_size() - 1; i >= 0; i--) {
      Value t = _locks.at(i);
      tty->print("lock %2d  ", i);
      if (t == NULL) {
        tty->print("this");
      } else {
        ip.print_instr(t);
      }
      tty->cr();
    }
  }
}


void ValueStack::verify() {
  Unimplemented();
}
#endif // PRODUCT

