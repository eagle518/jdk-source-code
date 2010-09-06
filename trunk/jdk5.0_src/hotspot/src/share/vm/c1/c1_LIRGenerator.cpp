#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIRGenerator.cpp	1.9 03/12/23 16:39:13 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_LIRGenerator.cpp.incl"

#ifndef PRODUCT

//--------------------------------------------------------------
// LIRItem

void LIRItem::load_item() {
  if (!result()->is_register()) {
    LIR_Opr reg = _gen->new_register(value()->type());
    _gen->emit()->lir()->move(result(), reg);
    set_result(reg);
  }
}


void LIRItem::load_item_with_reg_mask(c1_RegMask mask) {
  load_item();
  _gen->emit()->set_bailout("load_item_with_reg_mask not implemented");
}


void LIRItem::load_item_force(RInfo reg) {
  LIR_Opr dst = LIR_OprFact::rinfo(reg, as_BasicType(value()->type()));
  _gen->emit()->lir()->move(result(), dst);
  set_result(dst);
}


ciObject* LIRItem::get_jobject_constant() const {
  if (type()->as_ObjectConstant() != NULL) {
    return type()->as_ObjectConstant()->value();
  } else  if (type()->as_InstanceConstant() != NULL) {
    return (ciObject*)type()->as_InstanceConstant()->value();
  } else if (type()->as_ClassConstant() != NULL) {
    return type()->as_ClassConstant()->value();
  } else {
    assert(type()->as_ArrayConstant() != NULL, "wrong access");
    return (ciObject*)type()->as_ArrayConstant()->value();
  }
}


jint LIRItem::get_jint_constant() const {
  assert(is_constant() && value() != NULL, "");
  assert(type()->as_IntConstant() != NULL, "type check");
  return type()->as_IntConstant()->value();
}


jint LIRItem::get_address_constant() const {
  assert(is_constant() && value() != NULL, "");
  assert(type()->as_AddressConstant() != NULL, "type check");
  return type()->as_AddressConstant()->value();
}


jfloat LIRItem::get_jfloat_constant() const {
  assert(is_constant() && value() != NULL, "");
  assert(type()->as_FloatConstant() != NULL, "type check");
  return type()->as_FloatConstant()->value();
}


jdouble LIRItem::get_jdouble_constant() const {
  assert(is_constant() && value() != NULL, "");
  assert(type()->as_DoubleConstant() != NULL, "type check");
  return type()->as_DoubleConstant()->value();
}


jlong LIRItem::get_jlong_constant() const {
  assert(is_constant() && value() != NULL, "");
  assert(type()->as_LongConstant() != NULL, "type check");
  return type()->as_LongConstant()->value();
}



//--------------------------------------------------------------


void LIRGenerator::block_do_prolog(BlockBegin* block) {
  if (PrintIRWithLIR) {
    block->print();
  }

  start_block(block);
  bind_block_entry(block);
  // add PcDesc to debug info so we can find entry point for exception handling
  // (note: we are finding the handler entry point by searching the PcDescs - if
  //        we use another method to find the exception handler entry we don't
  //        need to emit this information anymore).
  ValueStack* lock_stack = block->state()->copy(); 
  lock_stack->clear_stack(); // only locks of interest and correctly set
  if (block->is_set(BlockBegin::exception_entry_flag)) exception_handler_start(block->scope(), block->bci(), lock_stack);
}


void LIRGenerator::block_do_epilog(BlockBegin* block) {
  if (emit() != NULL && emit()->must_bailout()) return;
  if (PrintIRWithLIR) {
    tty->cr();
  }
}


void LIRGenerator::block_do(BlockBegin* block) {
  if (emit() != NULL && emit()->must_bailout()) return;
  block_do_prolog(block);
  block_prolog(block);
  set_block(block);

  for (Instruction* instr = block; instr != NULL; instr = instr->next()) {
    if (instr->is_root()) do_root(instr);
  }
  set_block(NULL);
  block_epilog(block);
  block_do_epilog(block);
}


//-------------------------LIRGenerator-----------------------------

#define __ emit()->lir()->


bool LIRGenerator::is_backward_branch(Value instr, BlockBegin* dest) const {
  bool is_bb =  block()->is_after_block(dest);
  if (is_bb) {
    // the backward branch flag is set so that in the code emission pass we can 
    // align blocks that are backward branch targets
    dest->set(BlockBegin::backward_branch_target_flag);
  }
  return is_bb;
}


#ifndef PRODUCT
void LIRGenerator::print_at_do_root(Value instr, bool before_visit) {
}
#endif


// This is where the tree-walk starts; instr must be root;
void LIRGenerator::do_root(Value instr) {
#ifdef ASSERT
  InstructionMark im(compilation(), instr);
#endif // ASSERt

  _value = instr;
  assert(instr->is_root(), "use only with roots");

  if (emit()->must_bailout()) return;

  // Phi's are visited separately during block prolog
  if (instr->as_Phi() != NULL) {
    assert(instr->operand() != LIR_OprFact::illegalOpr, "phi item must be set");
    return;
  }

  print_at_do_root(instr, true);
  instr->visit(this);
  // emits code, allocates register
  finish_root(instr);

  print_at_do_root(instr, false);
  assert(!instr->has_uses() || instr->operand()->is_valid() || instr->as_Constant() != NULL, "invalid item set");
}


// releases result register or spilling slots if use_count of instruction is zero;
// this can occur only with root instructions
void LIRGenerator::check_result_usage(Value instr) {
  if (result()->is_valid() && !instr->has_uses()) {
    // there is a result but the use count is zero
    set_no_result(instr);
  }
}


#ifndef PRODUCT
void LIRGenerator::print_at_walk(Value instr, bool before_visit) {
}
#endif


// This is called for each node in tree; the walk stops if a root is reached
void LIRGenerator::walk(Value instr) {
#ifdef ASSERT
  InstructionMark im(compilation(), instr);
#endif // ASSERT
  //stop walk when encounter a root
  if (instr->is_root()) {
    assert(instr->operand() != LIR_OprFact::illegalOpr || instr->as_Constant() != NULL, "this root has not yet been visited");
  } else {
    assert(instr->use_count() <= 1, "an instruction with use_count > 1 must be root ");
    print_at_walk(instr, true);
    instr->visit(this);
    assert(instr->use_count() > 0, "leaf instruction must have a use");
    // check_result_usage(instr);
    print_at_walk(instr, false);
  }
}


CodeEmitInfo* LIRGenerator::state_for(Instruction* x, ValueStack* state) {
  return new CodeEmitInfo(emit(), x->bci(), NULL, state, x->exception_scope());
}


CodeEmitInfo* LIRGenerator::state_for(Instruction* x) {
  return new CodeEmitInfo(emit(), x->bci(), NULL, x->lock_stack(), x->exception_scope());
}


void LIRGenerator::round_item(LIR_Opr opr) {
  assert(opr->is_register(), "why spill if item is not register?");
  // XXX emit()->round(spill_ix, opr);
  emit()->set_bailout("round_item not implemented");
}


void LIRGenerator::spill_values_on_stack(ValueStack* stack, RInfo hide_reg, bool caller_to_callee) {
  // XXX TKR do nothing?
  // emit()->set_bailout("spill_values_on_stack not implemented");
}


// Check out code in "check_result_usage" and factor it out
void LIRGenerator::release_roots(ValueStack* stack) {
  for (int i = 0; i < stack->stack_size();) {
    Value val = stack->stack_at_inc(i);
    if (val->is_root()) {
    }
  }
}



// Phi technique:
// This is about passing live values from one basic block to the other.
// In code generated with Java it is rather rare that more than one
// value is on the stack from one basic block to the other.
// We optimize our technique for efficient passing of one value
// (of type long, int, double..) but it can be extended.
// When entering or leaving a basic block, all registers and all spill
// slots are release and empty. We use the released registers
// and spill slots to pass the live values from one block
// to the other. The topmost value, i.e., the value on TOS of expression
// stack is passed in registers. All other values are stored in spilling
// area. Every Phi has an index which designates its spill slot
// At exit of a basic block, we fill the register(s) and spill slots.
// At entry of a basic block, the block_prolog sets up the content of phi nodes
// and locks necessary registers and spilling slots.


// return phi tos value; spill slots for other values are returned in 'spill_ixs' and values in 'phi_values'
// 'stack' describes the spill values
Value LIRGenerator::compute_phi_arrays(ValueStack* stack, Values* phi_values, intStack* spill_ixs) {
  if (stack->stack_size() == 0) return NULL;
  bool  is_tos  = true;
  Value tos_val = NULL;
  // compute spill slots, spill values and tos-vallue
  for (int i = stack->stack_size(); i > 0;) {
    Value val = stack->stack_at_dec(i);
    assert(val != NULL, "wrong stack access");
    if (is_tos) {
      is_tos = false; tos_val = val;
    } else {
      int spill_ix = i;
      assert (0 <= spill_ix && spill_ix < stack->stack_size(), "wrong spill index");
      phi_values->append(val);
      spill_ixs->append(spill_ix);
    }
  }
  return tos_val;
}


// Moves all stack values into their PHI position
void LIRGenerator::move_to_phi(ValueStack* stack) {
  if (stack->stack_size() == 0) {
    return;
  }
  emit()->set_bailout("move_to_phi not implemented");
}


LIR_Opr LIRGenerator::new_register(BasicType type) {
  int vreg = _virtual_register_number;
  if (type == T_LONG || type == T_DOUBLE) {
    _virtual_register_number += 2;
  } else {
    _virtual_register_number += 1;
  }
  return LIR_OprFact::virtual_register(vreg, type);
}


// Try to lock using register in hint
LIR_Opr LIRGenerator::rlock(Value instr) {
  return new_register(instr->type());
}


// does an rlock and sets result
LIR_Opr LIRGenerator::rlock_result(Value x) {
  LIR_Opr reg = rlock(x);
  set_result(x, reg);
  return reg;
}


// does an rlock and sets result
LIR_Opr LIRGenerator::rlock_result(Value x, BasicType type) {
  LIR_Opr reg;
  switch (type) {
  case T_BYTE:
  case T_BOOLEAN:
    reg = rlock(x, _byte_reg_mask);
    break;
  default:
    reg = rlock(x);
    break;
  }

  set_result(x, reg);
  return reg;
}


// does an rlock and sets result
LIR_Opr LIRGenerator::rlock(Value x, c1_RegMask mask) {
  // XXX TKR need to handle byte mask case
  LIR_Opr reg = rlock(x);
  set_result(x, reg);
  return reg;
}


LIR_Opr LIRGenerator::set_with_result_register(Value x) {
  LIR_Opr reg = result_register_for(x->type());
  set_result(x, reg);
  return reg;
}


//---------------------------------------------------------------------

// Call this after finished generating code for root
// If the item of root is a constant or spilled, then leave it as it is
// If the item of root is a stack value  then preload it so that the following 
// bytecode sequence is executed correctly:
//          load local 0
//          load local 0
//          inc 1
//          store local 0
//          ....            <- on stack is the "old" local 0


// Call this after finished generating code for root
void LIRGenerator::finish_root(Value instr) {
}


ciObject* LIRGenerator::get_jobject_constant(Value value) {
  ValueType* type = value->type();
  if (type->as_ObjectConstant() != NULL) {
    return type->as_ObjectConstant()->value();
  } else  if (type->as_InstanceConstant() != NULL) {
    return (ciObject*)type->as_InstanceConstant()->value();
  } else if (type->as_ClassConstant() != NULL) {
    return type->as_ClassConstant()->value();
  } else {
    assert(type->as_ArrayConstant() != NULL, "wrong access");
    return (ciObject*) type->as_ArrayConstant()->value(); // XXX
  }
}




void LIRGenerator::block_epilog(BlockBegin* block) {
  // nothing to do for now.
}



void LIRGenerator::start_block(BlockBegin* block) {
  emit()->start_block(block);
  if (block->is_set(BlockBegin::backward_branch_target_flag)) {
    emit()->align_backward_branch(); 
  }
}



void LIRGenerator::block_prolog(BlockBegin* block) {
  // setup phi's
  assert(block->state() != NULL, "block state must exist");

  // setup phi's
  if (!block->state()->stack_is_empty()) { 
    emit()->set_bailout("setup phis not implemented");
  }
}


void LIRGenerator::bind_block_entry(BlockBegin* block) {
  emit()->bind_block_entry(block);
}


void LIRGenerator::exception_handler_start(IRScope* scope, int bci, ValueStack* lock_stack) {
  // the nop at start of exception handler prevents that we emit two pc/bci 
  // mappings with identical pc's but varying bci's
  emit()->handler_entry();
}


//-----------------------register allocation---------------------------------------------

LIR_Opr LIRGenerator::result_register_for(ValueType* type, bool callee) {
  ValueTag tag = type->tag();
  RInfo reg = norinfo;
  switch (tag) {
  case intTag:
  case objectTag:
  case addressTag: reg =  callee ? callee_return1RInfo() : return1RInfo(); break;
    
  case longTag:    reg =  callee ? callee_return2RInfo() : return2RInfo(); break;
    
  case floatTag:   reg = returnF0RInfo(); break;
  case doubleTag:  reg = returnD0RInfo(); break;
    
  default: ShouldNotReachHere(); return LIR_OprFact::illegalOpr;
  }
  return LIR_OprFact::rinfo(reg, as_BasicType(type));
}



//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//                        visitor functions
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//----------------------------------------------------------------------

// Phi that represent the TOS is in register; all others are in spill area
// TOS(1) element is at spill index 0.
void LIRGenerator::do_Phi(Phi* x) {
  assert(x->is_root(), "phi must be root");
  assert(x->operand() != LIR_OprFact::illegalOpr, "phi's item must be set by block prolog");
  Unimplemented();
}


// Code for a constant is generated lazily unless the constant is frequently used and can't be inlined.
void LIRGenerator::do_Constant(Constant* x) {
  if (x->use_count() > 1 && !can_inline_as_constant(x)) {
    ValueType* type = x->type();
    NEEDS_CLEANUP // should handle initialized class constants too
    if (type->as_ClassConstant() == NULL) {
      LIR_Opr reg = rlock(x);
      __ move(compilation()->lir_opr_for_instruction(x), reg);
      set_result(x, reg);
    }
  } else {
    set_result(x, LIR_OprFact::value_type(x->type()));
  }
}


void LIRGenerator::do_Local(Local* x) {
  ShouldNotReachHere();
}


// Example: object.getClass ()
void LIRGenerator::do_getClass(Intrinsic* x) {
  assert(x->number_of_arguments() == 1, "wrong type");

  LIRItem rcvr(x->argument_at(0), this);
  rcvr.load_item();
  LIR_Opr result = rlock(x);

  // need to perform the null check on the rcvr
  CodeEmitInfo* info = NULL;
  if (x->needs_null_check()) {
    info = state_for(x, x->state());
  }
  emit()->getClass(result->rinfo(), rcvr.result()->rinfo(), info);
}


//------------------------local access--------------------------------------

void LIRGenerator::do_StoreLocal(StoreLocal* x) {
  set_no_result(x);
  LIRItem vg(x->value(), this);
  if (can_inline_any_constant() && vg.is_constant()) {
    // let it be a constant
    vg.dont_load_item();
  } else {
    vg.load_item();
  }
  if (x->local()->operand()->is_illegal()) {
    // allocate a virtual register for this local
    x->local()->set_operand(rlock(x->local()));
  }
  LIR_Opr reg = x->local()->operand();
  __ move(vg.result(), reg);
  x->set_operand(reg);
}


void LIRGenerator::do_LoadLocal(LoadLocal* x) {
  assert(x->use_count() == 0 || x->has_offset(), "no offset assigned to LoadLocal");
  if (x->local()->operand()->is_illegal()) {
    // allocate a virtual register for this local
    x->local()->set_operand(rlock(x->local()));
  }
  set_result(x, x->local()->operand());
}


//------------------------field access--------------------------------------

// Comment copied form templateTable_i486.cpp
// ----------------------------------------------------------------------------
// Volatile variables demand their effects be made known to all CPU's in
// order.  Store buffers on most chips allow reads & writes to reorder; the
// JMM's ReadAfterWrite.java test fails in -Xint mode without some kind of
// memory barrier (i.e., it's not sufficient that the interpreter does not
// reorder volatile references, the hardware also must not reorder them).
// 
// According to the new Java Memory Model (JMM):
// (1) All volatiles are serialized wrt to each other.  
// ALSO reads & writes act as aquire & release, so:
// (2) A read cannot let unrelated NON-volatile memory refs that happen after
// the read float up to before the read.  It's OK for non-volatile memory refs
// that happen before the volatile read to float down below it.
// (3) Similar a volatile write cannot let unrelated NON-volatile memory refs
// that happen BEFORE the write float down to after the write.  It's OK for
// non-volatile memory refs that happen after the volatile write to float up
// before it.
//
// We only put in barriers around volatile refs (they are expensive), not
// _between_ memory refs (that would require us to track the flavor of the
// previous memory refs).  Requirements (2) and (3) require some barriers
// before volatile stores and after volatile loads.  These nearly cover
// requirement (1) but miss the volatile-store-volatile-load case.  This final
// case is placed after volatile-stores although it could just as well go
// before volatile-loads.


void LIRGenerator::do_StoreField(StoreField* x) {
  bool needs_patching = x->needs_patching();
  bool can_trap = needs_patching || x->needs_null_check();
  ValueStack* field_stack = x->lock_stack();
  if (needs_patching) {
    field_stack = x->state_before();
  }
  ValueTag valueTag = x->type()->tag();
  bool isFloatKind = valueTag == floatTag || valueTag == doubleTag;
  // valueTag, differs from field_type
  bool needs_byte_reg = x->field_type() == T_BYTE || x->field_type() == T_BOOLEAN;

  LIRItem object(x->obj(), this);
  LIRItem value(x->value(),  this);
  CodeEmitInfo* info = NULL;

  if (valueTag == objectTag) {
    // SPARC Note: we could use a G register for preserving the object register instead
    //             of destroying it; this would free up a potentially caller saved register for
    //             caching
    object.set_destroys_register();
  }

  if (needs_patching && x->is_static()) {
    LIR_Opr reg = rlock(x->obj());
    CodeEmitInfo* info = state_for(x, field_stack);
    emit()->jobject2reg_with_patching(reg->rinfo(), get_jobject_constant(x->obj()), info);
    object.set_result(reg);
  } else {
    object.load_item();
  }

  LIR_Opr tmp = LIR_OprFact::rinfo(scratch1_RInfo());
  if (x->field()->is_volatile() || !value.is_constant() || needs_patching || !can_inline_any_constant()) {
    // load item if field is volatile (less special cases for volatiles)
    // load item if field not initialized
    // load item if field not constant
    // load item if cannot inline constants
    // because of code patching we cannot inline constants

    if (prefer_alu_registers() && !needs_patching && value.is_stack() && isFloatKind) {
      // use integer registers for floats and doubles
      tmp = new_register(valueTag == floatTag ? (ValueType*)intType : (ValueType*)longType);
    } else {
      if (needs_byte_reg) { 
        value.load_byte_item();
      } else  {  
        value.load_item();       
      }
    }

  } else {
    assert(value.is_constant(), "sanity check");
    // does not need patching and value is a constant
    value.dont_load_item();
  }

  set_no_result(x);

  if (PrintNotLoaded && needs_patching) {
    tty->print_cr("   ###class not loaded at store_%s bci %d", x->is_static() ?  "static" : "field", x->bci());
  }
  if (info == NULL && can_trap) {
    NullCheck* nc = x->explicit_null_check();
    if (nc == NULL) {
      info = state_for(x, field_stack);
    } else {
      info = state_for(nc);
    }
  }
  if (x->field()->is_volatile() && os::is_MP()) emit()->membar_release();
  emit()->field_store(x->field(), object.result(), x->offset(), value.result(), needs_patching, x->is_loaded(), info, tmp->rinfo());
  if (x->field()->is_volatile() && os::is_MP()) emit()->membar();

}


void LIRGenerator::do_LoadField(LoadField* x) {
  bool needs_patching = x->needs_patching();
  bool can_trap = needs_patching || x->needs_null_check();
  ValueStack* field_stack = x->lock_stack();
  if (needs_patching) {
    field_stack = x->state_before();
  }
  LIRItem object(x->obj(), this);

  CodeEmitInfo* info = NULL; 
  // must collect oops in spill beforehand, otherwise the spill element may be released 
  // and may not appear in the oop map list.
  if (needs_patching) {
    info = state_for(x, field_stack);
  }
  if (needs_patching && x->is_static()) {
    LIR_Opr reg = rlock(x->obj());
    CodeEmitInfo* info = state_for(x, field_stack);
    emit()->jobject2reg_with_patching(reg->rinfo(), get_jobject_constant(x->obj()), info);
    object.set_result(reg);
  } else {
    object.load_item();
  }
  if (info == NULL && can_trap) {
    NullCheck* nc = x->explicit_null_check();
    if (nc == NULL) {
      info = state_for(x, field_stack);
    } else {
      info = state_for(nc);
    }
  }
  // result should be hint
  LIR_Opr reg = rlock_result(x, x->field_type());

  if (PrintNotLoaded && needs_patching) {
    tty->print_cr("   ###class not loaded at load_%s bci %d", x->is_static() ?  "static" : "field", x->bci());
  }
  emit()->field_load(reg->rinfo(), x->field(), object.result(), x->offset(), needs_patching, x->is_loaded(), info);
  if (x->field()->is_volatile() && os::is_MP()) emit()->membar_acquire();
}


//------------------------java.nio.Buffer.checkIndex------------------------

// int java.nio.Buffer.checkIndex(int)
void LIRGenerator::do_NIOCheckIndex(Intrinsic* x) {
  // NOTE: by the time we are in checkIndex() we are guaranteed that
  // the buffer is non-null (because checkIndex is package-private and
  // only called from within other methods in the buffer).
  assert(x->number_of_arguments() == 2, "wrong type");
  LIRItem buf  (x->argument_at(0), this);
  LIRItem index(x->argument_at(1), this);
  buf.load_item();
  index.load_item();

  LIR_Opr result = rlock_result(x);
  if (GenerateRangeChecks) {
    CodeEmitInfo* info = state_for(x);
    emit()->nio_range_check(buf.result(), index.result(), result->rinfo(), info);
  } else {
    // Just load the index into the result register
    __ move(index.result(), result);
  }
}


//------------------------array access--------------------------------------


void LIRGenerator::do_ArrayLength(ArrayLength* x) {
  LIRItem array(x->array(), this);
  array.load_item();
  LIR_Opr reg = rlock_result(x);

  int array_length_offset = arrayOopDesc::length_offset_in_bytes(); 
  CodeEmitInfo* info = NULL;
  if (x->needs_null_check()) {
    NullCheck* nc = x->explicit_null_check();
    if (nc == NULL) {
      info = state_for(x);
    } else {
      info = state_for(nc);
    }
  }
  emit()->array_length(reg->rinfo(), array.result(), info);
}


void LIRGenerator::do_LoadIndexed(LoadIndexed* x) {
  LIRItem array(x->array(), this);
  LIRItem index(x->index(), this);

  array.load_item();
  if (index.is_constant() && can_inline_as_constant(x->index())) {
    // let it be a constant
    index.dont_load_item();
  } else {
    index.load_item();
  }

  LIR_Opr reg = rlock_result(x, x->elt_type());

  CodeEmitInfo* range_check_info = state_for(x);
  CodeEmitInfo* null_check_info = NULL;
  if (x->needs_null_check()) {
    NullCheck* nc = x->explicit_null_check();
    if (nc != NULL) {
      null_check_info = state_for(nc);
    } else {
      null_check_info = range_check_info;
    }
  }
  if (GenerateRangeChecks) {
    emit()->array_range_check(array.result(), index.result(), null_check_info, range_check_info);
    // The range check performs the null check, so clear it out for the load
    null_check_info = NULL;
  }
  emit()->indexed_load(reg->rinfo(), x->elt_type(), array.result(), index.result(), null_check_info);
}


void LIRGenerator::do_NullCheck(NullCheck* x) {
  LIRItem value(x->obj(), this);
  if (x->can_trap()) {
    value.load_item();
    CodeEmitInfo* info = state_for(x);
    emit()->null_check(value.result(), info);
  } else {
    // eliminate the check
    value.dont_load_item();
  }
}


void LIRGenerator::do_Throw(Throw* x) {
  if (!x->state()->stack_is_empty()) {
    // release all roots (computed values)
    release_roots(x->state());
  }

  LIRItem exception(x->exception(), this);
  exception.load_item();
  set_no_result(x);
  CodeEmitInfo* info = state_for(x, x->state());
  emit()->throw_op(exception.result(), exceptionOopRInfo(), exceptionPcRInfo(), info);
}


void LIRGenerator::do_UnsafeGetRaw(UnsafeGetRaw* x) {
  LIRItem obj(x->unsafe(), this);
  LIRItem src(x->base(), this);
  LIRItem idx(x->index(), this);

  if (x->needs_null_check()) {
    obj.load_item();
  } else {
    obj.dont_load_item();
  }
  src.load_item();
  idx.load_item();

  LIR_Opr reg = rlock_result(x, x->basic_type());

  if (x->needs_null_check()) {
    CodeEmitInfo* info = state_for(x);
    emit()->null_check(obj.result(), info);
  }

  int   log2_scale = 0;
  if (x->has_index()) {
    assert(x->index()->type() == intType, "should not find non-int index");
    log2_scale = x->log2_scale();
  }

  assert(!x->has_index() || idx.value() == x->index(), "should match");
  emit()->get_raw_unsafe(reg->rinfo(), src.result(), idx.result(), log2_scale, x->basic_type());
}


void LIRGenerator::do_UnsafePutRaw(UnsafePutRaw* x) {
  int  log2_scale = 0;
  BasicType type = x->basic_type();

  if (x->has_index()) {
    assert(x->index()->type() == intType, "should not find non-int index");
    log2_scale = x->log2_scale();
  }

  LIRItem obj(x->unsafe(), this);
  LIRItem dest(x->base(), this);
  LIRItem data(x->value(), this);
  LIRItem idx(x->index(), this);
  data.handle_float_kind();       // destroy the register on i486.
  
  if (x->needs_null_check()) {
    obj.load_item();
  } else {
    obj.dont_load_item();
  }
  dest.load_item();
  idx.load_item();

  if (type == T_BYTE || type == T_BOOLEAN) {
    data.load_byte_item();
  } else {
    data.load_item();
  }

  set_no_result(x);

  if (x->needs_null_check()) {
    CodeEmitInfo* info = state_for(x);
    emit()->null_check(obj.result(), info);
  }
  emit()->put_raw_unsafe(dest.result(), idx.result(), log2_scale, data.result(), x->basic_type());
}


void LIRGenerator::do_UnsafeGetObject(UnsafeGetObject* x) {
  BasicType type = x->basic_type();
  LIRItem obj(x->unsafe(), this);
  LIRItem src(x->object(), this);
  LIRItem off(x->offset(), this);

  if (x->needs_null_check()) {
    obj.load_item();
  } else {
    obj.dont_load_item();
  }
  off.load_item();
  src.load_item();

  LIR_Opr reg = reg = rlock_result(x, x->basic_type());

  if (x->needs_null_check()) {
    CodeEmitInfo* info = state_for(x);
    emit()->null_check(obj.result(), info);
  }
  if (x->is_volatile() && os::is_MP()) emit()->membar_acquire();
  emit()->get_Object_unsafe(reg->rinfo(), src.result(), off.result(), type, x->is_volatile());
  if (x->is_volatile() && os::is_MP()) emit()->membar();
}


void LIRGenerator::do_UnsafePutObject(UnsafePutObject* x) {
  BasicType type = x->basic_type();
  LIRItem obj(x->unsafe(), this);
  LIRItem src(x->object(), this);
  LIRItem off(x->offset(), this);
  LIRItem data(x->value(), this);
  data.handle_float_kind();   // destroy the register on i486.

  if (x->needs_null_check()) {
    obj.load_item();
  } else {
    obj.dont_load_item();
  }

  src.load_item();
  if (type == T_BOOLEAN || type == T_BYTE) {
    data.load_byte_item();
  } else {
    data.load_item();
  }  
  off.load_item();


  set_no_result(x);

  if (x->needs_null_check()) {
    CodeEmitInfo* info = state_for(x);
    emit()->null_check(obj.result(), info);
  }
  if (x->is_volatile() && os::is_MP()) emit()->membar_release();
  emit()->put_Object_unsafe(src.result(), off.result(), data.result(), type, x->is_volatile());

}


// do a jump to default successor, except if it is the immediate successor in _code (sequence of blocks)
void LIRGenerator::goto_default_successor(BlockEnd* block_end) {
  NEEDS_CLEANUP; // may need nops if this block_end is a safepoint instead of using the jump.
  bool must_be_safepoint = block_end->is_safepoint() && block_end->as_Goto() != NULL;
  if (!must_be_safepoint && block()->weight() + 1 == block_end->default_sux()->weight()) {
    // default successor is next block; no goto necessary
  } else {
    CodeEmitInfo* info = NULL;
    if (must_be_safepoint) {
      info =  new CodeEmitInfo(emit(), block_end->bci(), NULL, block_end->state_before() ? block_end->state_before() : block_end->state(), block_end->exception_scope());
    }
    emit()->goto_op(block_end->default_sux(), info);
  }
}


void LIRGenerator::do_TableSwitch(TableSwitch* x) {
  LIRItem tag(x->tag(), this);
  tag.load_item();
  set_no_result(x);

  if (x->is_safepoint()) {
    CodeEmitInfo* info_before = state_for(x, x->state_before());
    emit()->safepoint_nop(info_before);
  }
  setup_phis_for_switch(tag.result(), x->state());
  int lo_key = x->lo_key();
  int hi_key = x->hi_key();
  int len = x->length();
  CodeEmitInfo* info = state_for(x, x->state());
  for (int i = 0; i < len && !emit()->must_bailout(); i++) {
    int matchVal = i + lo_key;
    emit()->tableswitch_op(tag.result(), matchVal, x->sux_at(i));
  }
  emit()->goto_op(x->default_sux(), NULL);
}


// we expect the keys to be sorted by increasing value
static SwitchLookupRangeArray* create_lookup_ranges(LookupSwitch* x) {
  SwitchLookupRangeList* res = new SwitchLookupRangeList();
  int len = x->length();
  if (len > 0) {
    int key = x->key_at(0);
    BlockBegin* sux = x->sux_at(0);
    SwitchLookupRange* range = new SwitchLookupRange(key, sux);
    for (int i = 1; i < len; i++) {
      int new_key = x->key_at(i);
      BlockBegin* new_sux = x->sux_at(i);
      if (key+1 == new_key && sux == new_sux) {
        // still in same range
        range->set_high_key(new_key);
      } else {
        res->append(range);
        range = new SwitchLookupRange(new_key, new_sux);
      }
      key = new_key;
      sux = new_sux;
    }
    if (res->length() == 0 || res->last() != range)  res->append(range);
  }
  return res;
}


void LIRGenerator::do_LookupSwitch(LookupSwitch* x) {
  LIRItem tag(x->tag(), this);
  tag.load_item();
  set_no_result(x);
  if (x->is_safepoint()) {
    CodeEmitInfo* info_before = state_for(x, x->state_before());
    emit()->safepoint_nop(info_before);
  }
  setup_phis_for_switch(tag.result(), x->state());
  if (UseTableRanges) {
    SwitchLookupRangeArray* ranges = create_lookup_ranges(x);
    int lng = ranges->length();

    for (int i = 0; i < lng && !emit()->must_bailout(); i++) {
      SwitchLookupRange* one_range = ranges->at(i);
      emit()->lookupswitch_range_op(tag.result(), one_range->low_key(), one_range->high_key(), one_range->sux());
    }
    emit()->goto_op(x->default_sux(), NULL);
  } else {
    int len = x->length();
    for (int i = 0; i < len && !emit()->must_bailout(); i++) {
      emit()->lookupswitch_op(tag.result(), x->key_at(i), x->sux_at(i));
    }
    emit()->goto_op(x->default_sux(), NULL);
  }
}


void LIRGenerator::do_Goto(Goto* x) {
  set_no_result(x);
  move_to_phi(x->state());

  // check that this is endless goto (add nop)
  // Note: this is probably not needed as we have switched off EliminateJumpToJumps
  bool endless_loop = x->is_safepoint() && x->default_sux()->bci() == x->bci();
  if (endless_loop) {
    emit()->nop();
  }
  goto_default_successor(x);
}

#endif // PRODUCT
