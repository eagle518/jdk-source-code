#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_Instruction.cpp	1.78 04/04/20 15:56:17 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c1_Instruction.cpp.incl"


// Implementation of Instruction


int Instruction::_next_id = 0;


Instruction::Condition Instruction::mirror(Condition cond) {
  switch (cond) {
    case eql: return eql;
    case neq: return neq;
    case lss: return gtr;
    case leq: return geq;
    case gtr: return lss;
    case geq: return leq;
  }
  ShouldNotReachHere();
  return eql;
}


Instruction::Condition Instruction::negate(Condition cond) {
  switch (cond) {
    case eql: return neq;
    case neq: return eql;
    case lss: return geq;
    case leq: return gtr;
    case gtr: return leq;
    case geq: return lss;
  }
  ShouldNotReachHere();
  return eql;
}


Instruction* Instruction::prev(BlockBegin* block) {
  Instruction* p = NULL;
  Instruction* q = block;
  while (q != this) {
    assert(q != NULL, "this is not in the block's instruction list");
    p = q; q = q->next();
  }
  return p;
}


#ifndef PRODUCT
void Instruction::print() {
  InstructionPrinter ip;
  print(ip);
}


void Instruction::print(InstructionPrinter& ip) {
  ip.print_head();
  ip.print_line(this);
  tty->cr();
}
#endif // PRODUCT


ciType* LoadIndexed::exact_type() const {
  ciType* array_type = array()->exact_type();
  if (array_type == NULL) {
    return NULL;
  }
  assert(array_type->is_array_klass(), "what else?");
  ciArrayKlass* ak = (ciArrayKlass*)array_type;
  return ak->element_type();
}


ciType* LoadIndexed::declared_type() const {
  ciType* array_type = array()->declared_type();
  if (array_type == NULL) {
    return NULL;
  }
  assert(array_type->is_array_klass(), "what else?");
  ciArrayKlass* ak = (ciArrayKlass*)array_type;
  return ak->element_type();
}


ciType* LoadField::declared_type() const {
  return field()->type();
}


ciType* LoadField::exact_type() const {
  ciType* type = declared_type();
  // for primitive arrays, the declared type is the exact type
  if (type->is_type_array_klass()) {
    return type;
  }
  return NULL;
}


ciType* NewTypeArray::exact_type() const {
  return ciTypeArrayKlass::make(elt_type());
}


ciType* NewObjectArray::exact_type() const {
  return ciObjArrayKlass::make(klass());
}


ciType* NewInstance::exact_type() const {
  return klass();
}


// Implementation of AccessField

void AccessField::other_values_do(void f(Value*)) { 
  if (state_before() != NULL) state_before()->values_do(f);
}


// Implementation of StoreIndexed

IRScope* StoreIndexed::scope() const {
  return lock_stack()->scope();
}


// Implementation of ArithmeticOp

bool ArithmeticOp::is_commutative() const {
  switch (op()) {
    case Bytecodes::_iadd: // fall through
    case Bytecodes::_ladd: // fall through
    case Bytecodes::_fadd: // fall through
    case Bytecodes::_dadd: // fall through
    case Bytecodes::_imul: // fall through
    case Bytecodes::_lmul: // fall through
    case Bytecodes::_fmul: // fall through
    case Bytecodes::_dmul: return true; 
  }
  return false;
}


bool ArithmeticOp::can_trap() const {
  switch (op()) {
    case Bytecodes::_idiv: // fall through
    case Bytecodes::_ldiv: // fall through
    case Bytecodes::_irem: // fall through
    case Bytecodes::_lrem: return true;
  }
  return false;
}


// Implementation of LogicOp

bool LogicOp::is_commutative() const {
#ifdef ASSERT
  switch (op()) {
    case Bytecodes::_iand: // fall through
    case Bytecodes::_land: // fall through
    case Bytecodes::_ior : // fall through
    case Bytecodes::_lor : // fall through
    case Bytecodes::_ixor: // fall through
    case Bytecodes::_lxor: break;
    default              : ShouldNotReachHere();
  }
#endif
  // all LogicOps are commutative
  return true;
}


// Implementation of CompareOp

void CompareOp::other_values_do(void f(Value*)) { 
  if (state_before() != NULL) state_before()->values_do(f);
}


// Implementation of IfOp

bool IfOp::is_commutative() const {
  return cond() == eql || cond() == neq;
}


// Implementation of StateSplit

IRScope* StateSplit::scope() const {
  return _state->scope();
}


void StateSplit::state_values_do(void f(Value*)) { 
  if (state() != NULL) state()->values_do(f);
}

// Implementation of Invoke


Invoke::Invoke(Bytecodes::Code code, ValueType* result_type, Value recv, Values* args,
               int vtable_index, bool target_is_final, bool target_is_loaded, bool target_is_strictfp)
  : StateSplit(result_type)
  , _code(code)
  , _recv(recv)
  , _args(args)
  , _vtable_index(vtable_index)
{
  set_needs_null_check(_code == Bytecodes::_invokespecial && (!EliminateNullChecks || (recv != NULL)));
  set_flag(TargetIsFinalFlag, target_is_final);
  set_flag(TargetIsLoadedFlag, target_is_loaded);
  set_flag(TargetIsStrictfpFlag, target_is_strictfp);

  assert(args != NULL, "args must exist");
#ifdef ASSERT
  values_do(assert_value);
#endif // ASSERT

  // provide an initial guess of signature size.
  _signature = new BasicTypeList(number_of_arguments() + (has_receiver() ? 1 : 0));
  if (has_receiver()) {
    _signature->append(as_BasicType(receiver()->type()));
  }
  for (int i = 0; i < number_of_arguments(); i++) {
    ValueType* t = argument_at(i)->type();
    BasicType bt = as_BasicType(t);
    _signature->append(bt);
    if (t->is_double_word()) {
      _signature->append(bt);
    }
  }
}


int Invoke::size_of_arguments() const {
  int s = has_receiver() ? 1 : 0;
  int i = number_of_arguments();
  while (i-- > 0) s += argument_at(i)->type()->size();
  return s;
}


// Implementation of Contant
intx Constant::hash() const {
  if (_may_be_commoned) {
    switch (type()->tag()) {
    case intTag:
      return HASH2(name(), type()->as_IntConstant()->value());
    case floatTag:
      return HASH2(name(), jint_cast(type()->as_FloatConstant()->value()));
    case doubleTag:
      {
        jlong temp = jlong_cast(type()->as_DoubleConstant()->value());
        return HASH3(name(), high(temp), low(temp));
      }
    case objectTag:
      if (type()->as_ObjectConstant() != NULL) {
        return HASH2(name(), type()->as_ObjectConstant()->value());
      } else if (type()->as_ClassConstant() != NULL) {
        if (type()->as_ClassConstant()->value()->is_loaded()) {
          return HASH2(name(), type()->as_ClassConstant()->value());
        }
      }
    }
  }
  return 0;
}

bool Constant::is_equal(Value v) const {
  Constant* _v = v->as_Constant();
  if (_v == NULL  ) return false;

  switch (type()->tag()) {
    case intTag:
      return (v->type()->as_IntConstant() != NULL && _v->type()->as_IntConstant() != NULL &&
              type()->as_IntConstant()->value() == _v->type()->as_IntConstant()->value());
    case floatTag:
      return (v->type()->as_FloatConstant() != NULL && _v->type()->as_FloatConstant() != NULL &&
              jint_cast(type()->as_FloatConstant()->value()) == jint_cast(_v->type()->as_FloatConstant()->value()));
    case doubleTag:
      return (v->type()->as_DoubleConstant() != NULL && _v->type()->as_DoubleConstant() != NULL &&
              jlong_cast(type()->as_DoubleConstant()->value()) == jlong_cast(_v->type()->as_DoubleConstant()->value()));
    case objectTag:
      if (type()->as_ObjectConstant() != NULL) {
        return (v->type()->as_ObjectConstant() != NULL && _v->type()->as_ObjectConstant() != NULL &&
                type()->as_ObjectConstant()->value() == _v->type()->as_ObjectConstant()->value());
      } else if (type()->as_ClassConstant() != NULL) {
        return (v->type()->as_ClassConstant() != NULL && _v->type()->as_ClassConstant() != NULL &&
                type()->as_ClassConstant()->value() == _v->type()->as_ClassConstant()->value());
      } else {
        NOT_PRODUCT(ShouldNotReachHere());
        return false;
      }

  }
  return false;
}


void Constant::other_values_do(void f(Value*)) { 
  if (state() != NULL) state()->values_do(f);
}


// Implementation of NewArray

void NewArray::other_values_do(void f(Value*)) {
  if (state_before() != NULL) state_before()->values_do(f);
}


// Implementation of TypeCheck

void TypeCheck::other_values_do(void f(Value*)) {
  if (state_before() != NULL) state_before()->values_do(f);
}


// Implementation of BlockBegin

int BlockBegin::_next_block_id = 0;


void BlockBegin::set_weight(int start_distance) {
  // computes the weight of a block by looking at its bci, it's distance from the
  // start block and some other attributes - the word's bits are used as follows:
  //
  //  3 3 2 2222222221111111111 0000000000
  //  1 0 9 8765432109876543210 9876543210
  // [0|h|g|       bci         | distance ]
  //
  // the g & h bits are set to force the block to the end of a method
  assert(BitsPerWord == 32 || BitsPerWord == 64, "adjust this code");
  int the_bci = scope()->is_top_scope() ? bci() : scope()->top_scope_bci();
  _weight = ((the_bci & 0x7FFFF) << 10) | (start_distance & 0x3FF);
  if (end() != NULL && (end()->as_Throw() != NULL || end()->as_Return() != NULL) && !is_set(BlockBegin::std_entry_flag)) {
    // blocks that end with a Throw or Return instruction and that are not the first block
    // (method entry) are given a huge weight (h) to make them move to the end of the method -
    // Throws are always following after Returns
    _weight |= 1 << (end()->as_Throw() != NULL ? 30 : 29);
  };
  assert(_weight >= 0, "weight should be positive");
}


bool BlockBegin::enqueued_for_oop_map_gen() const {
  return is_set(enqueued_for_oop_map_gen_flag);
}


void BlockBegin::set_enqueued_for_oop_map_gen(bool value) {
  if (value) {
    set(enqueued_for_oop_map_gen_flag);
  } else {
    clear(enqueued_for_oop_map_gen_flag);
  }
}


void BlockBegin::add_exception_handler(BlockBegin* b) {
  assert(b != NULL && b->is_set(exception_entry_flag), "exception handler must exist");
  // add only if not in the list already
  if (!_exception_handlers.contains(b)) _exception_handlers.append(b);
}


void BlockBegin::iterate_preorder(boolArray& mark, BlockClosure* closure) {
  if (!mark.at(block_id())) {
    mark.at_put(block_id(), true);
    closure->block_do(this);
    BlockEnd* e = end(); // must do this after block_do because block_do may change it!
    { for (int i = number_of_exception_handlers() - 1; i >= 0; i--) exception_handler_at(i)->iterate_preorder(mark, closure); }
    { for (int i = e->number_of_sux            () - 1; i >= 0; i--) e->sux_at           (i)->iterate_preorder(mark, closure); }
  }
}


void BlockBegin::iterate_postorder(boolArray& mark, BlockClosure* closure) {
  if (!mark.at(block_id())) {
    mark.at_put(block_id(), true);
    BlockEnd* e = end();
    { for (int i = number_of_exception_handlers() - 1; i >= 0; i--) exception_handler_at(i)->iterate_postorder(mark, closure); }
    { for (int i = e->number_of_sux            () - 1; i >= 0; i--) e->sux_at           (i)->iterate_postorder(mark, closure); }
    closure->block_do(this);
  }
}


void BlockBegin::iterate_preorder(BlockClosure* closure) {
  boolArray mark(number_of_blocks(), false);
  iterate_preorder(mark, closure);
}


void BlockBegin::iterate_postorder(BlockClosure* closure) {
  boolArray mark(number_of_blocks(), false);
  iterate_postorder(mark, closure);
}


void BlockBegin::block_values_do(void f(Value*)) {
  for (Instruction* n = this; n != NULL; n = n->next()) n->values_do(f);
}


bool BlockBegin::try_join(ValueStack* s) {
  if (state() == NULL) {
    // first visit of block => must initialize state
    set_state(new ValueStack(s));
    return true;
  } else {
    // not first visit to this block => check if states correspond
    return state()->is_same_across_scopes(s);
  }
}


static void resolve_substituted_value(Value* x)  { *x = (*x)->subst(); }

void BlockBegin::resolve_substitution() {
  block_values_do(resolve_substituted_value);
}


#ifndef PRODUCT
void BlockBegin::print_block() {
  InstructionPrinter ip;
  print_block(ip, true);
}


void BlockBegin::print_block(InstructionPrinter& ip, bool live_only) {
  ip.print_instr(this); tty->cr();
  ip.print_stack(this->state()); tty->cr();
  ip.print_inline_level(this);
  ip.print_head();
  for (Instruction* n = next(); n != NULL; n = n->next()) {
    if (!live_only || n->is_pinned() || n->use_count() > 0) {
      ip.print_line(n);
    }
  }
  tty->cr();
}
#endif // PRODUCT


// Implementation of BlockList

void BlockList::iterate_forward (BlockClosure* closure) {
  const int l = length();
  for (int i = 0; i < l; i++) closure->block_do(at(i));
}


void BlockList::iterate_backward(BlockClosure* closure) {
  for (int i = length() - 1; i >= 0; i--) closure->block_do(at(i));
}


void BlockList::blocks_do(void f(BlockBegin*)) {
  for (int i = length() - 1; i >= 0; i--) f(at(i));
}


void BlockList::values_do(void f(Value*)) {
  for (int i = length() - 1; i >= 0; i--) at(i)->block_values_do(f);
}


#ifndef PRODUCT
void BlockList::print(bool cfg_only) {
  InstructionPrinter ip;
  for (int i = 0; i < length(); i++) {
    BlockBegin* block = at(i);
    if (cfg_only) {
      ip.print_instr(block); tty->cr();
    } else {
      block->print_block();
    }
  }
}
#endif // PRODUCT


// Implementation of BlockEnd

void BlockEnd::substitute_sux(BlockBegin* old_sux, BlockBegin* new_sux) {
  NOT_PRODUCT(bool assigned = false;)
  for (int i = 0; i < number_of_sux(); i++) {
    BlockBegin** sa = addr_sux_at(i);
    if (*sa == old_sux) {
      *sa = new_sux;
      NOT_PRODUCT(assigned = true;)
    }
  }
  assert(assigned == true, "should have assigned at least once");
}


void BlockEnd::other_values_do(void f(Value*)) {  
  if (state_before() != NULL) state_before()->values_do(f);
}


// Implementation of Throw

void Throw::state_values_do(void f(Value*)) {
  BlockEnd::state_values_do(f);
  if (check_flag(KeepStateBeforeAliveFlag)) {
    assert(state_before() != NULL, "must be set");
    state_before()->values_do(f);
  }
}
