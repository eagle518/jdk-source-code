#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CodeGenerator_i486.cpp	1.335 04/04/20 15:56:22 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_CodeGenerator_i486.cpp.incl"

#define __ emit()->lir()->

// Notes to OopMap generations:
// - because the pc-offset is attached to each OopMap, we must create
//   a new OopMap for each gc-point

//--------------------------------------------------------------
//               ValueGen
//--------------------------------------------------------------


RInfo ValueGen::exceptionOopRInfo()              { return FrameMap::_eaxRInfo;     }
RInfo ValueGen::exceptionPcRInfo()               { return FrameMap::_edxRInfo;     }
RInfo ValueGen::return1RInfo()                   { return FrameMap::_eaxRInfo;     }
RInfo ValueGen::callee_return1RInfo()            { return FrameMap::_eaxRInfo;     }
RInfo ValueGen::divInRInfo()                     { return FrameMap::_eaxRInfo;     }
RInfo ValueGen::divOutRInfo()                    { return FrameMap::_eaxRInfo;     }
RInfo ValueGen::remOutRInfo()                    { return FrameMap::_edxRInfo;     }
RInfo ValueGen::shiftCountRInfo()                { return FrameMap::_ecxRInfo;     }
RInfo ValueGen::return2RInfo()                   { return FrameMap::_eax_edxRInfo; } // for long result Lo:eax/Hi:edx
RInfo ValueGen::callee_return2RInfo()            { return FrameMap::_eax_edxRInfo; } // for long result Lo:eax/Hi:edx
RInfo ValueGen::returnF0RInfo()                  { return FrameMap::_f0RInfo;      }
RInfo ValueGen::returnD0RInfo()                  { return FrameMap::_d0RInfo;      }
RInfo ValueGen::receiverRInfo()                  { return FrameMap::_ecxRInfo;     }
RInfo ValueGen::syncTempRInfo()                  { return FrameMap::_eaxRInfo;     }
RInfo ValueGen::nonReturnRInfo()                 { return FrameMap::_esiRInfo;     }
RInfo ValueGen::icKlassRInfo()                   { return FrameMap::_eaxRInfo;     }

//-----------------------spilling--------------------------------------------------------


// returns true if reg could be smashed by a callee.
// If is_C_call, return true when register could be smashed by a C-call.
bool ValueGen::is_caller_save_register(RInfo reg) {
  return true; // all registers are caller saved on INTEL
}



//--------- loading items into registers --------------------------------


// For correct fpu stack simulation we must dereference
// an fpu register explictly, by copying it into a new register;
// in such case, item will be set to a new register; the reason is the simple 
// FPU stack emulation which requires that every floating point register is being
// popped.
void ValueGen::check_float_register(Item* item) {
  if (item->type()->is_float_kind()) {
    if (ra()->get_register_rc(item->get_register()) > 1) {
      // we need a new register to hold the value
      RInfo reg = lock_free_rinfo(item->value(), item->type());
      emit()->copy_fpu_item(reg, item2lir(item));

      item_free(item);
      item->set_register(reg);
      set_maynot_spill(item);
    }
  }
}


// Item will be loaded into a byte register; Intel only
void ValueGen::load_byte_item(Item* item) {
  item->update();
  set_maynot_spill(item);
  debug_only(check_item(item);)
  if (item->is_register() && FrameMap::is_byte_rinfo(item->get_register()) && !must_copy_register(item)) {
    // it is a byte register: we are fine
  } else {
    // make sure that it is a byte register
    assert(!item->type()->is_float() && !item->type()->is_double() , "canot load floats in byte register");
    RInfo reg = lock_free_rinfo(item->value(), FrameMap::byte_reg_mask());
    emit()->move(item2lir(item), reg);

    item_free(item);
    item->set_register(reg);
    set_maynot_spill(item);
  }
}


RInfo ValueGen::rlock_byte_result_with_hint(Value instr, const Item* hint) {
  RInfo reg = norinfo;
  if (hint != NULL &&
      hint->has_hint() &&
      ra()->is_free_reg(hint->get_register()) &&
      FrameMap::byte_reg_mask().contains(hint->get_register())) {
    reg = hint->get_register();
    ra()->lock_register(instr, reg);
  } else {
    // no hint or hint register is locked
    while (!ra()->has_free_reg(FrameMap::byte_reg_mask())) {
      spill_one(FrameMap::byte_reg_mask());
    }
    reg = ra()->get_lock_reg(instr, FrameMap::byte_reg_mask());
  }
  set_result(instr, reg);
  return reg;
}


// Handle fanout of fpu registers: return false if no fanout;
// If fanout than, we copy the value of float register into a new one,
// so that the new FPU register has ref-count 1
bool ValueGen::fpu_fanout_handled(Item* item) {
  if (item->is_register() && (item->type()->is_float_kind()) &&  (ra()->get_register_rc(item->get_register()) > 1)) {
    // The item is float or double register with a use_count > 1
    RInfo reg = lock_free_rinfo(item->value(), item->type());
    emit()->copy_fpu_item(reg, item2lir(item));

    item_free(item);
    item->set_register(reg);
    return true;
  } else {
    return false;
  }
}

// i486 instructions can inline constants
bool ValueGen::can_inline_any_constant() const {
  return true;
}


// all constants can be inlined
bool ValueGen:: can_inline_as_constant(Item* i) {
  return true;
}


bool ValueGen::prefer_alu_registers() const {
  return true;
}


bool ValueGen::safepoint_poll_needs_register() {
  return false;
}


RInfo ValueGen::scratch1_RInfo() const {
  return norinfo; // no scratch registers on i486
}


//----------------------------------------------------------------------
//             visitor functions
//----------------------------------------------------------------------


void ValueGen::do_StoreIndexed(StoreIndexed* x) {
  assert(x->is_root(),"");
  bool obj_store = x->elt_type() == T_ARRAY || x->elt_type() == T_OBJECT;
  bool use_length = x->length() != NULL;
  Item  array(x->array());
  if (obj_store) {
    array.set_destroys_register();
  }
  Item  index(x->index());
  Item  value(x->value());
  value.handle_float_kind();
  Item  length;
  if (use_length) {
    length.set_instruction(x->length());
  }
  ValueGen a(&array, HintItem::no_hint(), this);
  ValueGen i(&index, HintItem::no_hint(), this);
  ValueGen l(&length, HintItem::no_hint(), this, true);
  // for T_BYTE element type, we must have a byte register free
  ValueGen v(&value, HintItem::no_hint(), this);
  load_item(&array);
  if (!index.is_constant()) {
    load_item(&index);
  }
 
  if (use_length) {
    load_item(&length);
    item_free(&length);
  }

  // debug info for exception throwing or handling has no expression stack, thus no oops-in-spill
  CodeEmitInfo* range_check_info = new CodeEmitInfo(emit(), x->bci(), NULL, x->lock_stack(), x->exception_scope());
  CodeEmitInfo* null_check_info = NULL;
  if (x->needs_null_check()) {
    null_check_info = range_check_info;
  }
  if (GenerateRangeChecks) {
    if (use_length) {
      emit()->length_range_check(item2lir(&length), item2lir(&index), range_check_info);
      assert(null_check_info == NULL, "should already have been done");
    } else {
      emit()->array_range_check(item2lir(&array), item2lir(&index), null_check_info, range_check_info);
      // range_check also does the null check, so NULL it out before passing to indexed_store
      null_check_info = NULL;
    }
  }
  bool must_load = true;
  if (x->elt_type() == T_SHORT || x->elt_type() == T_CHAR) {
    // there is no immediate move of word values in asembler_i486.?pp
    must_load = true;
  } else {
    if (obj_store && value.is_constant() && !value.get_jobject_constant()->is_loaded()) {
      // do nothing; do not load (NULL object)
      must_load = false;
    } else if (value.is_constant() && !obj_store) {
      // array store check needs a register, otherwise do not load a constant
      must_load = false;
    }
  }
  if (must_load) {
    if (x->elt_type() == T_BYTE || x->elt_type() == T_BOOLEAN) {
      load_byte_item(&value);
    } else {
      load_item(&value);
    }
  }

  set_no_result(x);

  if (obj_store) {
    if (value.is_constant() && !value.get_jobject_constant()->is_loaded()) {
      // skip store check
    } else {
      HideReg hr1(this, objectType);
      HideReg hr2(this, objectType);
      HideReg hr3(this, objectType);
      RInfo tmp1 = hr1.reg();
      RInfo tmp2 = hr2.reg();
      RInfo tmp3 = hr3.reg();

      assert(!tmp1.is_same(tmp2) && !tmp2.is_same(tmp3) && !tmp3.is_same(tmp1), "wrong allocation");
      emit()->array_store_check(item2lir(&array), item2lir(&value), tmp1, tmp2, tmp3, range_check_info);
    }
  }
  emit()->indexed_store(x->elt_type(), item2lir(&array), item2lir(&index), item2lir(&value), norinfo, null_check_info);  

  // release items after the CodeEmitInfo has been emitted, otherwise the items may be set to invalid
  item_free(&index);
  item_free(&value);
  item_free(&array);
}


void ValueGen::do_MonitorEnter(MonitorEnter* x) {
  spill_values_on_stack(x->state());
  lock_spill_temp(NULL, syncTempRInfo());
  assert(x->is_root(),"");
  Item obj(x->obj());
  ValueGen o(&obj, HintItem::no_hint(), this);
  load_item(&obj);
  RInfo lock = lock_free_rinfo(NULL, intType);
  ra()->free_reg(lock);
  item_free(&obj);
  ra()->free_reg(syncTempRInfo());
  assert(ra()->are_all_registers_free(), "slow case does not preserve registers");
  set_no_result(x);

  CodeEmitInfo* info_for_exception = NULL;
  if (x->needs_null_check()) {
    info_for_exception = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack_before(), x->exception_scope());
  }
  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope());
  emit()->monitor_enter(obj.get_register(), lock, syncTempRInfo(), norinfo, x->monitor_no(), info_for_exception, info);
}


void ValueGen::do_MonitorExit(MonitorExit* x) {
  spill_values_on_stack(x->state());
  lock_spill_temp(NULL, syncTempRInfo());
  assert(x->is_root(),"");

  Item obj(x->obj());
  ValueGen o(&obj, HintItem::no_hint(), this);
  dont_load_item(&obj);
  item_free(&obj);

  RInfo lock = lock_free_rinfo(NULL, intType);
  RInfo obj_temp = lock_free_rinfo(NULL, intType);
  ra()->free_reg(lock);
  ra()->free_reg(obj_temp);
  ra()->free_reg(syncTempRInfo());
  assert(ra()->are_all_registers_free(), "slow case does not preserve registers");
  set_no_result(x);
  emit()->monitor_exit(obj_temp, lock, syncTempRInfo(), x->monitor_no());
}


// returns true if either strictFP is true or 32-bit rounding precision is
// required.
// Note: this leads to excessive spilling in strictFP mode; it could be solved
// with an extension to items
bool ValueGen::must_round(Value x, const Item* hint) {
  if (!RoundFloatsWithStore) return false;
  assert(x->as_Intrinsic() != NULL || x->as_NegateOp() != NULL || x->as_ArithmeticOp() != NULL || x->as_Convert() != NULL, "wrong operation");
  if (method()->is_strict()) return true;
  if (x->type()->is_float()) {
    if (x->is_root() || x->use_count() > 1 || (hint->is_round32() && !ra()->is_32bit_precision()) ) {
      return !is_32bit_mode();
    }
  }
  return false;
}


bool ValueGen::is_32bit_mode() {
  return is_single_precision();
}


// _ineg, _lneg, _fneg, _dneg 
void ValueGen::do_NegateOp(NegateOp* x) {
  Item value(x->x());
  value.set_destroys_register();
  ValueGen v(&value, hint(), this);
  load_item_hint(&value, hint());
  item_free(&value);
  RInfo reg = rlock(x, hint());
  set_result(x, reg);
  emit()->negate(reg, item2lir(&value));

  if (must_round(x, hint())) {
    round_item(_result);
    assert(_result->is_spilled(), "spilling failed");
  }
}



// for  _fadd, _fmul, _fsub, _fdiv, _frem
//      _dadd, _dmul, _dsub, _ddiv, _drem
void ValueGen::do_ArithmeticOp_FPU(ArithmeticOp* x) {
  Item left_item(x->x());
  Item right_item(x->y());
  // Because of the way we simulate FPU  regsiter location in code emitter, we must
  // mark both items as being destroyed(i.e., they are going to be removed from the FPU stack)
  right_item.set_destroys_register();
  left_item.set_destroys_register();
  // missing test if instr is commutative and if we should swap
  HintItem left_hint(x->x()->type()); left_hint.set_from_item(hint());
  HintItem right_hint(x->y()->type()); right_hint.set_from_item(HintItem::no_hint());
  if (x->type()->is_float()) {
    left_hint.set_round32();
    right_hint.set_round32();
  }
  ValueGen l(&left_item,  &left_hint, this);
  ValueGen r(&right_item, &right_hint, this);
  Item* left_arg  = &left_item;
  Item* right_arg = &right_item;
  if (x->is_commutative() && left_item.is_stack() && right_item.is_register()) {
      // swap them if left is real stack (or cached) and right is real register(not cached)
    left_arg = &right_item;
    right_arg = &left_item;
  }
  load_item_hint(left_arg, &left_hint);
  // do not need to load right, as we can handle stack and constants
  if ( x->y()->type()->is_constant() || x->op() == Bytecodes::_frem || x->op() == Bytecodes::_drem) {
    // cannot handle inlined constants
    load_item(right_arg);
  } else {
    dont_load_item(right_arg);
  }
  item_free(left_arg);
  item_free(right_arg);
  RInfo reg = rlock(x, hint());
  set_result(x, reg);
  emit()->arithmetic_op_fpu(x->op(), item2lir(_result), item2lir(left_arg), item2lir(right_arg), x->is_strictfp());

  if (must_round(x, hint())) {
    round_item(_result);
    assert(_result->is_spilled(), "spilling failed");
  }
}



// for  _ladd, _lmul, _lsub, _ldiv, _lrem
void ValueGen::do_ArithmeticOp_Long(ArithmeticOp* x) {
  if (x->op() == Bytecodes::_ldiv || x->op() == Bytecodes::_lrem ) {
    Item left(x->x());
    Item right(x->y());
    
    RInfo first_arg = as_RInfo(eax, edx);;
    RInfo second_arg = as_RInfo(ebx, ecx);;
    
    HintItem left_hint(x->x()->type(), first_arg);
    HintItem right_hint(x->x()->type(), second_arg);
    ValueGen l(&left,  &left_hint, this);
    ValueGen r(&right, &right_hint, this);
    
    right.set_destroys_register();
    left.set_destroys_register();

    load_item_force(&left, first_arg);
    load_item_force(&right, second_arg);
    item_free(&left);
    item_free(&right);
    
    RInfo reg = result_register_for(x->type());
    lock_spill_rinfo(x, reg);
    set_result(x, reg);
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
    emit()->arithmetic_op_long(x->op(), LIR_OprFact::rinfo(reg), LIR_OprFact::rinfo(first_arg, T_LONG), LIR_OprFact::rinfo(second_arg, T_LONG), info);
  } else if (x->op() == Bytecodes::_lmul) {
    Item left(x->x());
    Item right(x->y());
    left.set_destroys_register();
    right.set_destroys_register();
    // missing test if instr is commutative and if we should swap
    ValueGen l(&left, HintItem::no_hint(), this);
    ValueGen r(&right, HintItem::no_hint(), this);
    load_item_hint(&left, hint());
    load_item(&right);

    // WARNING: for longs, we must guarantee (algorithmically) that the locked lo result
    //          register is not the same as the left-HI register , otherwise we 
    //          would overwrite the results.
    // 
    item_free(&left);
    item_free(&right);
    lock_spill_rinfo(x, FrameMap::_eax_edxRInfo);
    set_result(x, FrameMap::_eax_edxRInfo);
    emit()->arithmetic_op_long(x->op(), item2lir(_result), item2lir(&left), item2lir(&right));
  } else {
    Item left(x->x());
    Item right(x->y());
    left.set_destroys_register();
    // missing test if instr is commutative and if we should swap
    ValueGen l(&left, HintItem::no_hint(), this);
    ValueGen r(&right, HintItem::no_hint(), this);
    load_item_hint(&left, hint());
    load_item(&right);
    // WARNING: for longs, we must guarantee (algorithmically) that the locked lo result
    //          register is not the same as the left-HI register , otherwise we 
    //          would overwrite the results.
    // 
    item_free(&left);
    item_free(&right);
    RInfo reg = rlock(x, hint());
    set_result(x, reg);
    emit()->arithmetic_op_long(x->op(), item2lir(_result), item2lir(&left), item2lir(&right));
  }
}



// for: _iadd, _imul, _isub, _idiv, _irem
void ValueGen::do_ArithmeticOp_Int(ArithmeticOp* x) {
  if (x->op() == Bytecodes::_idiv || x->op() == Bytecodes::_irem) {
    // The requirements for division and modulo
    // input : eax: dividend                         min_int
    //         reg: divisor   (may not be eax/edx)   -1
    //
    // output: eax: quotient  (= eax idiv reg)       min_int
    //         edx: remainder (= eax irem reg)       0

    // eax and edx will be destroyed

    Item left(x->x());
    Item right(x->y());
    left.set_destroys_register();
    right.set_destroys_register();
    HintItem hint(x->type(), divInRInfo()); // eax
    // Note: does this invalidate the spec ???
    ValueGen r(&right, HintItem::no_hint(), this);
    ValueGen l(&left , &hint,       this);   // visit left second, so that the is_register test is valid
    
    // By forcing left in eax and right in edx, we guarantee that the
    // registers are available
    load_item_force(&left, hint.get_register()); // load "left" into eax 

    if (right.is_constant() && is_power_of_2(right.get_jint_constant()) && right.get_jint_constant() > 0) {
      // this only handles positive, non-zero numbers.  negative powers of 2
      // would be easy to do but is uncommon
      dont_load_item(&right);
      item_free(&left);
      item_free(&right);
      RInfo tmp = norinfo;
      if (x->op() == Bytecodes::_idiv) {
        // idiv uses edx in it's implemenation, so make sure it's free
        // at this point by forcing it to be spilled if it isn't free.
        HideReg hr(this, FrameMap::_edxRInfo, true);
        tmp = FrameMap::_edxRInfo;
      }
      rlock_result_with_hint(x, this->hint());
      emit()->arithmetic_idiv(x->op(), item2lir(_result), item2lir(&left), item2lir(&right), tmp, NULL);
    } else {

      { HideReg hr(this, remOutRInfo(), true); // needed for computation
        load_item(&right);
        item_free(&left);
        item_free(&right);
      }
    
      if (x->op() == Bytecodes::_idiv) {
        lock_spill_rinfo(x, divOutRInfo());
        set_result(x, divOutRInfo());
      } else {
        lock_spill_rinfo(x, remOutRInfo());
        set_result(x, remOutRInfo());
      }
      if (!ImplicitDiv0Checks) {
        CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
        emit()->explicit_div_by_zero_check(item2lir(&right), info);
      }
      CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope());
      RInfo tmp = FrameMap::_edxRInfo; // idiv and irem use edx in their implementation
      emit()->arithmetic_idiv(x->op(), item2lir(_result), item2lir(&left), item2lir(&right), tmp, info);
    }
  } else {
    Item left_item(x->x());
    Item right_item(x->y());
    // missing test if instr is commutative and if we should swap
    ValueGen l(&left_item, hint(), this);
    ValueGen r(&right_item, HintItem::no_hint(), this);
    Item* left_arg = &left_item;
    Item* right_arg = &right_item;
    if (x->is_commutative() && left_item.is_stack() && right_item.is_register()) {
      // swap them if left is real stack (or cached) and right is real register(not cached)
      left_arg = &right_item;
      right_arg = &left_item;
    }
    left_arg->set_destroys_register();
    load_item_hint(left_arg, hint());
    // do not need to load right, as we can handle stack and constants
    if (x->op() == Bytecodes::_imul ) {
      // check if we can use shift instead
      RInfo tmp;
      bool use_constant = false;
      bool use_tmp = false;
      if (right_arg->is_constant()) {
        int iconst = right_arg->get_jint_constant();
        if (iconst > 0) {
          if (is_power_of_2(iconst)) {
            use_constant = true;
          } else if (is_power_of_2(iconst - 1) || is_power_of_2(iconst + 1)) {
            use_constant = true;
            use_tmp = true;
          }
        }
      }
      if (use_constant) {
        dont_load_item(right_arg);
      } else {
        load_item(right_arg);
      }
      if (use_tmp) {
        tmp = lock_free_rinfo(NULL, intType);
        ra()->free_reg(tmp);
      }
      item_free(left_arg);
      item_free(right_arg);
      RInfo reg = rlock(x, hint());
      set_result(x, reg);

      emit()->arithmetic_op_int(x->op(), item2lir(_result), item2lir(left_arg), item2lir(right_arg), tmp);
    } else {
      dont_load_item(right_arg);
      item_free(left_arg);
      item_free(right_arg);
      RInfo reg = rlock(x, hint());
      set_result(x, reg);

      RInfo tmp;
      emit()->arithmetic_op_int(x->op(), item2lir(_result), item2lir(left_arg), item2lir(right_arg), tmp);
    }
  }
}


void ValueGen::do_ArithmeticOp(ArithmeticOp* x) {
  ValueTag tag = x->type()->tag();
  assert(x->x()->type()->tag() == tag && x->y()->type()->tag() == tag, "wrong parameters");
  switch (tag) {
    case floatTag:
    case doubleTag:  do_ArithmeticOp_FPU(x);  return;
    case longTag:    do_ArithmeticOp_Long(x); return;
    case intTag:     do_ArithmeticOp_Int(x);  return;
  }
  ShouldNotReachHere();
}


// _ishl, _lshl, _ishr, _lshr, _iushr, _lushr
void ValueGen::do_ShiftOp(ShiftOp* x) {
  Item value(x->x());
  Item count(x->y());
  value.set_destroys_register();
  HintItem count_hint(x->y()->type(), shiftCountRInfo());
  // count must always be in ecx
  ValueGen v(&value, hint(),    this);
  ValueGen c(&count, &count_hint, this);
  ValueTag elemType = x->type()->tag();
  bool must_load_count = !count.is_constant() || elemType == longTag;
  if (must_load_count) {
    // count for long must be in register
    load_item_hint(&count, &count_hint);
  } else {
    dont_load_item(&count);
  }
  load_item_hint(&value, hint());
  RInfo tmp;
  if (elemType == intTag && count.is_register()) {
    // in case we cache the count, we want always to have a register free
    // (without caching, the count may be loaded in ecx; with the caching count,
    // the count register may not be ecx
    tmp = lock_free_rinfo(NULL, intType);
    ra()->free_reg(tmp);
  }
  item_free(&count);
  item_free(&value);
  RInfo reg = rlock(x, &value);
  set_result(x, reg);

  emit()->shift_op(x->op(), reg, item2lir(&value), item2lir(&count), tmp);
}


// _iand, _land, _ior, _lor, _ixor, _lxor
void ValueGen::do_LogicOp(LogicOp* x) {
  Item left(x->x());
  Item right(x->y());
  left.set_destroys_register();
  // missing test if instr is commutative and if we should swap
  ValueGen l(&left, hint(), this);
  ValueGen r(&right, HintItem::no_hint(), this);
  load_item_hint (&left, hint());
  if (!right.is_constant()) {
    load_item(&right);
  } else {
    dont_load_item(&right);
  }
  item_free(&left);
  if (!right.is_constant()) {
    item_free(&right);
  }
  RInfo reg = rlock(x, hint());
  set_result(x, reg);

  emit()->logic_op(x->op(), reg, item2lir(&left), item2lir(&right));
}



// _lcmp, _fcmpl, _fcmpg, _dcmpl, _dcmpg
void ValueGen::do_CompareOp(CompareOp* x) {
  Item left(x->x());
  Item right(x->y());
  ValueTag tag = x->x()->type()->tag();
  if (tag == floatTag || tag == doubleTag) {
    left.set_destroys_register();
    right.set_destroys_register();
    // missing test if instr is commutative and if we should swap
    HintItem left_hint(x->x()->type()); left_hint.set_from_item(HintItem::no_hint());
    HintItem right_hint(x->y()->type()); right_hint.set_from_item(HintItem::no_hint());
    if (tag == floatTag) {
      left_hint.set_round32();
      right_hint.set_round32();
    }
    ValueGen l(&left,  &left_hint,  this);
    ValueGen r(&right, &right_hint, this);
    load_item(&left);
    load_item(&right);
  } else {
    if (tag == longTag) {
      left.set_destroys_register();
    }
    // NOTE: missing test if instr is commutative and if we should swap (optimization)
    ValueGen l(&left,  HintItem::no_hint(), this);
    ValueGen r(&right, HintItem::no_hint(), this);
    load_item(&left);
    load_item(&right);
  }
  item_free(&left);
  item_free(&right);
  RInfo reg = rlock(x, hint());
  set_result(x, reg);
  emit()->compare_op(x->op(), reg, item2lir(&left), item2lir(&right));
}


// Code for  :  x->x() {x->cond()} x->y() ? x->tval() : x->fval()
void ValueGen::do_IfOp(IfOp* x) {
  Item left(x->x());
  Item right(x->y());
  { ValueTag tag = x->x()->type()->tag();
    assert(tag == intTag || tag == objectTag, "cannot handle others");
  }
  { ValueTag tag = x->tval()->type()->tag();
    assert(tag == addressTag || tag == intTag || tag == objectTag || tag == longTag, "cannot handle others");
    assert(x->tval()->type()->tag() == x->fval()->type()->tag(), "cannot handle others");
  }
  HintItem left_hint(x->x()->type());  left_hint.set_from_item (HintItem::no_hint());
  HintItem right_hint(x->y()->type()); right_hint.set_from_item(HintItem::no_hint());
  if (x->x()->type()->is_float()) {
    left_hint.set_round32();
    right_hint.set_round32();
  }

  ValueGen l(&left,  &left_hint, this);
  ValueGen r(&right, &right_hint, this);
  load_item(&left);
  load_item(&right);
  item_free(&left);
  item_free(&right);

  emit()->ifop_phase1(x->cond(), item2lir(&left), item2lir(&right));

  Item t_val(x->tval());
  Item f_val(x->fval());
  // only one of the values will be taken
  HintItem tval_hint(x->tval()->type());
  if (x->tval()->as_LoadLocal() != NULL || x->tval()->as_Constant() != NULL) {
    tval_hint.set_from_item(hint());
  } else {
    tval_hint.set_from_item(HintItem::no_hint());
  }
  HintItem fval_hint(x->fval()->type());
  if (x->fval()->as_LoadLocal() != NULL || x->fval()->as_Constant() != NULL) {
    fval_hint.set_from_item(hint());
  } else {
    fval_hint.set_from_item(HintItem::no_hint());
  }
  ValueGen tv(&t_val, &tval_hint, this);  // use hint() if we make sure that input is local or constant
  ValueGen fv(&f_val, &fval_hint, this);  // use hint() if we make sure that input is local or constant
  dont_load_item(&t_val);
  dont_load_item(&f_val);
  RInfo reg;
  if (x->fval()->type()->tag() == longTag) {
    // must lock before releasing
    reg = rlock(x, hint());
    set_result(x, reg);
  }
  item_free(&t_val);
  item_free(&f_val);
  if (x->fval()->type()->tag() != longTag) {
    reg = rlock(x, hint());
    set_result(x, reg);
  }

  emit()->ifop_phase2(reg, item2lir(&t_val), item2lir(&f_val), x->cond());
}


void ValueGen::do_MathIntrinsic(Intrinsic* x) {
  assert(x->number_of_arguments() == 1, "wrong type");
  Item value(x->argument_at(0));
  value.set_destroys_register();
  ValueGen v(&value, hint(), this);
  load_item_hint(&value, hint());
  item_free(&value);
  RInfo reg = rlock(x, hint());
  set_result(x, reg);

  emit()->math_intrinsic(x->id(), reg, item2lir(&value));

  if (must_round(x, hint())) {
    round_item(_result);
    assert(_result->is_spilled(), "spilling failed");
  }
}


void ValueGen::do_ArrayCopy(Intrinsic* x) {
  spill_values_on_stack(x->state());
  assert(x->number_of_arguments() == 5, "wrong type");
  Item src     (x->argument_at(0));
  Item src_pos (x->argument_at(1));
  Item dst     (x->argument_at(2));
  Item dst_pos (x->argument_at(3));
  Item length  (x->argument_at(4));
  ValueGen src_v(&src, HintItem::no_hint(), this);
  ValueGen src_pos_v(&src_pos, HintItem::no_hint(), this);
  ValueGen dst_v(&dst, HintItem::no_hint(), this);
  ValueGen dst_pos_v(&dst_pos, HintItem::no_hint(), this);
  ValueGen length_v(&length, HintItem::no_hint(), this);

  load_item(&src);
  load_item(&src_pos); 
  load_item(&dst);
  load_item(&dst_pos);
  load_item(&length);
  RInfo tmp = lock_free_rinfo(NULL, intType);
  ra()->free_reg(tmp);
  item_free(&src);
  item_free(&src_pos);
  item_free(&dst);
  item_free(&dst_pos);
  item_free(&length);
  set_no_result(x);

  assert(ra()->are_all_registers_free(), "all registers must be free across arraycopy");

  int flags;
  ciArrayKlass* expected_type;
  arraycopy_helper(x, &flags, &expected_type);

  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope()); // we may want to have stack (deoptimization?)
  __ arraycopy(item2lir(&src),
               item2lir(&src_pos),
               item2lir(&dst),
               item2lir(&dst_pos),
               item2lir(&length), 
               LIR_OprFact::rinfo(tmp, T_ADDRESS),
               expected_type, flags, info);

}


void ValueGen::do_Intrinsic(Intrinsic* x) {
  switch (x->id()) {
  case ciMethod::_intBitsToFloat      :
  case ciMethod::_doubleToRawLongBits :
  case ciMethod::_longBitsToDouble    :
  case ciMethod::_floatToRawIntBits   : {
    do_FPIntrinsics(x);
    break;
  }

  case methodOopDesc::_currentTimeMillis: {
    assert(x->number_of_arguments() == 0, "wrong type");
    spill_values_on_stack(x->state());
    set_with_result_register(x);
    emit()->lir()->call_runtime_leaf(CAST_FROM_FN_PTR(address, os::javaTimeMillis), norinfo, 0, 0);
    break;
  }

  case methodOopDesc::_nanoTime: {
    assert(x->number_of_arguments() == 0, "wrong type");
    spill_values_on_stack(x->state());
    set_with_result_register(x);
    emit()->lir()->call_runtime_leaf(CAST_FROM_FN_PTR(address, os::javaTimeNanos), norinfo, 0, 0);
    break;
  }

  case methodOopDesc::_getClass:       do_getClass(x);      break;
  case methodOopDesc::_currentThread:  do_currentThread(x); break;

  case methodOopDesc::_dsqrt:          // fall through
  case methodOopDesc::_dsin :          // fall through
  case methodOopDesc::_dcos :          do_MathIntrinsic(x); break;
  case methodOopDesc::_arraycopy:      do_ArrayCopy(x);     break;

  // java.nio.Buffer.checkIndex
  case methodOopDesc::_checkIndex:     do_NIOCheckIndex(x); break;
  case methodOopDesc::_compareAndSwapObject_obj: 
    do_CompareAndSwap(x, objectType); 
    break;
  case methodOopDesc::_compareAndSwapInt_obj: 
    do_CompareAndSwap(x, intType); 
    break;
  case methodOopDesc::_compareAndSwapLong_obj: 
    do_CompareAndSwap(x, longType); 
    break;

    // sun.misc.AtomicLongCSImpl.attemptUpdate
  case methodOopDesc::_attemptUpdate: {
    assert(x->number_of_arguments() == 3, "wrong type");
    Item obj       (x->argument_at(0));  // AtomicLong object
    Item cmp_value (x->argument_at(1));  // value to compare with field
    Item new_value (x->argument_at(2));  // replace field with new_value if it matches cmp_value

    // compare value must be in edx,eax (hi,lo); may be destroyed by cmpxchg8 instruction
    cmp_value.set_destroys_register();
    RInfo cmp_value_rinfo = as_RInfo(eax, edx);
    HintItem cmp_value_hint(longType, cmp_value_rinfo);
    ValueGen cmp_value_v(&cmp_value, &cmp_value_hint, this);
    load_item_force(&cmp_value, cmp_value_rinfo);

    // new value must be in ecx,ebx (hi,lo)
    RInfo new_value_rinfo = as_RInfo(ebx, ecx);
    HintItem new_value_hint(longType, new_value_rinfo);
    ValueGen new_value_v(&new_value, &new_value_hint, this);
    load_item_force(&new_value, new_value_rinfo);

    // object pointer register is overwritten with field address
    obj.set_destroys_register();
    ValueGen obj_v(&obj, HintItem::no_hint(), this);
    load_item(&obj);
 
    item_free(&obj);
    item_free(&cmp_value);
    item_free(&new_value);

    // generate compare-and-swap; produces zero condition if swap occurs
    int value_offset = sun_misc_AtomicLongCSImpl::value_offset();
    LIR_Opr addr = item2lir(&obj);
    emit()->lir()->add(addr, LIR_OprFact::intConst(value_offset), addr);
    LIR_Opr t1 = LIR_OprFact::illegalOpr;  // no temp needed
    LIR_Opr t2 = LIR_OprFact::illegalOpr;  // no temp needed
    emit()->lir()->cas_long(addr, item2lir(&cmp_value), item2lir(&new_value), t1, t2);

    // generate conditional move of boolean result
    RInfo result = rlock_result_with_hint(x, hint());
    LabelObj* L = new LabelObj();
    emit()->move(LIR_OprFact::intConst(1), result);
    emit()->lir()->branch(LIR_OpBranch::equal, L->label());
    emit()->move(LIR_OprFact::intConst(0), result);
    emit()->lir()->branch_destination(L->label());
    break;
  }

  default: ShouldNotReachHere();
  }
}


// _i2l, _i2f, _i2d, _l2i, _l2f, _l2d, _f2i, _f2l, _f2d, _d2i, _d2l, _d2f
// _i2b, _i2c, _i2s
void ValueGen::do_Convert(Convert* x) {
  HintItem value_hint(x->value()->type()); value_hint.set_from_item(HintItem::no_hint());
  if (x->value()->type()->is_float()) {
    value_hint.set_round32();
  }
  if (x->op() == Bytecodes::_f2i || x->op() == Bytecodes::_d2i) {
    Item value(x->value());
    ValueGen v(&value, &value_hint, this);
    value.set_destroys_register();
    load_item(&value);
    item_free(&value);
    RInfo reg = result_register_for(x->type());
    lock_spill_rinfo(x, reg);
    set_result(x, reg);

    emit()->convert_op(x->op(), item2lir(&value), reg, is_32bit_mode());
  } else {
    Item value(x->value());
    value.handle_float_kind();
    ValueGen v(&value, &value_hint, this);
    value.handle_float_kind();
    if (value.is_constant()) {
      load_item(&value);
    } else if (x->op() != Bytecodes::_i2f && x->op() != Bytecodes::_i2d && x->op() != Bytecodes::_l2f && x->op() != Bytecodes::_l2d) {
      load_item(&value);
    } else {
      dont_load_item(&value);
    }
    item_free(&value);
    RInfo reg;
    if (x->op() == Bytecodes::_f2l || x->op() == Bytecodes::_d2l) {
      reg = FrameMap::_eax_edxRInfo;
      lock_spill_rinfo(x, reg);
    } else {
      reg = rlock(x);
    }
    set_result(x, reg);

    emit()->convert_op(x->op(), item2lir(&value), reg);
    if ( x->op() == Bytecodes::_d2f || must_round(x, hint())) {
      // verify that we do not need to round when going from double to float
      spill_item(_result);
      assert(_result->is_spilled(), "spilling failed");
    }
  }
}


void ValueGen::do_CachingChange(CachingChange* x) {
  if (x->sux_block()->is_set(BlockBegin::single_precision_flag) != x->pred_block()->is_set(BlockBegin::single_precision_flag)) {
    if (x->sux_block()->is_set(BlockBegin::single_precision_flag)) {
      emit()->set_24bit_fpu_precision();
    } else {
      emit()->restore_fpu_precision();
    }
  }
}


// handle arguments, without receiver
void ValueGen::invoke_do_arguments(Invoke* x) {
  // handle arguments if any
  int offset_from_esp_in_words = x->size_of_arguments() - 1;
  for (int i = 0; i < x->number_of_arguments(); i++) {
    Item param(x->argument_at(i));
    param.handle_float_kind();
    ValueGen p(&param, HintItem::no_hint(), this);
    load_item(&param);

    emit()->store_stack_parameter(item2lir(&param), offset_from_esp_in_words);

    item_free(&param);
    offset_from_esp_in_words -= param.type()->size();
  }
}


// Visits all arguments, returns appropriate items without loading them
ItemArray* ValueGen::invoke_visit_arguments(Invoke* x, CallingConvention* unused) {
  ItemArray* argument_items = new ItemArray(x->number_of_arguments());
  for (int i = 0; i < x->number_of_arguments(); i++) {
    Item* param = new Item(x->argument_at(i));
    (*argument_items)[i] = param;
    ValueGen p(param, HintItem::no_hint(), this);
  }
  return argument_items;
}


void ValueGen::invoke_load_arguments(Invoke* x, ItemArray* args, CallingConvention* unused) {
  // we need to lock a register so that we can transport values on stack
  int offset_from_esp_in_words = x->size_of_arguments() - 1;
  if (x->has_receiver()) {
    --offset_from_esp_in_words; // the receiver has been pushed already
  }
  for (int i = 0; i < x->number_of_arguments(); i++) {
    Item* param = args->at(i);
    param->handle_float_kind();
    load_item(param);

    emit()->store_stack_parameter(item2lir(param), offset_from_esp_in_words);

    item_free(param);
    offset_from_esp_in_words -= param->type()->size();
  }
}


void ValueGen::invoke_do_spill(Invoke* x, RInfo hide_reg) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
}


void ValueGen::invoke_do_result(Invoke* x, bool needs_null_check, Item* receiver) {
  assert(ra()->are_all_registers_free(), "all registers must be free across calls");
  // setup result register
  if (x->type()->is_void()) {
    set_no_result(x);
  } else {
    RInfo reg = result_register_for(x->type());
    lock_spill_rinfo(x, reg);
    set_result(x, reg);
  }
  // emit invoke code
  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope());
  bool optimized = x->target_is_loaded() && x->target_is_final();
  // The current backend doesn't support vtable calls, so pass -1
  // instead of x->vtable_index();
  emit()->call_op(x->code(), NULL, info, -1, optimized, needs_null_check, receiverRInfo(), item2lir(_result)); // does add_safepoint
  if (x->type()->is_float() || x->type()->is_double()) {
    emit()->set_fpu_result(_result->get_register());
    // Force rounding of results from non-strictfp when in strictfp
    // scope (or when we don't know the strictness of the callee, to
    // be safe.)
    if (method()->is_strict()) {
      if (!x->target_is_loaded() || !x->target_is_strictfp()) {
        round_item(result());
      }
    }
  }
}


void ValueGen::do_Invoke(Invoke* x) {
  // We want to spill roots only late so that any roots with use count > 1, may have
  // its registers used before being spilled
  // invoke_do_spill(x);
  if (x->has_receiver()) {
    bool needs_null_check = false;
    HintItem hint(x->receiver()->type(), receiverRInfo());
    Item receiver(x->receiver());
    // make sure that ref count of register holding the receiver has
    // use count 1 by marking receiver as register-destroying
    receiver.set_destroys_register(); 
    ValueGen r(&receiver, &hint, this);
    // visit first all arguments, so that the register containing receiver (and is going to be
    // hidden) will certainly not be needed; we hide the receiver with NULL instruction, and
    // therefore we cannot spill it if it is needed by somebody else
    ItemArray* args = invoke_visit_arguments(x, NULL);
    load_item_hint(&receiver, &hint);
    item_free(&receiver);

    emit()->store_stack_parameter(item2lir(&receiver), x->size_of_arguments() - 1);
    if (x->needs_null_check() ||
        !x->target_is_loaded() || x->target_is_final()) {
      // emit receiver NULL check if the class of target is not loaded (i.e., we cannot know if it is final)
      // or if the target us final
      if (PrintNotLoaded && !x->target_is_loaded()) {
        tty->print_cr("   ### class not loaded at invoke bci %d", x->bci());
      }
      needs_null_check = true;
    }

    // After freeing the register used by receiver, we are going to lock it with HideReg;
    // By specifying the Invoke as the locking instructions, we guarantee that it will not be
    // spilled (the spill order is defined by the bci of instructions)
    { HideReg reg(this, receiver.get_register());
      invoke_load_arguments(x, args, NULL);
    }
    // the invoke spill does not lock new registers, therefore
    // it is safe to spill while receiver register is not locked
    debug_only(ra()->lock_locking(true);)
    invoke_do_spill(x);
    debug_only(ra()->lock_locking(false);)
    if (!receiver.get_register().is_same(hint.get_register())) {
      emit()->move(item2lir(&receiver), hint.get_register());
    }
    invoke_do_result(x, needs_null_check);
  } else {
    ItemArray* args = invoke_visit_arguments(x, NULL);
    invoke_load_arguments(x, args, NULL);
    invoke_do_spill(x);
    invoke_do_result(x, false);
  }
}


void ValueGen::do_NewInstance(NewInstance* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  assert(ra()->are_all_registers_free(), "all registers must be free across calls");
  RInfo reg = result_register_for(x->type());
  ra()->lock_register(x, reg);
  set_result(x, reg);
  RInfo tmp1;
  RInfo tmp2;
  RInfo tmp3;
  RInfo klassR;
  {
    HideReg hr1(this, objectType); tmp1 = hr1.reg();
    HideReg hr2(this, objectType); tmp2 = hr2.reg();
    HideReg hr3(this, objectType); tmp3 = hr3.reg();
    HideReg hr4(this, objectType); klassR = hr4.reg();

  }

  if (PrintNotLoaded && !x->klass()->is_loaded()) {
    tty->print_cr("   ###class not loaded at new bci %d", x->bci());
  }
  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope());
  emit()->new_instance(reg, x->klass(), tmp1, tmp2, tmp3, klassR, info);
}


void ValueGen::do_NewTypeArray(NewTypeArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  Item length(x->length());
  ValueGen l(&length, HintItem::no_hint(), this);
  load_item(&length);
  item_free(&length);
  assert(ra()->are_all_registers_free(), "all registers must be free across calls");
  RInfo reg = result_register_for(x->type());
  ra()->lock_register(x, reg);
  set_result(x, reg);
  RInfo length_reg;
  RInfo tmp2;
  RInfo tmp3;
  RInfo klass_reg;
  {
    HideReg lengthHR(this, FrameMap::_ebxRInfo, true); length_reg = lengthHR.reg();
    HideReg hr2(this, objectType); tmp2 = hr2.reg();
    HideReg hr3(this, objectType); tmp3 = hr3.reg();
    HideReg klassHR(this, FrameMap::_edxRInfo, true);  klass_reg  = klassHR.reg();
  }

  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope());
  emit()->new_type_array(reg, x->elt_type(), item2lir(&length), length_reg, tmp2, tmp3, norinfo, klass_reg, info);
}


void ValueGen::do_NewObjectArray(NewObjectArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  Item length(x->length());
  ValueGen l(&length, HintItem::no_hint(), this);
  // in case of patching (i.e., object class is not yet loaded), we need to reexecute the instruction
  // and therefore provide the state before the parameters have been consumed
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info =  new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope());
  }

  load_item(&length);
  item_free(&length);
  RInfo reg = result_register_for(x->type());
  ra()->lock_register(x, reg);
  set_result(x, reg);
  RInfo klass_reg;
  RInfo length_reg;
  RInfo tmp2;
  RInfo tmp4;
  {
    HideReg lengthHR(this, FrameMap::_ebxRInfo, true); length_reg = lengthHR.reg();
    HideReg klassHR(this, FrameMap::_edxRInfo, true);  klass_reg  = klassHR.reg();
    HideReg hr2(this, objectType); tmp2 = hr2.reg();
    HideReg hr4(this, objectType); tmp4 = hr4.reg();
  }

  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope());
  emit()->new_object_array(reg, x->klass(), item2lir(&length), length_reg, tmp2, norinfo, tmp4, klass_reg, info, patching_info);
}


void ValueGen::do_NewMultiArray(NewMultiArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  if (x->state_before() != NULL) {
    spill_values_on_stack(x->state_before());
  }
  Values* dims = x->dims();
  int i = dims->length();
  GrowableArray<Item*>* items = new GrowableArray<Item*>(i, i, NULL);
  while (i-- > 0) {
    Item* size = new Item(dims->at(i));
    ValueGen s(size, HintItem::no_hint(), this);
    items->at_put(i, size);
    assert(!size->is_register() || x->state_before() == NULL, "shouldn't be since it was spilled above");
  }

  // need to get the info before, as the items may become invalid through item_free

  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope());
  }
  i = dims->length();
  while (i-- > 0) {
    Item* size = items->at(i);
    load_item(size);
    item_free(size);

    emit()->store_stack_parameter(item2lir(size), i);
  }
  assert(ra()->are_all_registers_free(), "all registers must be free across calls");
  RInfo reg = result_register_for(x->type());
  ra()->lock_register(x, reg);
  set_result(x, reg);

  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope());
  emit()->new_multi_array(reg, x->klass(), x->rank(), norinfo, info, patching_info);
}


void ValueGen::do_BlockBegin(BlockBegin* x) {
  // nothing to do for now
}


void ValueGen::do_CheckCast(CheckCast* x) {
  // all values are spilled
  spill_values_on_stack(x->state());
  Item obj(x->obj());
  obj.set_destroys_register();
  ValueGen o(&obj, HintItem::no_hint(), this);
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || (PatchALot && !x->is_incompatible_class_change_check())) {
    // must do this before locking the destination register as an oop register,
    // and before the obj is loaded (the latter is for deoptimization)
    patching_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
  }
  load_item(&obj);
  const RInfo in_reg = obj.get_register();

  // info for exceptions
  CodeEmitInfo* info_for_exception = new CodeEmitInfo(emit(), x->bci(), NULL, x->state()->copy_locks(), x->exception_scope(), ra()->oops_in_registers());

  RInfo reg = rlock(x);
  set_result(x, reg);
  RInfo tmp1 = lock_free_rinfo(NULL, objectType);
  RInfo tmp2 = lock_free_rinfo(NULL, objectType);
  ra()->free_reg(tmp1);
  ra()->free_reg(tmp2);
  item_free(&obj);

  // make sure dst_reg is not equal as obj_reg
  assert(!obj.get_register().is_same(reg), "check");
  if (patching_info != NULL) {
    patching_info->add_register_oop(in_reg);
  }
  CodeStub* stub;
  LIR_Opr obj_opr = item2lir(&obj);
  if (x->is_incompatible_class_change_check()) {
    assert(patching_info == NULL, "can't patch this");
    stub = new SimpleExceptionStub(Runtime1::throw_incompatible_class_change_error_id, norinfo, info_for_exception);
  } else {
    stub = new SimpleExceptionStub(Runtime1::throw_class_cast_exception_id, obj_opr->rinfo(), info_for_exception);
  }
  __ checkcast(LIR_OprFact::rinfo(reg, T_OBJECT), obj_opr, x->klass(),
               LIR_OprFact::rinfo(tmp1), LIR_OprFact::rinfo(tmp2),
               x->direct_compare(), info_for_exception, patching_info, stub);
}


void ValueGen::do_InstanceOf(InstanceOf* x) {
  spill_values_on_stack(x->state());
  Item obj(x->obj());
  obj.set_destroys_register();
  ValueGen o(&obj, HintItem::no_hint(), this);
  // result and test object may not be in same register
  RInfo reg = rlock(x);
  set_result(x, reg);
  CodeEmitInfo* patching_info = NULL;
  if ((!x->klass()->is_loaded() || PatchALot)) {
    // must do this before locking the destination register as an oop register
    patching_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
  }
  load_item(&obj);
  // do not include tmp in the oop map 
  if (patching_info != NULL) {
    // object must be part of the oop map
    patching_info->add_register_oop(obj.get_register());
  }
  RInfo tmp = lock_free_rinfo(NULL, objectType);
  ra()->free_reg(tmp);
  item_free(&obj);
  emit()->instanceof_op(LIR_OprFact::rinfo(reg, T_OBJECT), item2lir(&obj), x->klass(), tmp, obj.get_register(), x->direct_compare(), patching_info);
}


void ValueGen::do_If(If* x) {
  assert(x->number_of_sux() == 2, "inconsistency");
  ValueTag tag = x->x()->type()->tag();
  HintItem left_hint(x->x()->type());  left_hint.set_from_item (HintItem::no_hint());
  HintItem right_hint(x->y()->type()); right_hint.set_from_item(HintItem::no_hint());
  if (x->x()->type()->is_float()) {
    left_hint.set_round32();
    right_hint.set_round32();
  }

  Item xitem(x->x());
  Item yitem(x->y());
  Item* xin = &xitem;
  Item* yin = &yitem;
  If::Condition cond = x->cond();
  {  // call destructors before doing move to phi

    ValueGen xg(&xitem, &left_hint, this);
    ValueGen yg(&yitem, &right_hint, this);
    if (tag == longTag) {
      // for longs, only conditions "eql", "neq", "lss", "geq" are valid;
      // mirror for other conditions
      if (cond == If::gtr || cond == If::leq) {
        cond = Instruction::mirror(cond);
        xin = &yitem;
        yin = &xitem;
      }
      xin->set_destroys_register();
    } if (tag == floatTag || tag == doubleTag) {
      xin->set_destroys_register();
      yin->set_destroys_register();
    }
    load_item(xin);
    if (tag == longTag && yin->is_constant() && yin->get_jlong_constant() == 0 && (cond == If::eql || cond == If::neq)) {
      // inline long zero
      dont_load_item(yin);
    } else if (tag == longTag || tag == floatTag || tag == doubleTag) {
      // longs cannot handle constants at right side
      load_item(yin);
    } else {
      dont_load_item(yin);
    }
  }

  CodeEmitInfo* safepoint_info = NULL;
  if (x->is_safepoint()) {
    safepoint_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
    safepoint_info->set_is_compiled_safepoint();

    if (SafepointPolling) {
      __ safepoint(norinfo, safepoint_info);
      safepoint_info = NULL;
    }
  }

  item_free(xin);
  item_free(yin);
  set_no_result(x);

  emit()->if_op(1, cond, item2lir(xin), item2lir(yin), x->tsux(), x->fsux(), x->usux(), safepoint_info);
  move_to_phi(x->state());
  emit()->if_op(2, cond, item2lir(xin), item2lir(yin), x->tsux(), x->fsux(), x->usux(), NULL);

  goto_default_successor(x, NULL);
}


void ValueGen::do_IfInstanceOf(IfInstanceOf* x) {
  Unimplemented();
  NEEDS_CLEANUP // remove this code?
  spill_values_on_stack(x->state());
  Item obj (x->obj());
  obj.set_destroys_register();
  ValueGen o(&obj, HintItem::no_hint(), this);
  // result and test object may not be in same register
  RInfo reg = rlock(x);
  set_result(x, reg);
  load_item(&obj);
  RInfoCollection* oop_regs = NULL;

  if (!x->klass()->is_loaded()) {
    // must do this before locking the destination register as an oop register
    oop_regs = ra()->oops_in_registers();
  }
  RInfo tmp = lock_free_rinfo(NULL, objectType);
  ra()->free_reg(tmp);
  item_free(&obj);

  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->instanceof_bci(), ra()->oops_in_spill(), x->state(), x->exception_scope(), oop_regs);
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = new CodeEmitInfo(emit(), x->instanceof_bci(), ra()->oops_in_spill(), /*x->state_before()*/ NULL, x->exception_scope(), oop_regs);
  }
  emit()->instance_of_test_op(reg, item2lir(&obj), x->klass(), tmp, x->tsux(), x->fsux(), !x->test_is_instance(), info, patching_info);
}


// May change the content of tag
void ValueGen::setup_phis_for_switch(Item* tag, ValueStack* stack) {
  // if stack() exists, then move_to_phi may destroy the tag register as
  // the tag register is not locked anymore.
  if (stack != NULL && stack->stack_size() > 0 ) {
    // stack exists, move values into phi locations
    // preserve the tag

    // save
    emit()->push_item(item2lir(tag));

    move_to_phi(stack);
    { HideReg hr(this, nonReturnRInfo()); // use one register that is not a result register
      tag->set_register(hr.reg());
      // restore (do not use result registers)
      emit()->pop_item(item2lir(tag));
    }
  }
}


void ValueGen::do_TableSwitch(TableSwitch* x) {
  Item tag(x->tag());
  { ValueGen v(&tag, HintItem::no_hint(), this);
    load_item(&tag);
    item_free(&tag);
    set_no_result(x);
  }

  setup_phis_for_switch(&tag, x->state());
  if (x->is_safepoint()) {
    CodeEmitInfo* info_before = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
    if (SafepointPolling) {
      __ safepoint(norinfo, info_before);
    } else {
      emit()->safepoint_nop(info_before);
    }
  }
  int lo_key = x->lo_key();
  int hi_key = x->hi_key();
  int len = x->length();
  for (int i = 0; i < len && !emit()->must_bailout(); i++) {
    int matchVal = i + lo_key;
    emit()->tableswitch_op(item2lir(&tag), matchVal, x->sux_at(i));
  }
  emit()->goto_op(x->default_sux(), NULL);
}


define_array(LookupRangeArray, LookupRange*)
define_stack(LookupRangeList, LookupRangeArray)


// we expect the keys to be sorted by increasing value
static LookupRangeArray* create_lookup_ranges(LookupSwitch* x) {
  LookupRangeList* res = new LookupRangeList();
  int len = x->length();
  if (len > 0) {
    int key = x->key_at(0);
    BlockBegin* sux = x->sux_at(0);
    LookupRange* range = new LookupRange(key, sux);
    for (int i = 1; i < len; i++) {
      int new_key = x->key_at(i);
      BlockBegin* new_sux = x->sux_at(i);
      if (key+1 == new_key && sux == new_sux) {
        // still in same range
        range->set_high_key(new_key);
      } else {
        res->append(range);
        range = new LookupRange(new_key, new_sux);
      }
      key = new_key;
      sux = new_sux;
    }
    if (res->length() == 0 || res->last() != range)  res->append(range);
  }
  return res;
}


void ValueGen::do_LookupSwitch(LookupSwitch* x) {
  Item tag(x->tag());
  { ValueGen v(&tag, HintItem::no_hint(), this);
    load_item(&tag);
    item_free(&tag);
    set_no_result(x);
  }
  setup_phis_for_switch(&tag, x->state());
  if (x->is_safepoint()) {
    CodeEmitInfo* info_before = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
    if (SafepointPolling) {
      __ safepoint(norinfo, info_before);
    } else {
      emit()->safepoint_nop(info_before);
    }
  }
  if (UseTableRanges) {
    LookupRangeArray* ranges = create_lookup_ranges(x);
    int lng = ranges->length();
    for (int i = 0; i < lng && !emit()->must_bailout(); i++) {
      LookupRange* one_range = ranges->at(i);
      emit()->lookupswitch_range_op(item2lir(&tag), one_range->low_key(), one_range->high_key(), one_range->sux());
    }
    emit()->goto_op(x->default_sux(), NULL);
  } else {
    int len = x->length();
    for (int i = 0; i < len && !emit()->must_bailout(); i++) {
      emit()->lookupswitch_op(item2lir(&tag), x->key_at(i), x->sux_at(i));
    }
    emit()->goto_op(x->default_sux(), NULL);
  }
}


void ValueGen::do_Return(Return* x) {
  assert(x->is_synchronized() ? method()->is_synchronized() : !method()->is_synchronized(),
         "should match");
  if (x->type()->is_void()) {
    if (x->is_synchronized()) {
      emit()->return_op_prolog(x->monitor_no());
    }
    emit()->return_op(LIR_OprFact::illegalOpr);
  } else {
    RInfo reg = result_register_for(x->type());
    Item result(x->result());
    result.handle_float_kind();
    HintItem result_hint(x->type(), reg);
    if (x->type()->is_float()) {
      result_hint.set_round32();
    }
    ValueGen r(&result, &result_hint, this);

    if (x->is_synchronized()) {
      if (result.is_register()) {
        NEEDS_CLEANUP;
        // we may be spilling the result of a LoadLocal which is available
        // from it's original stack location.  We should be doing something
        // smarter about spilling values which are easily recreated.
        // spill the temporary result
        spill_value(x->result());
        result.update();
      }
      lock_spill_temp(NULL, syncTempRInfo());
      ra()->free_reg(syncTempRInfo());

      emit()->return_op_prolog(x->monitor_no());
    }

    load_item_force(&result, reg);
    item_free(&result);
    assert(reg.is_same(result.get_register()), "result should be in return register");
    emit()->return_op(item2lir(&result));
  }
  set_no_result(x);
}


void ValueGen::do_Base(Base* x) {
  emit()->std_entry(scope(), compilation()->get_init_vars(), receiverRInfo(), icKlassRInfo());
}


// Example: Thread.currentThread()
void ValueGen::do_currentThread(Intrinsic* x) {
  assert(x->number_of_arguments() == 0, "wrong type");
  RInfo result = rlock_result_with_hint(x, hint());
  emit()->lir()->get_thread(result);
  emit()->lir()->load_mem_reg(result, in_bytes(JavaThread::threadObj_offset()), result, T_OBJECT, NULL);
}

void ValueGen::do_CompareAndSwap(Intrinsic* x, ValueType* type) {
  assert(x->number_of_arguments() == 4, "wrong type");
  Item obj   (x->argument_at(0));  // object
  Item offset(x->argument_at(1));  // offset of field
  Item cmp   (x->argument_at(2));  // value to compare with field
  Item val   (x->argument_at(3));  // replace field with val if matches cmp

  // Some common code before hitting different flavors ...

  // evaluate offset first, to free up double reg
  ValueGen offset_v(&offset, HintItem::no_hint(), this); 

  cmp.set_destroys_register(); // destroyed by cmpxchg op
  obj.set_destroys_register(); // replaced with address
  LIR_Opr ill = LIR_OprFact::illegalOpr;  // for convenience

  // The int and object cases are almost the same. The long case
  // needs different registers. All cases require care in not
  // stressing out register allocator.

  if (type == intType || type == objectType) {
    RInfo cmp_rinfo = as_RInfo(eax); // cmpxchg requires cmp val in eax
    HintItem cmp_hint(type, cmp_rinfo);
    ValueGen cmp_v(&cmp, &cmp_hint, this);
    load_item_force(&cmp, cmp_rinfo);
    
    ValueGen val_v(&val, HintItem::no_hint(), this);
    load_item(&val);
    
    ValueGen obj_v(&obj, HintItem::no_hint(), this);
    load_item(&obj);
    load_item(&offset);
    
    item_free(&offset);
    item_free(&cmp);
    item_free(&val);
    if (type != objectType)
      item_free(&obj);
    // else don't free yet, to make sure we have reg for write barrier
    
    LIR_Opr addr = item2lir(&obj); // obj becomes offset
    emit()->lir()->add(addr, item2lir(&offset), addr);

    if (type == objectType)
      emit()->lir()->cas_obj(addr, item2lir(&cmp), item2lir(&val), ill, ill);
    else
      emit()->lir()->cas_int(addr, item2lir(&cmp), item2lir(&val), ill, ill);

    // convert flag to boolean/int result
    RInfo result = rlock_result_with_hint(x, hint());
    LabelObj* L = new LabelObj();
    emit()->move(LIR_OprFact::intConst(1), result);
    emit()->lir()->branch(LIR_OpBranch::equal, L->label());
    emit()->move(LIR_OprFact::intConst(0), result);
    emit()->lir()->branch_destination(L->label());
    
    if (type == objectType) { // Write-barrier needed for Object fields.
      item_free(&obj);
      emit()->write_barrier(addr, ill);
    }
  }
  else if (type == longType) {
    // cmpxch8 requires cmp in edx,eax (hi,lo); 
    RInfo cmp_rinfo = as_RInfo(eax, edx);   
    HintItem cmp_hint(longType, cmp_rinfo);
    ValueGen cmp_v(&cmp, &cmp_hint, this);
    load_item_force(&cmp, cmp_rinfo);
    
    // new value must be in ecx,ebx (hi,lo)
    RInfo val_rinfo = as_RInfo(ebx, ecx);
    HintItem val_hint(longType, val_rinfo);
    ValueGen val_v(&val, &val_hint, this);
    load_item_force(&val, val_rinfo);
    
    ValueGen obj_v(&obj, HintItem::no_hint(), this);
    load_item(&offset);
    load_item(&obj);

    item_free(&obj);
    item_free(&offset);
    item_free(&cmp);
    item_free(&val);

    LIR_Opr addr = item2lir(&obj);
    emit()->lir()->add(addr, item2lir(&offset), addr);
    emit()->lir()->cas_long(addr, item2lir(&cmp), item2lir(&val), ill, ill);
    
    // convert flag to boolean/int result
    RInfo result = rlock_result_with_hint(x, hint());
    LabelObj* L = new LabelObj();
    emit()->move(LIR_OprFact::intConst(1), result);
    emit()->lir()->branch(LIR_OpBranch::equal, L->label());
    emit()->move(LIR_OprFact::intConst(0), result);
    emit()->lir()->branch_destination(L->label());
  }
  else {
    ShouldNotReachHere();
  }

}

