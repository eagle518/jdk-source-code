#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIRGenerator_sparc.cpp	1.8 04/04/20 15:56:18 JVM"
#endif
//
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
//

#ifndef PRODUCT

# include "incls/_precompiled.incl"
# include "incls/_c1_LIRGenerator_sparc.cpp.incl"


void LIRItem::load_byte_item() {
  // byte loads use same registers as other loads
  load_item();
}


void LIRItem::load_nonconstant() {
  if (result()->is_constant() && result()->type() == T_INT &&
      Assembler::is_simm13(result()->as_constant_ptr()->as_jint())) {
    dont_load_item();
  } else {
    load_item();
  }
}


//--------------------------------------------------------------
//               LIRGenerator
//--------------------------------------------------------------

RInfo LIRGenerator::exceptionOopRInfo()              { return FrameMap::_Oexception_RInfo; }
RInfo LIRGenerator::exceptionPcRInfo()               { return FrameMap::_Oissuing_pc_RInfo; }
RInfo LIRGenerator::return1RInfo()                   { return FrameMap::_O0_RInfo; }
RInfo LIRGenerator::callee_return1RInfo()            { return FrameMap::_I0_RInfo; }
RInfo LIRGenerator::return2RInfo()                   { return FrameMap::_O0_O1_RInfo; }
RInfo LIRGenerator::callee_return2RInfo()            { return FrameMap::_I0_I1_RInfo; }
RInfo LIRGenerator::returnF0RInfo()                  { return FrameMap::_F0_RInfo; }
RInfo LIRGenerator::returnD0RInfo()                  { return FrameMap::_F0_double_RInfo; }
RInfo LIRGenerator::receiverRInfo()                  { return FrameMap::_O0_RInfo; }
RInfo LIRGenerator::callee_receiverRInfo()           { return FrameMap::_I0_RInfo; }
RInfo LIRGenerator::nonReturnRInfo()                 { return FrameMap::_I0_RInfo; }
RInfo LIRGenerator::icKlassRInfo()                   { return FrameMap::_GIC_RInfo; }


//-----------------------spilling--------------------------------------------------------


// returns true if reg could be smashed by a callee.
bool LIRGenerator::is_caller_save_register(RInfo reg) {
  return FrameMap::is_caller_save_register(reg);
}



//--------- loading items into registers --------------------------------

// This method is used for proper handling of i486 FPU registers
void LIRGenerator::check_float_register(LIR_Opr item) {
  // do nothing
}


// No FPU register fanout needed
// This method is used for proper handling of i486 FPU registers
bool LIRGenerator::fpu_fanout_handled() {
  return false;
}


// SPARC cannot inline all constants
bool LIRGenerator::can_inline_any_constant() const {
  return false;
}


// only simm13 constants can be inlined
bool LIRGenerator:: can_inline_as_constant(Value i) const {
  return is_simm13(i->operand());
}


bool LIRGenerator::prefer_alu_registers() const {
  return true;
}


RInfo LIRGenerator::scratch1_RInfo() {
  return FrameMap::_G1_RInfo; // no scratch registers on i486
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

  array.load_item();
  index.load_nonconstant();
  value.load_item();
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
      RInfo tmp1 = FrameMap::_G1_RInfo;
      RInfo tmp2 = FrameMap::_G3_RInfo;
      RInfo tmp3 = FrameMap::_G5_RInfo;

      emit()->array_store_check(array.result(), value.result(), tmp1, tmp2, tmp3, range_check_info);
    }
  }
  emit()->indexed_store(x->elt_type(), array.result(), index.result(), value.result(), FrameMap::_G1_RInfo, null_check_info);
}


void LIRGenerator::do_MonitorEnter(MonitorEnter* x) {
  spill_values_on_stack(x->state());
  assert(x->is_root(),"");
  LIRItem obj(x->obj(), this);
  obj.load_item();

  set_no_result(x);

  RInfo lock    = FrameMap::_G1_RInfo;
  RInfo scratch = FrameMap::_G3_RInfo;
  RInfo hdr     = FrameMap::_G4_RInfo;

  CodeEmitInfo* info_for_exception = NULL;
  if (x->needs_null_check()) {
    info_for_exception = state_for(x, x->lock_stack_before());
  }
  CodeEmitInfo* info = state_for(x, x->state());
  emit()->monitor_enter(obj.get_register(), lock, hdr, scratch, x->monitor_no(), info_for_exception, info);
}


void LIRGenerator::do_MonitorExit(MonitorExit* x) {
  spill_values_on_stack(x->state());
  assert(x->is_root(),"");
  LIRItem obj(x->obj(), this);
  obj.dont_load_item();

  set_no_result(x);
  RInfo lock = FrameMap::_G1_RInfo;
  RInfo hdr  = FrameMap::_G3_RInfo;
  RInfo obj_temp  = FrameMap::_G4_RInfo;
  emit()->monitor_exit(obj_temp, lock, hdr, x->monitor_no());
}


// _ineg, _lneg, _fneg, _dneg
void LIRGenerator::do_NegateOp(NegateOp* x) {
  LIRItem value(x->x(), this);
  value.load_item();
  RInfo reg = rlock_result(x)->rinfo();
  emit()->negate(reg, value.result());
}



// for  _fadd, _fmul, _fsub, _fdiv, _frem
//      _dadd, _dmul, _dsub, _ddiv, _drem
void LIRGenerator::do_ArithmeticOp_FPU(ArithmeticOp* x) {
  switch (x->op()) {
  case Bytecodes::_fadd:
  case Bytecodes::_fmul:
  case Bytecodes::_fsub:
  case Bytecodes::_fdiv:
  case Bytecodes::_dadd:
  case Bytecodes::_dmul:
  case Bytecodes::_dsub:
  case Bytecodes::_ddiv: {
    LIRItem left(x->x(), this);
    LIRItem right(x->y(), this);
    left.load_item();
    right.load_item();
    RInfo reg = rlock_result(x)->rinfo();
    emit()->arithmetic_op_fpu(x->op(), x->operand(), left.result(), right.result(), x->is_strictfp());
  }
  break;

  case Bytecodes::_frem:
  case Bytecodes::_drem: {

    LIRItem left(x->x(), this);
    LIRItem right(x->y(), this);

#ifdef _LP64
    // The 64 bit ABI passed arguments in Float/Double registers.
    // There is no need to move them to int registers.
    const RInfo left_reg    = (x->op() == Bytecodes::_frem) ? FrameMap::_F1_RInfo : FrameMap::_F0_double_RInfo;
    const RInfo right_reg   = (x->op() == Bytecodes::_frem) ? FrameMap::_F3_RInfo : FrameMap::_F2_double_RInfo;
    const RInfo result_reg  = (x->op() == Bytecodes::_frem) ? FrameMap::_F0_RInfo : FrameMap::_F0_double_RInfo;

    left.load_item_force(left_reg);
    right.load_item_force(right_reg);
#else
    // There are no sparc machine instructions to move a value from a
    // floating point register to an int register or vice-versa.
    // At this point left & might may reside in fp registers. They have to
    // be moved to the appropiate O registers for the C call.
    const RInfo left_reg    = (x->op() == Bytecodes::_frem) ? FrameMap::_O0_RInfo : FrameMap::_O0_O1_RInfo;
    const RInfo right_reg   = (x->op() == Bytecodes::_frem) ? FrameMap::_O1_RInfo : FrameMap::_O2_O3_RInfo;
    const RInfo result_reg  = (x->op() == Bytecodes::_frem) ? FrameMap::_F0_RInfo : FrameMap::_F0_double_RInfo;

    left.dont_load_item();
    right.dont_load_item();
    emit()->move(left.result(),  LIR_OprFact::rinfo(left_reg));
    emit()->move(right.result(), LIR_OprFact::rinfo(right_reg));
#endif

    set_result(x, result_reg);

    LIRHideReg temp(this, FrameMap::callee_save_regs());
    emit()->arithmetic_call_op(x->op(), temp.reg());
  }
  break;

  default: ShouldNotReachHere();
  }
}


// for  _ladd, _lmul, _lsub, _ldiv, _lrem
void LIRGenerator::do_ArithmeticOp_Long(ArithmeticOp* x) {
  switch (x->op()) {
  case Bytecodes::_lrem:
  case Bytecodes::_lmul:
  case Bytecodes::_ldiv: {

    LIRItem left(x->x(), this);
    LIRItem right(x->y(), this);

    const RInfo left_reg   = FrameMap::_O2_O3_RInfo;
    const RInfo right_reg  = FrameMap::_O0_O1_RInfo;
    const RInfo result_reg = FrameMap::_O0_O1_RInfo;
    left.load_item_force(left_reg);
    right.load_item_force(right_reg);

    if (x->op() == Bytecodes::_ldiv || x->op() == Bytecodes::_lrem) {
      CodeEmitInfo* info = state_for(x);
      emit()->explicit_div_by_zero_check(right.result(), info);
    }

    LIRHideReg temp(this, FrameMap::callee_save_regs());
    set_result(x,result_reg);

    emit()->arithmetic_call_op(x->op(), temp.reg());
    break;
  }
  case Bytecodes::_ladd:
  case Bytecodes::_lsub: {
    LIRItem left(x->x(), this);
    LIRItem right(x->y(), this);
    left.load_item();
    right.load_item();
    RInfo reg = rlock_result(x)->rinfo();

    emit()->arithmetic_op_long(x->op(), x->operand(), left.result(), right.result(), NULL);
    break;
  }
  default: ShouldNotReachHere();
  }
}


// Returns if item is an int constant that can be represented by a simm13
bool LIRGenerator::is_simm13(LIR_Opr item) const {
  if (item->is_constant() && item->type() == T_INT) {
    return Assembler::is_simm13(item->as_constant_ptr()->as_jint());
  } else {
    return false;
  }
}


// for: _iadd, _imul, _isub, _idiv, _irem
void LIRGenerator::do_ArithmeticOp_Int(ArithmeticOp* x) {
  bool is_div_rem = x->op() == Bytecodes::_idiv || x->op() == Bytecodes::_irem;
  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
  // missing test if instr is commutative and if we should swap
  right.load_nonconstant();
  assert(right.is_constant() || right.is_register(), "wrong state of right");
  left.load_item();
  RInfo reg = rlock_result(x)->rinfo();
  if (is_div_rem) {
    CodeEmitInfo* info = state_for(x);
    emit()->arithmetic_idiv(x->op(), x->operand(), left.result(), right.result(), FrameMap::_G1_RInfo, info);
  } else {
    emit()->arithmetic_op_int(x->op(), x->operand(), left.result(), right.result(), FrameMap::_G1_RInfo);
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
  LIRItem value(x->x(), this);
  LIRItem count(x->y(), this);
  // Long shift destroys count register
  if (value.type()->is_long()) {
    count.set_destroys_register();
  }
  value.load_item();
  // the old backend doesn't support this
  if (count.is_constant() && count.type()->as_IntConstant() != NULL && value.type()->is_int()) {
    jint c = count.get_jint_constant() & 0x1f;
    assert(c >= 0 && c < 32, "should be small");
    count.dont_load_item();
  } else {
    count.load_item();
  }
  RInfo reg = rlock_result(x)->rinfo();
  emit()->shift_op(x->op(), reg, value.result(), count.result(), norinfo);
}


// _iand, _land, _ior, _lor, _ixor, _lxor
void LIRGenerator::do_LogicOp(LogicOp* x) {
  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
  // missing test if instr is commutative and if we should swap
  left.load_item();
  right.load_nonconstant();
  RInfo reg = rlock_result(x)->rinfo();
  emit()->logic_op(x->op(), reg, left.result(), right.result());
}



// _lcmp, _fcmpl, _fcmpg, _dcmpl, _dcmpg
void LIRGenerator::do_CompareOp(CompareOp* x) {
  LIRItem left(x->x(), this);
  LIRItem right(x->y(), this);
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
  switch (x->id()) {
    case methodOopDesc::_dsqrt: {
      assert(x->number_of_arguments() == 1, "wrong type");
      LIRItem value(x->argument_at(0), this);
      value.load_item();
      RInfo result_reg = rlock_result(x)->rinfo();
      emit()->math_intrinsic(x->id(), result_reg, value.result());
      break;
    }
    case methodOopDesc::_dsin: // fall through
    case methodOopDesc::_dcos: {
      assert(x->number_of_arguments() == 1, "wrong type");
      // Note: spill caller save before setting the item

      LIRItem value(x->argument_at(0), this);
      value.set_destroys_register();


#ifdef _LP64
      // 64 bit Sparc ABI passes first double in D0
      RInfo value_reg  = FrameMap::_F0_double_RInfo;
      RInfo result_reg = FrameMap::_F0_double_RInfo;
      value.load_item_force(value_reg);
#else

      // There are no sparc machine instructions to move a value from a
      // floating point register to an int register or vice-versa.
      // At this point value may reside in an fp register. It has to
      // be moved to the appropiate O registers for the C call.
      RInfo value_reg  = FrameMap::_O0_O1_RInfo;
      RInfo result_reg = FrameMap::_F0_double_RInfo;

      value.dont_load_item();
      emit()->move(value.result(), LIR_OprFact::rinfo(value_reg));
#endif

      set_result(x, result_reg);
      LIRHideReg thread(this, FrameMap::callee_save_regs());
      emit()->math_intrinsic(x->id(), result_reg, value.result(), thread.reg());

    }
  }
}


void LIRGenerator::do_ArrayCopy(Intrinsic* x) {
  assert(x->number_of_arguments() == 5, "wrong type");
  // Note: spill caller save before setting the item
  LIRItem src     (x->argument_at(0), this);
  LIRItem src_pos (x->argument_at(1), this);
  LIRItem dst     (x->argument_at(2), this);
  LIRItem dst_pos (x->argument_at(3), this);
  LIRItem length  (x->argument_at(4), this);
  // load all values in callee_save_registers, as this makes the
  // parameter passing to the fast case simpler
  src.load_item_with_reg_mask(FrameMap::callee_save_regs());
  src_pos.load_item_with_reg_mask(FrameMap::callee_save_regs());
  dst.load_item_with_reg_mask(FrameMap::callee_save_regs());
  dst_pos.load_item_with_reg_mask(FrameMap::callee_save_regs());
  length.load_item_with_reg_mask(FrameMap::callee_save_regs());
  
  ciType* expected_type = src.value()->exact_type();
  bool is_exact = false;
  if (expected_type != NULL && expected_type == dst.value()->exact_type()) {
    // the types exactly match so the type is fully known
    is_exact = true;
  } else {
    // at least pass along a good guess
    if (expected_type == NULL) {
      expected_type = src.value()->exact_type();
    }
    if (expected_type == NULL) {
      expected_type = src.value()->declared_type();
    }
    if (expected_type == NULL) {
      expected_type = dst.value()->declared_type();
    }
  }
  
  { LIRHideReg thread(this, FrameMap::callee_save_regs());
  CodeEmitInfo* info = state_for(x, x->state()); // we may want to have stack (deoptimization?)
  emit()->arraycopy(src.result(), src_pos.result(), dst.result(), dst_pos.result(), length.result(), thread.reg(), is_exact, expected_type, info); // does add_safepoint
  }
  set_no_result(x);
}

// _i2l, _i2f, _i2d, _l2i, _l2f, _l2d, _f2i, _f2l, _f2d, _d2i, _d2l, _d2f
// _i2b, _i2c, _i2s
void LIRGenerator::do_Convert(Convert* x) {
  switch (x->op()) {
    case Bytecodes::_f2l:
    case Bytecodes::_d2l:
    case Bytecodes::_d2i: {

      LIRItem value(x->value(), this);

#ifdef _LP64
      // Make sure value is in correct floating point register for call
      RInfo value_reg  = (value.type()->tag() == floatTag ? FrameMap::_F1_RInfo :
                              FrameMap::_F0_double_RInfo);
      value.load_item_force(value_reg);
#else
      // There are no sparc machine instructions to move a value from a
      // floating point register to an int register or vice-versa.
      // At this point value may reside in an fp register. It has to
      // be moved to the appropiate O registers for the C call.
      RInfo value_reg  = (value.type()->tag() == floatTag ? FrameMap::_O0_RInfo :
                        FrameMap::_O0_O1_RInfo);
      value.dont_load_item();

      emit()->move(value.result(), LIR_OprFact::rinfo(value_reg));
#endif
      RInfo result_reg = (x->op() == Bytecodes::_d2i ? FrameMap::_O0_RInfo : FrameMap::_O0_O1_RInfo);
      set_result(x, result_reg);
      {
        LIRHideReg thread(this, FrameMap::callee_save_regs());
        emit()->call_convert_op(x->op(), thread.reg());
      }
      break;
    }

    case Bytecodes::_l2f:
    case Bytecodes::_l2d: {

      LIRItem value(x->value(), this);
      // XXX TKR HintLIRItem value_hint(x->value()->type(), FrameMap::_O0_O1_RInfo, this);

      // force long value into o0/o1 for the C call
      value.load_item_force(FrameMap::_O0_O1_RInfo);

      { // thread register must be callee saved
        LIRHideReg thread(this, FrameMap::callee_save_regs());

        // require the result to use the first float registers
        RInfo result_reg;
        switch (x->type()->tag()) {
          case floatTag:   result_reg = FrameMap::_F0_RInfo;        break;
          case doubleTag:  result_reg = FrameMap::_F0_double_RInfo; break;
          default: ShouldNotReachHere();
        }

        set_result(x, result_reg);
        emit()->call_convert_op(x->op(), thread.reg());
      }
    }
    break;

    case Bytecodes::_i2f:
    case Bytecodes::_i2d: {
      LIRItem value(x->value(), this);

      value.dont_load_item();
      RInfo reg = rlock_result(x)->rinfo();
      if (x->op() == Bytecodes::_i2d) {
        // To convert an int to double, we need to load the 32-bit int
        // from memory into a single precision floating point register
        // (even numbered). Then the sparc fitod instruction takes care
        // of the conversion. This is a bit ugly, but is the best way to
        // get the int value in a single precision floating point register
        RInfo float_reg = as_RInfo(reg.as_double_reg(), false);
        emit()->move(value.result(), LIR_OprFact::rinfo(float_reg));
        emit()->convert_op(x->op(), LIR_OprFact::rinfo(float_reg), reg);
      } else {
        emit()->move(value.result(), LIR_OprFact::rinfo(reg));
        emit()->convert_op(x->op(), LIR_OprFact::rinfo(reg), reg);
      }
      break;
    }
    break;

    case Bytecodes::_i2l:
    case Bytecodes::_i2b:
    case Bytecodes::_i2c:
    case Bytecodes::_i2s:
    case Bytecodes::_l2i:
    case Bytecodes::_f2d:
    case Bytecodes::_d2f: { // inline code
      LIRItem value(x->value(), this);

      value.load_item();
      RInfo reg = rlock_result(x)->rinfo();
      emit()->convert_op(x->op(), value.result(), reg);
    }
    break;

    case Bytecodes::_f2i: {
      LIRItem value (x->value(), this);
      value.set_destroys_register();
      value.load_item();
      RInfo reg = rlock_result(x)->rinfo();
      emit()->convert_op(x->op(), value.result(), reg);
    }
    break;

    default: ShouldNotReachHere();
  }
}


void LIRGenerator::do_CachingChange(CachingChange* x) {
  // do nothing
}

// Visits all arguments, returns appropriate items without loading them
LIRItemList* LIRGenerator::invoke_visit_arguments(Invoke* x, CallingConvention* arg_loc) {
  LIRItemList* argument_items = new LIRItemList();
  if (x->has_receiver()) {
    LIRItem* receiver = new LIRItem(x->receiver(), this);
    argument_items->append(receiver);
    ArgumentLocation loc = arg_loc->arg_at(0);
    assert(loc.is_register_arg() && loc.outgoing_reg_location().is_same(FrameMap::_O0_RInfo), "receiver must be in O0");
    // XXX TKR HintLIRItem hint(objectType, FrameMap::_O0_RInfo, this);
  }
  int idx = x->has_receiver() ? 1 : 0;
  for (int i = 0; i < x->number_of_arguments(); i++) {
    LIRItem* param = new LIRItem(x->argument_at(i), this);
    argument_items->append(param);
    ArgumentLocation loc = arg_loc->arg_at(idx);
    idx += (param->type()->is_double_word() ? 2 : 1);
//     XXX TKR HintLIRItem hint(x->argument_at(i)->type(), this);
//     if (loc.is_register_arg()) {
//       hint.set_register(loc.outgoing_reg_location());
//     } else {
//       hint.set_from_item(HintItem::no_hint());
//     }
  }
  return argument_items;
}


void LIRGenerator::invoke_load_arguments(Invoke* x, LIRItemList* args, CallingConvention* arg_loc) {
  LIRItemList register_args(FrameMap::nof_reg_args);
  int idx = x->has_receiver() ? 1 : 0;
  for (int i = 0; i < x->number_of_arguments(); i++) {
    LIRItem* param = args->at(x->has_receiver() ? i+1 : i);
    ArgumentLocation loc = arg_loc->arg_at(idx);
    if (loc.is_register_arg()) {
      param->load_item_force(loc.outgoing_reg_location());
      // don't free items for register arguments until all other arguments are loaded;
      // otherwise an argument register could be overwritten while initializing a stack argument
      register_args.append(param);
    } else {
      assert(loc.is_stack_arg(), "just checking");
      param->load_item();
      emit()->store_stack_parameter (param->result(), loc.stack_offset_in_words());
    }
    idx += (param->type()->is_double_word() ? 2 : 1);
  }
  if (x->has_receiver()) {
    LIRItem* receiver = args->at(0);
    ArgumentLocation loc = arg_loc->arg_at(0);
    assert(loc.is_register_arg(), "the receiver must be in O0");
    receiver->load_item_force(loc.outgoing_reg_location());
  }
}


// handle arguments, without receiver
void LIRGenerator::invoke_do_arguments(Invoke* x) {
  ShouldNotReachHere();
}



void LIRGenerator::invoke_do_spill(Invoke* x, RInfo receiver_reg) {
  // move caller save registers to callee save registers (saves the spill)
  // make sure that caller save registers are spilled
  spill_values_on_stack(x->state(), receiver_reg, true);
}


void LIRGenerator::invoke_do_result(Invoke* x, bool needs_null_check, LIR_Opr receiver) {
  // setup result register
  if (x->type()->is_void()) {
    set_no_result(x);
  } else {
    set_with_result_register(x);
  }
  // emit invoke code
  CodeEmitInfo* info = state_for(x, x->state());
  bool optimized = x->target_is_loaded() && x->target_is_final();
  emit()->call_op(x->code(), x->signature(), info, x->vtable_index(), optimized, needs_null_check, receiver == NULL ? norinfo : receiver->rinfo(), x->operand()); // does add_safepoint
}


// The invoke with receiver has following phases:
//   a) traverse and load/lock receiver;
//   b) traverse all arguments -> item-array (invoke_visit_argument)
//   c) push receiver on stack
//   d) load each of the items and push on stack
//   e) unlock receiver
//   f) move receiver into receiver-register %o0
//   g) lock result registers and emit call operation
//
// Before issuing a call, we must spill-save all values on stack
// that are in caller-save register. "spill-save" moves thos registers
// either in a free callee-save register or spills them if no free
// callee save register is available.
//
// The problem is where to invoke spill-save.
// - if invoked between e) and f), we may lock callee save
//   register in "spill-save" that destroys the receiver register
//   before f) is executed
// - if we rearange the f) to be earlier, by loading %o0, it
//   may destroy a value on the stack that is currently in %o0
//   and is waiting to be spilled
// - if we keep the receiver locked while doing spill-save,
//   we cannot spill it as it is spill-locked
//
void LIRGenerator::do_Invoke(Invoke* x) {
  CallingConvention* arg_loc = FrameMap::calling_convention(!x->has_receiver(), *x->signature());
  LIRItemList* args = invoke_visit_arguments(x, arg_loc);
  if (x->has_receiver()) {
    LIRItem* receiver = args->at(0);
    invoke_load_arguments(x, args, arg_loc);
    invoke_do_spill(x);

    bool needs_null_check = false;
    if (x->needs_null_check() ||
        !x->target_is_loaded() || x->target_is_final()) {
      // emit receiver NULL check if the class of target is not loaded (i.e., we cannot know if it is final)
      // or if the target is final
      if (PrintNotLoaded && !x->target_is_loaded()) {
        tty->print_cr("   ### class not loaded at invoke bci %d", x->bci());
      }
      needs_null_check = true;
    }
    invoke_do_result(x, needs_null_check, receiver->result());
  } else {
    invoke_load_arguments(x, args, arg_loc);
    invoke_do_spill(x);
    invoke_do_result(x, false);
  }
}


void LIRGenerator::do_NewInstance(NewInstance* x) {
  NEEDS_CLEANUP;
  // Note: change this to require any register as result of a new,
  //       instead of specifically %o0; do not spill all values.

  // make sure registers are spilled
  spill_values_on_stack(x->state());

  // This instruction can be deoptimized in the slow path : use
  // O0 as result register.
  const RInfo reg = set_with_result_register(x)->rinfo();

  if (PrintNotLoaded && !x->klass()->is_loaded()) {
    tty->print_cr("   ###class not loaded at new bci %d", x->bci());
  }
  CodeEmitInfo* info = state_for(x, x->state());
  emit()->new_instance(reg, x->klass(), FrameMap::_G1_RInfo, FrameMap::_G3_RInfo, FrameMap::_G4_RInfo, FrameMap::_G5_RInfo, info);
}


void LIRGenerator::do_NewTypeArray(NewTypeArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());

  LIRItem length(x->length(), this);
  length.load_item();

  // XXX TKR HintLIRItem result_hint(x->type(), result_register_for(x->type()), this);
  RInfo reg = rlock_result(x)->rinfo();

  {
    RInfo t1 = FrameMap::_G1_RInfo;
    RInfo t2 = FrameMap::_G3_RInfo;
    RInfo t3 = FrameMap::_G4_RInfo;
    RInfo t4 = FrameMap::_G5_RInfo;
    LIRHideReg klass_reg(this, objectType);
    CodeEmitInfo* info = state_for(x, x->state());
    emit()->new_type_array(reg, x->elt_type(), length.result(), t1, t2, t3, t4, klass_reg.reg(), info);
  }
}


void LIRGenerator::do_NewObjectArray(NewObjectArray* x) {
  // make sure registers are spilled
  NEEDS_CLEANUP // factor out, very similar
  spill_values_on_stack(x->state());

  LIRItem length(x->length(), this);
  // in case of patching (i.e., object class is not yet loaded), we need to reexecute the instruction
  // and therefore provide the state before the parameters have been consumed
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = state_for(x, x->state_before());
  }

  length.load_item();

  // XXX TKR HintLIRItem result_hint(x->type(), result_register_for(x->type()), this);
  RInfo reg = rlock_result(x)->rinfo();

  {
    RInfo t1 = FrameMap::_G1_RInfo;
    RInfo t2 = FrameMap::_G3_RInfo;
    RInfo t3 = FrameMap::_G4_RInfo;
    RInfo t4 = FrameMap::_G5_RInfo;
    LIRHideReg klass_reg(this, objectType);

    CodeEmitInfo* info = state_for(x, x->state());
    emit()->new_object_array(reg, x->klass(), length.result(), t1, t2, t3, t4, klass_reg.reg(), info, patching_info);
  }
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
  }

  // need to get the info before, as the items may become invalid through item_free
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = state_for(x, x->state_before());
  }

  intStack* free_after = new intStack();
  i = dims->length();
  while (i-- > 0) {
    LIRItem* size = items->at(i);
    // if a patching_info was generated above then debug information for the state before
    // the call is going to be emitted.  The LIRGenerator calls above may have left some values
    // in registers and that's been recorded in the CodeEmitInfo.  In that case the items
    // for those values can't simply be freed if they are registers because the values
    // might be destroyed by store_stack_parameter.  So in the case of patching, delay the
    // freeing of the items that already were in registers
    bool was_register = size->is_register();
    size->load_item();
    emit()->store_stack_parameter (size->result(), i + frame::memory_parameter_word_sp_offset);

    if (was_register && patching_info != NULL) {
      free_after->append(i);
    } else {
    }
  }

  // This instruction can be deoptimized in the slow path : use
  // O0 as result register.
  const RInfo reg = set_with_result_register(x)->rinfo();
  CodeEmitInfo* info = state_for(x, x->state());
  emit()->new_multi_array(reg, x->klass(), x->rank(), norinfo, info, patching_info);

  // free any items which were referenced from the x->state_before() debug information
  while (free_after->length() > 0) {
    int index = free_after->pop();
    LIRItem* size = items->at(index);
  }
}


void LIRGenerator::do_BlockBegin(BlockBegin* x) {
}


void LIRGenerator::do_CheckCast(CheckCast* x) {
  spill_values_on_stack(x->state());
  LIRItem obj(x->obj(), this);
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    // must do this before locking the destination register as an oop register,
    // and before the obj is loaded (so x->obj()->item() is valid for creating a debug info location)
    patching_info = state_for(x, x->state_before());
  }
  obj.load_item();
  const RInfo in_reg = obj.get_register();
  const RInfo out_reg = rlock_result(x)->rinfo();

  if (patching_info != NULL) {
    patching_info->add_register_oop(in_reg);
  }
  CodeEmitInfo* info_for_exception = state_for(x, x->state()->copy_locks());
  emit()->checkcast_op(LIR_OprFact::rinfo(out_reg, T_OBJECT), obj.result(), x->klass(), FrameMap::_G1_RInfo, FrameMap::_G3_RInfo, x->direct_compare(), info_for_exception, patching_info);
}


void LIRGenerator::do_InstanceOf(InstanceOf* x) {
  spill_values_on_stack(x->state());
  LIRItem obj(x->obj(), this);
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = state_for(x, x->state_before());
  }
  // ensure the result register is not the input register because the result is initialized before the patching safepoint
  obj.load_item();
  const RInfo in_reg = obj.get_register();
  const RInfo out_reg = rlock_result(x)->rinfo();
  if (patching_info != NULL) {
    // object must be part of the oop map
    patching_info->add_register_oop(in_reg);
  }
  emit()->instanceof_op(LIR_OprFact::rinfo(out_reg, T_OBJECT), obj.result(), x->klass(), FrameMap::_G1_RInfo, FrameMap::_G3_RInfo,  x->direct_compare(), patching_info);
}


void LIRGenerator::do_If(If* x) {
  assert(x->number_of_sux() == 2, "inconsistency");
  ValueTag tag = x->x()->type()->tag();
  LIRItem xitem(x->x(), this);
  LIRItem yitem(x->y(), this);
  LIRItem* xin = &xitem;
  LIRItem* yin = &yitem;
  If::Condition cond = x->cond();
  {  // call destructors before doing move to phi
    if (tag == longTag) {
      // for longs, only conditions "eql", "neq", "lss", "geq" are valid;
      // mirror for other conditions
      if (cond == If::gtr || cond == If::leq) {
        // swap inputs
        cond = Instruction::mirror(cond);
        xin = &yitem;
        yin = &xitem;
      }
      xin->set_destroys_register();
    }
    xin->load_item();
    if (is_simm13(yin->result())) {
      // inline int constants which are small enough to be immediate operands
      yin->dont_load_item();
    } else if (tag == longTag && yin->is_constant() && yin->get_jlong_constant() == 0 && (cond == If::eql || cond == If::neq)) {
      // inline long zero
      yin->dont_load_item();
    } else if (tag == objectTag && yin->is_constant() && (yin->get_jobject_constant()->is_null_object())) {
      yin->dont_load_item();
    } else {
      yin->load_item();
    }
  }

  // note that the condition test must happen before the
  // moves into Phi area happen, and that the control flow
  // jump must happen after the moves into the phi area

  // add safepoint before generating condition code so it can be recomputed
  CodeEmitInfo* info = NULL;
  if (x->is_safepoint()) {
    info = state_for(x, x->state_before());
  }
  set_no_result(x);

  // Note that in phase 2, the registers holding the testing values may have been destroyed
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


// May change the content of tag;
void LIRGenerator::setup_phis_for_switch(LIR_Opr tag, ValueStack* stack) {
  // if stack() exists, then move_to_phi may destroy the tag register as
  // the tag register is not locked anymore.
  if (stack != NULL && stack->stack_size() > 0 ) {
    // stack exists, move values into phi locations
    // preserve the tag

    emit()->set_bailout("setup_phis_for_switch not implemented");
    // save in G1
//     emit()->move(tag, FrameMap::_G1_RInfo);
//     move_to_phi(stack);
//     { LIRHideReg hr(this, nonReturnRInfo()); // use one register that is not a return register
//       tag->set_register(hr.reg());
//       // restore from G1(do not use return registers)
//       emit()->move(LIR_OprFact::rinfo(FrameMap::_G1_RInfo), tag);
//     }
  }
}


void LIRGenerator::do_Return(Return* x) {
  if (x->type()->is_void()) {
    if (x->is_synchronized()) {
      emit()->return_op_prolog(x->monitor_no());
    }
    emit()->return_op(LIR_OprFact::illegalOpr);
  } else {
    RInfo reg = result_register_for(x->type(), /*callee=*/true)->rinfo();
    LIRItem result(x->result(), this);
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
  RInfo result = rlock_result(x)->rinfo();
  assert(G2_thread == FrameMap::_G2_RInfo.as_register(), "update this");
  emit()->lir()->load_mem_reg(FrameMap::_G2_RInfo, in_bytes(JavaThread::threadObj_offset()), result, T_OBJECT, NULL);
}


#endif // PRODUCT
