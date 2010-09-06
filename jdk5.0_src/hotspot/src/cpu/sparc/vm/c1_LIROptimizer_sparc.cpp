#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIROptimizer_sparc.cpp	1.29 03/12/23 16:37:02 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_LIROptimizer_sparc.cpp.incl"


bool LIR_Optimizer::optimize_move(LIR_Op1* move, LIR_Opr& src, LIR_Opr& dst) {
  if (move->patch_code() != LIR_Op1::patch_none) {
    src = maybe_opto(src, _state.equivalent_address(src));
    dst = maybe_opto(dst, _state.equivalent_address(dst));
    return false;
  }

  LIR_Opr new_src = src;
  if (dst->is_address() || dst->is_stack()) {
    LIR_Opr opr = _state.equivalent_register_or_constant(src);
    if (src->is_register() && opr->is_register()) {
      new_src = opr;
    } else {
      // the constant zero can be loaded from G0
      LIR_Const* c = opr->as_constant_ptr();
      if (c->type() == T_INT && c->as_jint() == 0) {
        new_src = LIR_OprFact::rinfo(FrameMap::_G0_RInfo);
      }
    }
  } else if (src->is_address()) {
    new_src = _state.equivalent_address(src);
  } else {
    new_src = _state.equivalent_register_or_constant(src);
  }

  // if we have a new src opr, make sure it's type compatible with the
  // dst so we don't mix fpu and cpu registers.
  if (src != new_src &&
      ((new_src->is_float_kind() && dst->is_float_kind()) ||
       (!new_src->is_float_kind() && !dst->is_float_kind()) ||
       new_src->is_address() || dst->is_address()) &&
      (!new_src->is_constant() || LIR_Assembler::is_small_constant(new_src))) {
    src = maybe_opto(src, new_src);
    if (src == new_src && dst->is_address()) {
      // make sure the type of the move is still correct
      if (move->_type == T_INT && src->type() == T_FLOAT) {
        move->_type = src->type();
      }
    }
  }

  if (dst->is_address()) {
    dst = maybe_opto(dst, _state.equivalent_address(dst));
  }

  return true;
}

// select_delay creates a new instruction to go into the delay slot.  Call
// instruction should pass their CodeEmitInfo info in so that the right debug
// information gets emitted since calls have both at_call and not_at_call states.
// branches should just pass NULL.

LIR_Op* LIR_Optimizer::select_delay(CodeEmitInfo* info) {
  if (LIRFillDelaySlots) {
    LIR_Op* slot = op_at(_op_index - 1);
    FrameMap* frame_map = ir()->compilation()->frame_map();
    if (slot && LIR_Assembler::is_single_instruction(slot, frame_map) &&
        _state.refcount(_op_index - 1) == 0 && allow_opto()) {
      // replace the instruction with a nop
      NOT_PRODUCT(_did_opto = true);
      replace_op(_op_index - 1, new LIR_Op0(lir_nop, NULL));
      _state.mark_safe_to_delete(_op_index - 1);
      return new LIR_OpDelay(slot, info);
    }
    return new LIR_OpDelay(new LIR_Op0(lir_nop, NULL), info);
  } else {
    return NULL;
  }
}


void LIR_Optimizer::delayed_emit(LIR_Op* op) {
#ifndef PRODUCT
  if (LIRTracePeephole && op->as_OpDelay()->delay_op()->code() != lir_nop) {
    tty->print("*delayed: ");
    op->as_OpDelay()->delay_op()->print();
  }
#endif // PRODUCT

  assert(op->as_OpDelay()->delay_op()->code() == lir_nop || LIRFillDelaySlots, "should be filling with nop unless LIRFillDelaySlots is true");
  LIR_OpList* lir = block()->lir()->instructions_list();
  assert(_op_index < lir->length(), "what?");
  lir->at_put_grow(lir->length(), NULL, NULL);
  for (int i = lir->length() - 2; i > _op_index; i--) {
    lir->at_put(i + 1, lir->at(i));
  }
  lir->at_put(_op_index + 1, op);
}

void LIR_Optimizer::emit_op0(LIR_Op0* op) {
  switch (op->code()) {
    case lir_word_align:
      // get rid of this because instructions area always word aligned
    case lir_empty_fpu:
    case lir_fpop_raw:
      // do nothing and we'll end up deleting this.
      break;

    default:
      visit();
  }
}


void LIR_Optimizer::emit_op1(LIR_Op1* op) {
  switch (op->code()) {
    case lir_fpu_push:
    case lir_fpu_dup:
    case lir_fpu_pop:
    case lir_push:
      // do nothing and we'll end up deleting this.
      break;

    case lir_branch:
      ShouldNotReachHere(); // cond_branch is handled as emit_opBranch
      break;

    case lir_volatile_move:
    case lir_move:
      process_move(op);
      break;

    case lir_array_move:
      result_substitute();
      visit();
      break;

    case lir_return: 
    case lir_neg:
    case lir_null_check:
    case lir_monaddr:
    case lir_rtcall:
    case lir_new_multi:
    case lir_safepoint:
      visit();
      break;

    default:
      ShouldNotReachHere();
      break;
  }
}


void LIR_Optimizer::emit_op2(LIR_Op2* op) {
  switch (op->code()) {

  case lir_logic_and:
  case lir_logic_or:
  case lir_logic_xor:
  case lir_shl:
  case lir_shr:
  case lir_add:
  case lir_sub:
  case lir_mul: {
    op->_opr1 = maybe_opto(op->in_opr1(), _state.equivalent_register(op->in_opr1()));
    op->_opr2 = maybe_opto(op->in_opr2(), _state.equivalent_register(op->in_opr2()));
    result_substitute();
    visit();
    break;
  }
  
  case lir_cmp: {
    LIR_Opr new_opr1 = _state.equivalent_register(op->in_opr1());
    LIR_Opr new_opr2 = LIR_OprFact::illegalOpr;
    if (op->_opr2->is_float_kind()) {
      new_opr2 = _state.equivalent_register(op->in_opr2());
    } else {
      LIR_Opr maybe_opr = _state.equivalent_register_or_constant(op->in_opr2());
      if (maybe_opr->is_register() || LIR_Assembler::is_small_constant(maybe_opr)) {
        new_opr2 = maybe_opr;
      }
    }

    // make sure the types match up
    if ((new_opr1->is_float_kind() && op->in_opr1()->is_float_kind()) ||
        (!new_opr1->is_float_kind() && !op->in_opr1()->is_float_kind())) {
      op->_opr1 = maybe_opto(op->in_opr1(), new_opr1);
    }
    if ((new_opr2->is_float_kind() && op->in_opr2()->is_float_kind()) ||
        (!new_opr2->is_float_kind() && !op->in_opr2()->is_float_kind())) {
      op->_opr2 = maybe_opto(op->in_opr2(), new_opr2);
    }

    visit();
    break;
  }


  default:
    visit();
  }
}


void LIR_Optimizer::emit_op3(LIR_Op3* op) {
  switch (op->code()) {
  case lir_irem:
  case lir_idiv: {
    op->_opr1 = maybe_opto(op->in_opr1(), _state.equivalent_register(op->in_opr1()));
    op->_opr2 = maybe_opto(op->in_opr2(), _state.equivalent_register(op->in_opr2()));
    result_substitute();
    visit();
    break;
  }

  default:
    ShouldNotReachHere();
  }
}


void LIR_Optimizer::emit_opBranch(LIR_OpBranch* op) {
  if (op->info() != NULL) {
    // only fill delay slots on non safepoint branches
    visit();
    delayed_emit(new LIR_OpDelay(new LIR_Op0(lir_nop, NULL), NULL));
    return;
  }

  FrameMap* frame_map = ir()->compilation()->frame_map();
  LIR_Op* delay_op = select_delay(NULL);
  LIR_Op* next = op_at(_op_index + 1);
  // if select_delay couldn't find an appropriate instruction see if the instruction
  // after the current one can be placed in the delay slot.  This is only appropriate for
  // branches to stubs which don't return.
  if (op->stub() != NULL && op->stub()->is_exception_throw_stub() &&
      delay_op != NULL && delay_op->as_OpDelay()->delay_op()->code() == lir_nop &&
      next && next->code() != lir_move && LIR_Assembler::is_single_instruction(next, frame_map) && allow_opto()) {
    assert(next->info() == NULL, "shouldn't have info");
    replace_op(_op_index + 1, new LIR_OpDelay(next, op->info()));
    assert(_pending_delay == false, "should have been cleared already");
    _pending_delay = true;
    visit();
  } else {
    visit();
    delayed_emit(delay_op);
  }
}


void LIR_Optimizer::emit_opLabel(LIR_OpLabel* op) {
  visit();
}


void LIR_Optimizer::emit_arraycopy(LIR_OpArrayCopy* op) {
  visit();
}


void LIR_Optimizer::emit_opConvert(LIR_OpConvert* op) {
  assert(op->_opr->is_valid(), "used");          _visit_state.do_input(op->_opr);
  assert(op->_result->is_valid(), "used");       _visit_state.do_input(op->_result);
                                                 _visit_state.do_output(op->_result);
}


void LIR_Optimizer::emit_alloc_obj(LIR_OpAllocObj* op) {
  visit();
}


void LIR_Optimizer::emit_alloc_array(LIR_OpAllocArray* op) {
  visit();
}


void LIR_Optimizer::emit_opTypeCheck(LIR_OpTypeCheck* op) {
  switch (op->code()) {
  case lir_instanceof:
  case lir_checkcast:
    op->_object = maybe_opto(op->object(), _state.equivalent_register(op->object()));
    break;
  }
  visit();
}


void LIR_Optimizer::emit_compare_and_swap(LIR_OpCompareAndSwap* op) {
  visit();
}


void LIR_Optimizer::emit_lock(LIR_OpLock* op) {
  visit();
}


void LIR_Optimizer::emit_rtcall(LIR_OpRTCall* op) {
  visit();
}

void LIR_Optimizer::emit_call(LIR_OpJavaCall* op) {
  // don't put anything in the delay slot of calls yet
  LIR_Op* delay_op = new LIR_OpDelay(new LIR_Op0(lir_nop, NULL), op->info());
  visit();
  delayed_emit(delay_op);
}


void LIR_Optimizer::emit_code_stub(CodeStub* op) {
  visit();
}

void LIR_Optimizer::emit_delay(LIR_OpDelay* op) {
  if (_pending_delay) {
    _pending_delay = false;
    _visit_state.set_op(op->delay_op());
    op->delay_op()->emit_code(this);
    _visit_state.set_op(op);
  }
}

