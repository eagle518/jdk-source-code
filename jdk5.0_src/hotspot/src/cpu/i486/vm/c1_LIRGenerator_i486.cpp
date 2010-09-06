#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIRGenerator_i486.cpp	1.8 04/04/20 15:56:20 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

# include "incls/_precompiled.incl"
# include "incls/_c1_LIRGenerator_i486.cpp.incl"

#ifndef PRODUCT

// Item will be loaded into a byte register; Intel only
void LIRItem::load_byte_item() {
  if (result()->is_register() && FrameMap::is_byte_rinfo(result()->rinfo())) {
    // it is a byte register: we are fine
  } else {
    // make sure that it is a byte register
    assert(!value()->type()->is_float() && !value()->type()->is_double() , "canot load floats in byte register");
    LIR_Opr reg = _gen->rlock_result(value(), T_BYTE);
    _gen->emit()->move(result(), reg);

    _result = reg;
  }
}


void LIRItem::load_nonconstant() {
  if (is_constant()) {
    dont_load_item();
  } else {
    load_item();
  }
}

//--------------------------------------------------------------
//               LIRGenerator
//--------------------------------------------------------------


RInfo LIRGenerator::exceptionOopRInfo()              { return FrameMap::_eaxRInfo;     }
RInfo LIRGenerator::exceptionPcRInfo()               { return FrameMap::_edxRInfo;     }
RInfo LIRGenerator::return1RInfo()                   { return FrameMap::_eaxRInfo;     }
RInfo LIRGenerator::callee_return1RInfo()            { return FrameMap::_eaxRInfo;     }
RInfo LIRGenerator::divInRInfo()                     { return FrameMap::_eaxRInfo;     }
RInfo LIRGenerator::divOutRInfo()                    { return FrameMap::_eaxRInfo;     }
RInfo LIRGenerator::remOutRInfo()                    { return FrameMap::_edxRInfo;     }
RInfo LIRGenerator::shiftCountRInfo()                { return FrameMap::_ecxRInfo;     }
RInfo LIRGenerator::return2RInfo()                   { return FrameMap::_eax_edxRInfo; } // for long result Lo:eax/Hi:edx
RInfo LIRGenerator::callee_return2RInfo()            { return FrameMap::_eax_edxRInfo; } // for long result Lo:eax/Hi:edx
RInfo LIRGenerator::returnF0RInfo()                  { return FrameMap::_f0RInfo;      }
RInfo LIRGenerator::returnD0RInfo()                  { return FrameMap::_d0RInfo;      }
RInfo LIRGenerator::receiverRInfo()                  { return FrameMap::_ecxRInfo;     }
RInfo LIRGenerator::syncTempRInfo()                  { return FrameMap::_eaxRInfo;     }
RInfo LIRGenerator::nonReturnRInfo()                 { return FrameMap::_esiRInfo;     }
RInfo LIRGenerator::icKlassRInfo()                   { return FrameMap::_eaxRInfo;     }

//-----------------------spilling--------------------------------------------------------


// returns true if reg could be smashed by a callee.
// If is_C_call, return true when register could be smashed by a C-call.
bool LIRGenerator::is_caller_save_register(RInfo reg) {
  return true; // all registers are caller saved on INTEL
}



//--------- loading items into registers --------------------------------


// For correct fpu stack simulation we must dereference
// an fpu register explictly, by copying it into a new register;
// in such case, item will be set to a new register; the reason is the simple
// FPU stack emulation which requires that every floating point register is being
// popped.
void LIRGenerator::check_float_register(LIR_Opr) {
  Unimplemented();
}


// Handle fanout of fpu registers: return false if no fanout;
// If fanout than, we copy the value of float register into a new one,
// so that the new FPU register has ref-count 1
bool LIRGenerator::fpu_fanout_handled() {
  if (result()->is_register() && (value()->type()->is_float_kind())) {
    // The item is float or double register with a use_count > 1
    LIR_Opr reg = rlock(value());
    emit()->copy_fpu_item(reg->rinfo(), result());

    set_result(value(), reg);
    return true;
  } else {
    return false;
  }
}

// i486 instructions can inline constants
bool LIRGenerator::can_inline_any_constant() const {
  return true;
}


bool LIRGenerator::can_inline_as_constant(Value v) const {
  return true;
}


bool LIRGenerator::prefer_alu_registers() const {
  return true;
}

RInfo LIRGenerator::scratch1_RInfo() {
  return norinfo; // no scratch registers on i486
}


//----------------------------------------------------------------------
//             visitor functions
//----------------------------------------------------------------------


void LIRGenerator::do_StoreIndexed(StoreIndexed* x) {
  assert(x->is_root(),"");
  bool obj_store = x->elt_type() == T_ARRAY || x->elt_type() == T_OBJECT;
  LIRItem array(x->array(), this);
  LIRItem index(x->index(), this);
  LIRItem value(x->value(), this);
  if (obj_store) {
    array.set_destroys_register();
  }
  value.handle_float_kind();
  array.load_item();
  index.load_nonconstant();

  bool must_load = true;
  if (x->elt_type() == T_SHORT || x->elt_type() == T_CHAR) {
    // there is no immediate move of word values in asembler_i486.?pp
    must_load = true;
  } else {
    if (obj_store && value.is_constant() && !get_jobject_constant(x->value())->is_loaded()) {
      // do nothing; do not load (NULL object)
      must_load = false;
    } else if (value.is_constant() && !obj_store) {
      // array store check needs a register, otherwise do not load a constant
      must_load = false;
    }
  }
  if (must_load) {
    // for T_BYTE element type, we must have a byte register free
    if (x->elt_type() == T_BYTE || x->elt_type() == T_BOOLEAN) {
      value.load_byte_item();
    } else {
      value.load_item();
    }
  }
  set_no_result(x);

  CodeEmitInfo* range_check_info = state_for(x);
  CodeEmitInfo* null_check_info = NULL;
  bool needs_null_check = x->needs_null_check();
  if (needs_null_check) {
    null_check_info = range_check_info;
  }

  if (GenerateRangeChecks) {
    emit()->array_range_check(array.result(), index.result(), null_check_info, range_check_info);
    // range_check also does the null check
    needs_null_check = false;
  }

  if (obj_store) {
    if (value.is_constant() && !get_jobject_constant(x->value())->is_loaded()) {
      // skip store check
    } else if (GenerateArrayStoreCheck) {
      RInfo tmp1 = new_register(objectType)->rinfo();
      RInfo tmp2 = new_register(objectType)->rinfo();
      RInfo tmp3 = new_register(objectType)->rinfo();

      emit()->array_store_check(array.result(), value.result(), tmp1, tmp2, tmp3, range_check_info);
    }
  }
  emit()->indexed_store(x->elt_type(), array.result(), index.result(), value.result(), norinfo, null_check_info);
}


void LIRGenerator::do_MonitorEnter(MonitorEnter* x) {
  spill_values_on_stack(x->state());
  assert(x->is_root(),"");
  LIRItem obj(x->obj(), this);
  obj.load_item();
  set_no_result(x);

  RInfo lock = new_register(T_OBJECT)->rinfo();

  CodeEmitInfo* info_for_exception = NULL;
  if (x->needs_null_check()) {
    info_for_exception = state_for(x, x->lock_stack_before());
  }
  CodeEmitInfo* info = state_for(x, x->state());
  emit()->monitor_enter(obj.get_register(), lock, syncTempRInfo(), norinfo, x->monitor_no(), info_for_exception, info);
}


void LIRGenerator::do_MonitorExit(MonitorExit* x) {
  spill_values_on_stack(x->state());
  assert(x->is_root(),"");

  LIRItem obj(x->obj(), this);
  obj.dont_load_item();

  RInfo lock = new_register(T_INT)->rinfo();
  RInfo obj_temp = new_register(T_INT)->rinfo();
  set_no_result(x);
  emit()->monitor_exit(obj_temp, lock, syncTempRInfo(), x->monitor_no());
}


// _ineg, _lneg, _fneg, _dneg
void LIRGenerator::do_NegateOp(NegateOp* x) {
  LIRItem value(x->x(), this);
  value.set_destroys_register();
  value.load_item();
  RInfo reg = rlock_result(x)->rinfo();
  emit()->negate(reg, value.result());

  round_item(x->operand());
}



// for  _fadd, _fmul, _fsub, _fdiv, _frem
//      _dadd, _dmul, _dsub, _ddiv, _drem
void LIRGenerator::do_ArithmeticOp_FPU(ArithmeticOp* x) {
  // Because of the way we simulate FPU  register location in code emitter, we must
  // mark both items as being destroyed(i.e., they are going to be removed from the FPU stack)
  LIRItem left(x->x(),  this);
  LIRItem right(x->y(), this);
  right.set_destroys_register();
  left.set_destroys_register();
  LIRItem* left_arg  = &left;
  LIRItem* right_arg = &right;
  if (x->is_commutative() && left.is_stack() && right.is_register()) {
      // swap them if left is real stack (or cached) and right is real register(not cached)
    left_arg = &right;
    right_arg = &left;
  }
  left_arg->load_item();
  // do not need to load right, as we can handle stack and constants
  if ( x->y()->type()->is_constant() || x->op() == Bytecodes::_frem || x->op() == Bytecodes::_drem) {
    // cannot handle inlined constants
    right_arg->load_item();
  } else {
    right_arg->dont_load_item();
  }
  RInfo reg = rlock_result(x)->rinfo();
  emit()->arithmetic_op_fpu(x->op(), x->operand(), left_arg->result(), right_arg->result(), x->is_strictfp());

  round_item(x->operand());
}



// for  _ladd, _lmul, _lsub, _ldiv, _lrem
void LIRGenerator::do_ArithmeticOp_Long(ArithmeticOp* x) {
  if (x->op() == Bytecodes::_ldiv || x->op() == Bytecodes::_lrem ) {
    // this is a call
    LIRItem left(x->x(),  this);
    LIRItem right(x->y(), this);
    right.set_destroys_register(); // actually: only the div_by_zero destroys register

    left.load_item();
    emit()->push_item(left.result());

    right.load_item();
    emit()->push_item(right.result());

    CodeEmitInfo* info = state_for(x);
    emit()->explicit_div_by_zero_check(right.result(), info);

    set_with_result_register(x);
    emit()->arithmetic_call_op(x->op(), norinfo);
  } else if (x->op() == Bytecodes::_lmul) {
    // missing test if instr is commutative and if we should swap
    LIRItem left(x->x(), this);
    LIRItem right(x->y(), this);
    left.set_destroys_register();
    left.load_item();
    right.load_item();

    // WARNING: for longs, we must guarantee (algorithmically) that the locked lo result
    //          register is not the same as the left-HI register , otherwise we
    //          would overwrite the results.
    //
    set_result(x, FrameMap::_eax_edxRInfo);
    emit()->arithmetic_op_long(x->op(), x->operand(), left.result(), right.result(), NULL);
  } else {
    // missing test if instr is commutative and if we should swap
    LIRItem left(x->x(), this);
    LIRItem right(x->y(), this);
    left.set_destroys_register();
    left.load_item();
    right.load_item();
    // WARNING: for longs, we must guarantee (algorithmically) that the locked lo result
    //          register is not the same as the left-HI register , otherwise we
    //          would overwrite the results.
    //
    RInfo reg = rlock_result(x)->rinfo();
    emit()->arithmetic_op_long(x->op(), x->operand(), left.result(), right.result(), NULL);
  }
}



// for: _iadd, _imul, _isub, _idiv, _irem
void LIRGenerator::do_ArithmeticOp_Int(ArithmeticOp* x) {
  if (x->op() == Bytecodes::_idiv || x->op() == Bytecodes::_irem) {
    // The requirements for division and modulo
    // input : eax: dividend                         min_int
    //         reg: divisor   (may not be eax/edx)   -1
    //
    // output: eax: quotient  (= eax idiv reg)       min_int
    //         edx: remainder (= eax irem reg)       0

    // eax and edx will be destroyed

    // Note: does this invalidate the spec ???
    LIRItem right(x->y(), this);
    LIRItem left(x->x() , this);   // visit left second, so that the is_register test is valid
    left.set_destroys_register();
    right.set_destroys_register();
    left.load_item();
    right.load_item();

    if (x->op() == Bytecodes::_idiv) {
      set_result(x, divOutRInfo());
    } else {
      set_result(x, remOutRInfo());
    }
    if (!ImplicitDiv0Checks) {
      CodeEmitInfo* info = state_for(x);
      emit()->explicit_div_by_zero_check(right.result(), info);
    }
    CodeEmitInfo* info = state_for(x);
    RInfo tmp = FrameMap::_edxRInfo; // idiv and irem use edx in their implementation
    emit()->arithmetic_idiv(x->op(), x->operand(), left.result(), right.result(), tmp, info);
  } else {
    // missing test if instr is commutative and if we should swap
    LIRItem left(x->x(),  this);
    LIRItem right(x->y(), this);
    LIRItem* left_arg = &left;
    LIRItem* right_arg = &right;
    if (x->is_commutative() && left.is_stack() && right.is_register()) {
      // swap them if left is real stack (or cached) and right is real register(not cached)
      left_arg = &right;
      right_arg = &left;
    }
    left_arg->set_destroys_register();
    left_arg->load_item();
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
        right_arg->dont_load_item();
      } else {
        right_arg->load_item();
      }
      if (use_tmp) {
        tmp = new_register(T_INT)->rinfo();
      }
      RInfo reg = rlock_result(x)->rinfo();

      emit()->arithmetic_op_int(x->op(), x->operand(), left_arg->result(), right_arg->result(), tmp);
    } else {
      right_arg->dont_load_item();
      RInfo reg = rlock_result(x)->rinfo();

      RInfo tmp;
      emit()->arithmetic_op_int(x->op(), x->operand(), left_arg->result(), right_arg->result(), tmp);
    }
  }
}


void LIRGenerator::do_ArithmeticOp(ArithmeticOp* x) {
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
void LIRGenerator::do_ShiftOp(ShiftOp* x) {
  // count must always be in ecx
  LIRItem value(x->x(), this);
  LIRItem count(x->y(), this);
  value.set_destroys_register();
  ValueTag elemType = x->type()->tag();
  bool must_load_count = !count.is_constant() || elemType == longTag;
  if (must_load_count) {
    // count for long must be in register
    count.load_item();
  } else {
    count.dont_load_item();
  }
  value.load_item();
  RInfo tmp;
  if (elemType == intTag && count.is_register()) {
    // in case we cache the count, we want always to have a register free
    // (without caching, the count may be loaded in ecx; with the caching count,
    // the count register may not be ecx
    tmp = new_register(T_INT)->rinfo();
  }
  RInfo reg = rlock_result(x)->rinfo();

  emit()->shift_op(x->op(), reg, value.result(), count.result(), tmp);
}


// _iand, _land, _ior, _lor, _ixor, _lxor
void LIRGenerator::do_LogicOp(LogicOp* x) {
  // missing test if instr is commutative and if we should swap
  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
  left.set_destroys_register();
  left.load_item();
  right.load_nonconstant();
  RInfo reg = rlock_result(x)->rinfo();

  emit()->logic_op(x->op(), reg, left.result(), right.result());
}



// _lcmp, _fcmpl, _fcmpg, _dcmpl, _dcmpg
void LIRGenerator::do_CompareOp(CompareOp* x) {
  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
  ValueTag tag = x->x()->type()->tag();
  if (tag == floatTag || tag == doubleTag) {
    left.set_destroys_register();
    right.set_destroys_register();
  } else {
    if (tag == longTag) {
      left.set_destroys_register();
    }
  }
  left.load_item();
  right.load_item();
  RInfo reg = rlock_result(x)->rinfo();
  emit()->compare_op(x->op(), reg, left.result(), right.result());
}


// Code for  :  x->x() {x->cond()} x->y() ? x->tval() : x->fval()
void LIRGenerator::do_IfOp(IfOp* x) {
#ifdef ASSERT
  {
    ValueTag xtag = x->x()->type()->tag();
    ValueTag ttag = x->tval()->type()->tag();
    assert(xtag == intTag || xtag == objectTag, "cannot handle others");
    assert(ttag == addressTag || ttag == intTag || ttag == objectTag || ttag == longTag, "cannot handle others");
    assert(ttag == x->fval()->type()->tag(), "cannot handle others");
  }
#endif

  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
  left.load_item();
  right.load_item();

  emit()->ifop_phase1(x->cond(), left.result(), right.result());

  LIRItem t_val(x->tval(), this);
  LIRItem f_val(x->fval(), this);
  t_val.dont_load_item();
  f_val.dont_load_item();
  RInfo reg;
  if (x->fval()->type()->tag() == longTag) {
    // must lock before releasing
    reg = rlock_result(x)->rinfo();
  }
  if (x->fval()->type()->tag() != longTag) {
    reg = rlock_result(x)->rinfo();
  }

  emit()->ifop_phase2(reg, t_val.result(), f_val.result(), x->cond());
}


void LIRGenerator::do_Intrinsic(Intrinsic* x) {
  switch (x->id()) {
  case methodOopDesc::_getClass:       do_getClass(x);      break;
  case methodOopDesc::_currentThread:  do_currentThread(x); break;

  case methodOopDesc::_dsqrt:          // fall through
  case methodOopDesc::_dsin :          // fall through
  case methodOopDesc::_dcos :          do_MathIntrinsic(x); break;
  case methodOopDesc::_arraycopy:      do_ArrayCopy(x);     break;

  // java.nio.Buffer.checkIndex
  case methodOopDesc::_checkIndex:     do_NIOCheckIndex(x); break;

  default: ShouldNotReachHere(); break;
  }
}


void LIRGenerator::do_MathIntrinsic(Intrinsic* x) {
  assert(x->number_of_arguments() == 1, "wrong type");
  LIRItem value(x->argument_at(0), this);
  value.set_destroys_register();
  value.load_item();
  RInfo reg = rlock_result(x)->rinfo();

  emit()->math_intrinsic(x->id(), reg, value.result());

  round_item(x->operand());
}


void LIRGenerator::do_ArrayCopy(Intrinsic* x) {
  spill_values_on_stack(x->state());
  assert(x->number_of_arguments() == 5, "wrong type");
  LIRItem src(x->argument_at(0), this);
  LIRItem src_pos(x->argument_at(1), this);
  LIRItem dst(x->argument_at(2), this);
  LIRItem dst_pos(x->argument_at(3), this);
  LIRItem length(x->argument_at(4), this);

  src.load_item();
  src_pos.load_item();
  dst.load_item();
  dst_pos.load_item();
  length.load_item();
  RInfo tmp = new_register(T_INT)->rinfo();
  set_no_result(x);

  CodeEmitInfo* info = state_for(x, x->state()); // we may want to have stack (deoptimization?)
  emit()->arraycopy(src.result(), src_pos.result(), dst.result(), dst_pos.result(), length.result(), tmp, false, NULL, info); // does add_safepoint
}


// _i2l, _i2f, _i2d, _l2i, _l2f, _l2d, _f2i, _f2l, _f2d, _d2i, _d2l, _d2f
// _i2b, _i2c, _i2s
void LIRGenerator::do_Convert(Convert* x) {
  if (x->op() == Bytecodes::_f2i || x->op() == Bytecodes::_d2i) {
    LIRItem value(x->value(), this);
    value.set_destroys_register();
    value.load_item();
    RInfo reg = set_with_result_register(x)->rinfo();

    emit()->convert_op(x->op(), value.result(), reg, is_32bit_mode());
  } else {
    LIRItem value(x->value(), this);
    value.handle_float_kind();
    if (value.is_constant()) {
      value.load_item();
    } else if (x->op() != Bytecodes::_i2f && x->op() != Bytecodes::_i2d && x->op() != Bytecodes::_l2f && x->op() != Bytecodes::_l2d) {
      value.load_item();
    } else {
      value.dont_load_item();
    }
    RInfo reg;
    if (x->op() == Bytecodes::_f2l || x->op() == Bytecodes::_d2l) {
      reg = FrameMap::_eax_edxRInfo;
      set_result(x, reg);
    } else {
      reg = rlock_result(x)->rinfo();
    }

    emit()->convert_op(x->op(), value.result(), reg);
    round_item(x->operand());
  }
}


void LIRGenerator::do_CachingChange(CachingChange* x) {
  if (x->sux_block()->is_set(BlockBegin::single_precision_flag) != x->pred_block()->is_set(BlockBegin::single_precision_flag)) {
    if (x->sux_block()->is_set(BlockBegin::single_precision_flag)) {
      emit()->set_24bit_fpu_precision();
    } else {
      emit()->restore_fpu_precision();
    }
  }
}


// handle arguments, without receiver
void LIRGenerator::invoke_do_arguments(Invoke* x) {
  // handle arguments if any
  int offset_from_esp_in_words = x->size_of_arguments() - 1;
  for (int i = 0; i < x->number_of_arguments(); i++) {
    LIRItem param(x->argument_at(i), this);
    param.handle_float_kind();
    if (!param.is_constant()) {
      param.load_item();
    }

    emit()->store_stack_parameter(param.result(), offset_from_esp_in_words);

    offset_from_esp_in_words -= param.result()->is_double_word() ? 2 : 1;
  }
}


// Visits all arguments, returns appropriate items without loading them
LIRItemList* LIRGenerator::invoke_visit_arguments(Invoke* x, CallingConvention* unused) {
  LIRItemList* argument_items = new LIRItemList(x->number_of_arguments());
  for (int i = 0; i < x->number_of_arguments(); i++) {
    LIRItem* param = new LIRItem(x->argument_at(i), this);
    (*argument_items)[i] = param;
  }
  return argument_items;
}


void LIRGenerator::invoke_load_arguments(Invoke* x, LIRItemList* args, CallingConvention* unused) {
  // we need to lock a register so that we can transport values on stack
  int offset_from_esp_in_words = x->size_of_arguments() - 1;
  if (x->has_receiver()) {
    --offset_from_esp_in_words; // the receiver has been pushed already
  }
  for (int i = 0; i < x->number_of_arguments(); i++) {
    LIRItem* param = args->at(i);
    param->handle_float_kind();
    if (!param->is_constant()) {
      param->load_item();
    }

    emit()->store_stack_parameter(param->result(), offset_from_esp_in_words);

    offset_from_esp_in_words -= param->result()->is_double_word() ? 2 : 1;
  }
}


void LIRGenerator::invoke_do_spill(Invoke* x, RInfo hide_reg) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
}


void LIRGenerator::invoke_do_result(Invoke* x, bool needs_null_check, LIR_Opr receiver) {
  // setup result register
  if (x->type()->is_void()) {
    set_no_result(x);
  } else {
    RInfo reg = set_with_result_register(x)->rinfo();
  }
  // emit invoke code
  CodeEmitInfo* info = state_for(x, x->state());
  bool optimized = x->target_is_loaded() && x->target_is_final();
  // The current backend doesn't support vtable calls, so pass -1
  // instead of x->vtable_index();
  emit()->call_op(x->code(), NULL, -1, info, optimized, needs_null_check, receiverRInfo(), x->operand()); // does add_safepoint
  if (x->type()->is_float() || x->type()->is_double()) {
    emit()->set_fpu_result(x->operand()->rinfo());
    // Force rounding of results from non-strictfp when in strictfp
    // scope (or when we don't know the strictness of the callee, to
    // be safe.)
    if (method()->is_strict()) {
      if (!x->target_is_loaded() || !x->target_is_strictfp()) {
        round_item(x->operand());
      }
    }
  }
}


void LIRGenerator::do_Invoke(Invoke* x) {
  // We want to spill roots only late so that any roots with use count > 1, may have
  // its registers used before being spilled
  // invoke_do_spill(x);
  if (x->has_receiver()) {
    bool needs_null_check = false;
    // make sure that ref count of register holding the receiver has
    // use count 1 by marking receiver as register-destroying
    LIRItem receiver(x->receiver(), this);
    receiver.set_destroys_register();
    // visit first all arguments, so that the register containing receiver (and is going to be
    // hidden) will certainly not be needed; we hide the receiver with NULL instruction, and
    // therefore we cannot spill it if it is needed by somebody else
    LIRItemList* args = invoke_visit_arguments(x, NULL);
    receiver.load_item();

    emit()->store_stack_parameter(receiver.result(), x->size_of_arguments() - 1);
    if (x->needs_null_check() ||
        !x->target_is_loaded() || x->target_is_final()) {
      // emit receiver NULL check if the class of target is not loaded (i.e., we cannot know if it is final)
      // or if the target us final
      if (PrintNotLoaded && !x->target_is_loaded()) {
        tty->print_cr("   ### class not loaded at invoke bci %d", x->bci());
      }
      needs_null_check = true;
    }

    invoke_load_arguments(x, args, NULL);

    // the invoke spill does not lock new registers, therefore
    // it is safe to spill while receiver register is not locked
    invoke_do_spill(x);
    invoke_do_result(x, needs_null_check);
  } else {
    LIRItemList* args = invoke_visit_arguments(x, NULL);
    invoke_load_arguments(x, args, NULL);
    invoke_do_spill(x);
    invoke_do_result(x, false);
  }
}


void LIRGenerator::do_NewInstance(NewInstance* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  RInfo reg = set_with_result_register(x)->rinfo();
  RInfo tmp1;
  RInfo tmp2;
  RInfo tmp3;
  RInfo klassR;
  {
    LIRHideReg hr1(this, objectType); tmp1 = hr1.reg();
    LIRHideReg hr2(this, objectType); tmp2 = hr2.reg();
    LIRHideReg hr3(this, objectType); tmp3 = hr3.reg();
    LIRHideReg hr4(this, objectType); klassR = hr4.reg();

  }

  if (PrintNotLoaded && !x->klass()->is_loaded()) {
    tty->print_cr("   ###class not loaded at new bci %d", x->bci());
  }
  CodeEmitInfo* info = state_for(x, x->state());
  emit()->new_instance(reg, x->klass(), tmp1, tmp2, tmp3, klassR, info);
}


void LIRGenerator::do_NewTypeArray(NewTypeArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  LIRItem length(x->length(), this);
  length.load_item();
  RInfo reg = set_with_result_register(x)->rinfo();
  RInfo tmp1;
  RInfo tmp2;
  RInfo tmp3;
  RInfo klassR;
  {
    LIRHideReg hr1(this, objectType); tmp1 = hr1.reg();
    LIRHideReg hr2(this, objectType); tmp2 = hr2.reg();
    LIRHideReg hr3(this, objectType); tmp3 = hr3.reg();
    LIRHideReg hr4(this, objectType); klassR = hr4.reg();
  }

  CodeEmitInfo* info = state_for(x, x->state());
  emit()->new_type_array(reg, x->elt_type(), length.result(), tmp1, tmp2, tmp3, norinfo, klassR, info);
}


void LIRGenerator::do_NewObjectArray(NewObjectArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  LIRItem length(x->length(), this);
  // in case of patching (i.e., object class is not yet loaded), we need to reexecute the instruction
  // and therefore provide the state before the parameters have been consumed
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info =  state_for(x, x->state_before());
  }

  length.load_item();
  RInfo reg = set_with_result_register(x)->rinfo();
  RInfo tmp1;
  RInfo tmp2;
  RInfo tmp3;
  RInfo tmp4;
  {
    LIRHideReg hr1(this, objectType); tmp1 = hr1.reg();
    LIRHideReg hr2(this, objectType); tmp2 = hr2.reg();
    LIRHideReg hr3(this, objectType); tmp3 = hr3.reg();
    LIRHideReg hr4(this, objectType); tmp4 = hr4.reg();
  }

  CodeEmitInfo* info = state_for(x, x->state());
  emit()->new_object_array(reg, x->klass(), length.result(), tmp1, tmp2, tmp3, tmp4, norinfo, info, patching_info);
}


void LIRGenerator::do_NewMultiArray(NewMultiArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());
  if (x->state_before() != NULL) {
    spill_values_on_stack(x->state_before());
  }
  Values* dims = x->dims();
  int i = dims->length();
  LIRItemList* items = new LIRItemList();
  while (i-- > 0) {
    LIRItem* size = new LIRItem(dims->at(i), this);
    items->at_put(i, size);
    assert(!size->is_register() || x->state_before() == NULL, "shouldn't be since it was spilled above");
  }

  // need to get the info before, as the items may become invalid through item_free

  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = state_for(x, x->state_before());
  }
  i = dims->length();
  while (i-- > 0) {
    LIRItem* size = items->at(i);
    size->load_item();

    emit()->store_stack_parameter(size->result(), i);
  }
  RInfo reg = set_with_result_register(x)->rinfo();

  CodeEmitInfo* info = state_for(x, x->state());
  emit()->new_multi_array(reg, x->klass(), x->rank(), norinfo, info, patching_info);
}


void LIRGenerator::do_BlockBegin(BlockBegin* x) {
  // nothing to do for now
}


void LIRGenerator::do_CheckCast(CheckCast* x) {
  // all values are spilled
  spill_values_on_stack(x->state());
  LIRItem obj(x->obj(), this);
  obj.set_destroys_register();
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    // must do this before locking the destination register as an oop register,
    // and before the obj is loaded (the latter is for deoptimization)
    patching_info = state_for(x, x->state_before());
  }
  obj.load_item();
  RInfo in_reg = obj.result()->rinfo();

  // info for exceptions
  CodeEmitInfo* info_for_exception = state_for(x, x->state()->copy_locks());

  RInfo reg = rlock_result(x)->rinfo();
  RInfo tmp1 = new_register(objectType)->rinfo();
  RInfo tmp2 = new_register(objectType)->rinfo();

  if (patching_info != NULL) {
    patching_info->add_register_oop(in_reg);
  }
  emit()->checkcast_op(LIR_OprFact::rinfo(reg, T_OBJECT), obj.result(), x->klass(), tmp1, tmp2, x->direct_compare(), info_for_exception, patching_info);
}


void LIRGenerator::do_InstanceOf(InstanceOf* x) {
  spill_values_on_stack(x->state());
  LIRItem obj(x->obj(), this);
  obj.set_destroys_register();
  // result and test object may not be in same register
  RInfo reg = rlock_result(x)->rinfo();
  CodeEmitInfo* patching_info = NULL;
  if ((!x->klass()->is_loaded() || PatchALot)) {
    // must do this before locking the destination register as an oop register
    patching_info = state_for(x, x->state_before());
  }
  obj.load_item();
  // do not include tmp in the oop map
  if (patching_info != NULL) {
    // object must be part of the oop map
    patching_info->add_register_oop(obj.result()->rinfo());
  }
  RInfo tmp = new_register(objectType)->rinfo();
  emit()->instanceof_op(LIR_OprFact::rinfo(reg, T_OBJECT), obj.result(), x->klass(), tmp, obj.result()->rinfo(), x->direct_compare(), patching_info);
}


void LIRGenerator::do_If(If* x) {
  assert(x->number_of_sux() == 2, "inconsistency");
  ValueTag tag = x->x()->type()->tag();
  bool is_safepoint = x->is_safepoint();

  If::Condition cond = x->cond();

  LIRItem xitem(x->x(), this);
  LIRItem yitem(x->y(), this);
  LIRItem* xin = &xitem;
  LIRItem* yin = &yitem;

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
  xin->load_item();
  if (tag == longTag && yin->is_constant() && yin->get_jlong_constant() == 0 && (cond == If::eql || cond == If::neq)) {
    // inline long zero
    yin->dont_load_item();
  } else if (tag == longTag || tag == floatTag || tag == doubleTag) {
    // longs cannot handle constants at right side
    yin->load_item();
  } else {
    yin->dont_load_item();
  }

  // note that the condition test must happen before the
  // moves into Phi area happen, and that the control flow
  // jump must happen after the moves into the phi area

  set_no_result(x);

  if (x->is_safepoint()) {
    CodeEmitInfo* info_before = state_for(x, x->state_before());
    emit()->safepoint_nop(info_before);
  }

  emit()->if_op(1, cond, xin->result(), yin->result(), x->tsux(), x->fsux(), x->usux());
  move_to_phi(x->state());
  emit()->if_op(2, cond, xin->result(), yin->result(), x->tsux(), x->fsux(), x->usux());

  goto_default_successor(x);
}


void LIRGenerator::do_IfInstanceOf(IfInstanceOf* x) {
  Unimplemented();
}


// May change the content of tag
void LIRGenerator::setup_phis_for_switch(LIR_Opr tag, ValueStack* stack) {
  // if stack() exists, then move_to_phi may destroy the tag register as
  // the tag register is not locked anymore.
  if (stack != NULL && stack->stack_size() > 0 ) {
    // stack exists, move values into phi locations
    // preserve the tag

    // save
    emit()->push_item(tag);

    move_to_phi(stack);
    emit()->pop_item(tag);
  }
}


void LIRGenerator::do_Return(Return* x) {
  if (x->type()->is_void()) {
    if (x->is_synchronized()) {
      emit()->return_op_prolog(x->monitor_no());
    }
    emit()->return_op(LIR_OprFact::illegalOpr);
  } else {
    RInfo reg = result_register_for(x->type())->rinfo();
    LIRItem result(x->result(), this);
    result.handle_float_kind();

    if (x->is_synchronized()) {
      emit()->return_op_prolog(x->monitor_no());
    }

    result.load_item_force(reg);
    emit()->return_op(result.result());
  }
  set_no_result(x);
}


void LIRGenerator::do_Base(Base* x) {
  emit()->std_entry(scope(), compilation()->get_init_vars(), receiverRInfo(), icKlassRInfo());
}


// Example: Thread.currentThread()
void LIRGenerator::do_currentThread(Intrinsic* x) {
  assert(x->number_of_arguments() == 0, "wrong type");
  RInfo result = rlock(x)->rinfo();
  emit()->lir()->get_thread(result);
  emit()->lir()->load_mem_reg(result, in_bytes(JavaThread::threadObj_offset()), result, T_OBJECT, NULL);
}


#endif // PRODUCT
