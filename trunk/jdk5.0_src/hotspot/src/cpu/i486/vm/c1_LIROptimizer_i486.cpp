#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIROptimizer_i486.cpp	1.18 03/12/23 16:36:08 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

# include "incls/_precompiled.incl"
# include "incls/_c1_LIROptimizer_i486.cpp.incl"


bool LIR_Optimizer::optimize_move(LIR_Op1* move, LIR_Opr& src, LIR_Opr& dst) {
  if (move->patch_code() != LIR_Op1::patch_none) {
    return false;
  }

  // some bool and byte moves may require particular registers
  // so don't modify the src unless it's not byte or bool
  bool substitute_result = false;
  if (move->type() != T_BOOLEAN && move->type() != T_BYTE) {
    LIR_Opr new_src;
    if (src->is_address()) {
      new_src = _state.equivalent_address(src);
    } else { 
      new_src = _state.equivalent_register_or_constant(src);
    }
    if (new_src->is_constant()) {
      LIR_Const* c = new_src->as_constant_ptr();
      if (move->type() == c->type()) {
        src = maybe_opto(src, new_src);
      }
    } else {
      src = maybe_opto(src, new_src);
    }
    substitute_result = true;
  }
  dst = maybe_opto(dst, _state.equivalent_address(dst));
  return substitute_result;
}


void LIR_Optimizer::emit_op0(LIR_Op0* op) {
  visit();
}


void LIR_Optimizer::emit_op1(LIR_Op1* op) {
  switch (op->code()) {
    case lir_volatile_move:
    case lir_move:
      process_move(op);
      break;

    case lir_push:
      op->_opr = maybe_opto(op->_opr, _state.equivalent_register_or_constant(op->_opr));
      visit();
      break;

  default:
    visit();
  }
}



void LIR_Optimizer::emit_op2(LIR_Op2* op) {
  bool is_commutative = false;
  switch (op->code()) {
  case lir_logic_orcc:
    op->_opr2 = maybe_opto(op->in_opr2(), _state.equivalent_register(op->in_opr2()));
    // customize the visiting since the default one doesn't work
    assert(op->_opr1->is_valid() && op->_opr2->is_valid() && op->_result->is_valid(), "must all be valid for this op");
    if (op->_info)                     _visit_state.do_info(op->_info);

    // if src1 and dst don't match then src1 is effectively a temporary
    if (op->in_opr1() != op->result_opr()) {
      _visit_state.do_temp(op->_opr1);
    } else {
      _visit_state.do_input(op->_opr1);
    }
    _visit_state.do_input(op->_opr2);
    _visit_state.do_output(op->_result);
    break;

  case lir_logic_and:
  case lir_logic_or:
  case lir_logic_xor:
  case lir_add:
  case lir_mul:
    is_commutative = true;

  case lir_sub:
    op->_opr2 = maybe_opto(op->in_opr2(), _state.equivalent_register(op->in_opr2()));
    // fall through

  case lir_ushr:
  case lir_shl:
  case lir_shr: {
    LIR_Opr new_src;
    if ((op->code() == lir_add || op->code() == lir_sub) &&
        !op->in_opr2()->is_stack() && op->in_opr1()->is_equivalent(op->result_opr())) {
      // support instructions which can reference memory directly
      new_src = maybe_opto(op->in_opr1(), _state.equivalent_register_or_stack(op->in_opr1()));
    } else {
      new_src = maybe_opto(op->in_opr1(), _state.equivalent_register(op->in_opr1()));
    }
    LIR_Op* next = next_op_with_code(lir_move);
    if (next != NULL && !op->result_opr()->is_float_kind()) {
      LIR_Op1* move = next->as_Op1();
      assert(move != NULL, "must have changed type");
      
      if (move->in_opr() == op->result_opr()) {
        LIR_Opr result_reg = move->result_opr();
        if (is_cached(result_reg)) {
          result_reg = replace_stack_opr(result_reg);
        }
        LIR_Opr opr2_reg = op->_opr2;
        if (is_cached(opr2_reg)) {
          opr2_reg = replace_stack_opr(opr2_reg);
        }
        //
        // There are several constructs we are trying to optimize here
        //
        // move [esi] [eax]
        // add  [eax] [ebx] [eax]
        // move [eax] [esi]
        //
        // can become 
        //
        // move [esi] [eax]
        // add  [esi] [ebx] [esi]
        // move [esi] [eax]
        //
        // Later those moves are likely to be eliminated because the
        // values aren't used.  On intel we can also do the next
        // optimization for some instructions which can directly reference memory.
        //
        // move [stack:1] [esi]
        // add  [esi] [ebx] [esi]
        // move [esi] [stack:1]
        //
        // can become 
        //
        // move [stack:1] [esi]
        // add  [stack:1] [ebx] [stack:1]
        // move [stack:1] [esi]
        //
        // Again it's likely that the moves to esi will be eliminated.
        //
        // add  [ebx] [stack:1] [eax]
        // move [eax] [stack:1]
        // 
        // can become
        //
        // add  [stack:1] [ebx] [stack:1]
        // move [stack:1] [ebx]


        if ((result_reg->is_register() || result_reg->is_stack()) &&
            (result_reg == new_src || (is_commutative && opr2_reg == result_reg)) && allow_opto()) {
          if (opr2_reg == result_reg && is_commutative) {
            op->_opr2 = new_src;
          }
          result_reg = _state.equivalent_register(result_reg);
          op->_opr1 = result_reg;
          op->_result = result_reg;
          NOT_PRODUCT(_did_opto = true);
          
          // reverse the move
          move->_result = move->_opr;
          move->_opr = result_reg;
          _state.mark_safe_to_delete(_op_index + 1);
        }
      }
    }

    if (op->in_opr1() != op->result_opr()) {
      // src1 and dst don't match so let's see if we swap src1 and
      // src2 so we don't need to emit an extra move.
      if ((op->code() == lir_add || op->code() == lir_mul) &&
          op->result_opr() == op->in_opr1() && allow_opto()) {
        NOT_PRODUCT(_did_opto = true);
        LIR_Opr opr = op->_opr1;
        op->_opr1 = op->_opr2;
        op->_result = opr;
      }
    }

    // customize the visiting since the default one doesn't work
    assert(op->_opr1->is_valid() && op->_opr2->is_valid() && op->_result->is_valid(), "must all be valid for this op");
    if (op->_info)                     _visit_state.do_info(op->_info);

    // if src1 and dst don't match then src1 is effectively a temporary
    if (!op->in_opr1()->is_equivalent(op->result_opr())) {
      _visit_state.do_temp(op->_opr1);
    } else {
      _visit_state.do_input(op->_opr1);
    }
    _visit_state.do_input(op->_opr2);
    _visit_state.do_output(op->_result);
    break;
  }
  
  case lir_cmp: {
    LIR_Opr new_opr1 = _state.equivalent_register(op->in_opr1());
    LIR_Opr new_opr2 = LIR_OprFact::illegalOpr;
    if (op->_opr2->is_float_kind()) {
      new_opr2 = _state.equivalent_register(op->in_opr2());
    } else {
      new_opr2 = _state.equivalent_register_or_constant(op->in_opr2());
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
    // if _opr2 is a constant, this op will have no CodeEmitInfo and opr3 will be invalid
    assert(op->_info != NULL || op->_opr2->is_constant(), "must have info"); _visit_state.do_info(op->_info);
    assert(op->_opr1->is_valid(), "used");       _visit_state.do_temp(op->_opr1);
    assert(op->_opr2->is_valid(), "used");       _visit_state.do_temp(op->_opr2);
    assert(op->_opr3->is_valid() || op->_opr2->is_constant(), "used");       _visit_state.do_temp(op->_opr3);
    assert(op->_result->is_valid(), "used");     _visit_state.do_output(op->_result);
    break;
  }

  default:
    ShouldNotReachHere();
  }
}


void LIR_Optimizer::emit_opBranch(LIR_OpBranch* op) {
  visit();
}


void LIR_Optimizer::emit_opLabel(LIR_OpLabel* op) {
  visit();
}


void LIR_Optimizer::emit_arraycopy(LIR_OpArrayCopy* op) {
  visit();
}


void LIR_Optimizer::emit_opConvert(LIR_OpConvert* op) {
  visit();
}


void LIR_Optimizer::emit_alloc_obj(LIR_OpAllocObj* op) {
  visit();
}


void LIR_Optimizer::emit_alloc_array(LIR_OpAllocArray* op) {
  visit();
}


void LIR_Optimizer::emit_opTypeCheck(LIR_OpTypeCheck* op) {
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
  visit();
}


void LIR_Optimizer::emit_code_stub(CodeStub* op) {
  visit();
}


