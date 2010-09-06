#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CodeGenerator_sparc.cpp	1.99 04/04/20 15:56:19 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_CodeGenerator_sparc.cpp.incl"

#define __ emit()->lir()->

// Notes to OopMap generations:
// - because the pc-offset is attached to each OopMap, we must create
//   a new OopMap for each gc-point

//--------------------------------------------------------------
class SpillLockReg: public StackObj {
 private:
  ValueGen* _vg;
  RInfo     _reg;
 public:
  SpillLockReg::SpillLockReg(ValueGen* vg, RInfo reg): _vg(vg), _reg(reg) {
    vg->ra()->incr_spill_lock(reg);
  }
  SpillLockReg::~SpillLockReg() {
    _vg->ra()->decr_spill_lock(_reg);
  }

};


//--------------------------------------------------------------
//               ValueGen
//--------------------------------------------------------------

// returns true if reg could be smashed by a callee.
bool ValueGen::is_caller_save_register(RInfo reg) {
  return FrameMap::is_caller_save_register(reg);
}



RInfo ValueGen::exceptionOopRInfo()              { return FrameMap::_Oexception_RInfo; }
RInfo ValueGen::exceptionPcRInfo()               { return FrameMap::_Oissuing_pc_RInfo; }
RInfo ValueGen::return1RInfo()                   { return FrameMap::_O0_RInfo; }
RInfo ValueGen::callee_return1RInfo()            { return FrameMap::_I0_RInfo; }
RInfo ValueGen::return2RInfo()                   { return FrameMap::_O0_O1_RInfo; }
RInfo ValueGen::callee_return2RInfo()            { return FrameMap::_I0_I1_RInfo; }
RInfo ValueGen::returnF0RInfo()                  { return FrameMap::_F0_RInfo; }
RInfo ValueGen::returnD0RInfo()                  { return FrameMap::_F0_double_RInfo; }
RInfo ValueGen::receiverRInfo()                  { return FrameMap::_O0_RInfo; }
RInfo ValueGen::callee_receiverRInfo()           { return FrameMap::_I0_RInfo; }
RInfo ValueGen::nonReturnRInfo()                 { return FrameMap::_I0_RInfo; }
RInfo ValueGen::icKlassRInfo()                   { return FrameMap::_GIC_RInfo; }


//--------- loading items into registers --------------------------------

// This method is used for proper handling of i486 FPU registers
void ValueGen::check_float_register(Item* item) {
  // do nothing
}


// No FPU register fanout needed
// This method is used for proper handling of i486 FPU registers
bool ValueGen::fpu_fanout_handled(Item* item) {
  return false;
}


// SPARC cannot inline all constants
bool ValueGen::can_inline_any_constant() const {
  return false;
}


// only simm13 constants can be inlined
bool ValueGen:: can_inline_as_constant(Item* i) {
  return is_simm13(i);
}


bool ValueGen::prefer_alu_registers() const {
  return false;
}


bool ValueGen::safepoint_poll_needs_register() {
  return true;
}


void ValueGen::load_byte_item(Item* item) {
  // byte loads use same registers as other loads
  load_item(item);
}


RInfo ValueGen::rlock_byte_result_with_hint(Value instr, const Item* hint) {
  return rlock_result_with_hint(instr, hint);
}


RInfo ValueGen::scratch1_RInfo() const {
  return FrameMap::_G1_RInfo; // no scratch registers on i486
}


//----------------------------------------------------------------------
//             visitor functions
//----------------------------------------------------------------------


void ValueGen::do_StoreIndexed(StoreIndexed* x) {
  assert(x->is_root(),"");
  bool use_length = x->length() != NULL;

  Item  array(x->array());
  Item  index(x->index());
  Item  value(x->value());
  Item  length;
  if (use_length) {
    length.set_instruction(x->length());
  }
  ValueGen a(&array, HintItem::no_hint(), this);
  ValueGen i(&index, HintItem::no_hint(), this);
  ValueGen v(&value, HintItem::no_hint(), this);
  ValueGen l(&length, HintItem::no_hint(), this, true);

  load_item(&array);
  if (is_simm13(&index)) {
    dont_load_item(&index);
  } else { 
    load_item(&index); 
  }
  load_item(&value);
  if (use_length) {
    load_item(&length);
    item_free(&length);
  }
  set_no_result(x);


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
      // The range check performs the null check, so clear it out for the load
      null_check_info = NULL;
    }
  }

  if (value.type()->is_object() && GenerateArrayStoreCheck) {
    emit()->array_store_check(item2lir(&array), item2lir(&value), FrameMap::_G1_RInfo, FrameMap::_G3_RInfo, FrameMap::_G5_RInfo, range_check_info);
  }

  emit()->indexed_store(x->elt_type(), item2lir(&array), item2lir(&index), item2lir(&value), FrameMap::_G1_RInfo, null_check_info);

  // release items after the CodeEmitInfo has been emitted, otherwise the items may be set to invalid
  item_free(&index);
  item_free(&value);
  item_free(&array);
}


void ValueGen::do_MonitorEnter(MonitorEnter* x) {
  spill_values_on_stack(x->state());
  assert(x->is_root(),"");
  Item obj(x->obj());
  ValueGen o(&obj, HintItem::no_hint(), this);
  load_item(&obj);

  set_no_result(x);

  RInfo lock    = FrameMap::_G1_RInfo;
  RInfo scratch = FrameMap::_G3_RInfo;
  RInfo hdr     = FrameMap::_G4_RInfo;

  CodeEmitInfo* info_for_exception = NULL;
  if (x->needs_null_check()) {
    info_for_exception = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack_before(), x->exception_scope());
  }
  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope(), ra()->oops_in_registers());
  emit()->monitor_enter(obj.get_register(), lock, hdr, scratch, x->monitor_no(), info_for_exception, info);
  item_free(&obj);
}


void ValueGen::do_MonitorExit(MonitorExit* x) {
  spill_values_on_stack(x->state());
  assert(x->is_root(),"");
  Item obj(x->obj());
  ValueGen o(&obj, HintItem::no_hint(), this);
  dont_load_item(&obj);

  set_no_result(x);
  RInfo lock = FrameMap::_G1_RInfo;
  RInfo hdr  = FrameMap::_G3_RInfo;
  RInfo obj_temp  = FrameMap::_G4_RInfo;
  if (x->monitor_no() == -1) {
    warning("Unreachable monitor exit, wait for x86 merge to avoid compiling this bytecode");
  } else {
    emit()->monitor_exit(obj_temp, lock, hdr, x->monitor_no());
  }
  item_free(&obj); 
}


// _ineg, _lneg, _fneg, _dneg 
void ValueGen::do_NegateOp(NegateOp* x) {
  Item value(x->x());
  ValueGen v(&value, hint(), this);
  load_item_hint(&value, hint());
  RInfo reg = rlock_result_with_hint(x, hint());
  item_free(&value);
  emit()->negate(reg, item2lir(&value));
}



// for  _fadd, _fmul, _fsub, _fdiv, _frem
//      _dadd, _dmul, _dsub, _ddiv, _drem
void ValueGen::do_ArithmeticOp_FPU(ArithmeticOp* x) { 
  // don't create any items before possible spill_caller_save
  switch (x->op()) {
  case Bytecodes::_fadd:
  case Bytecodes::_fmul:
  case Bytecodes::_fsub:
  case Bytecodes::_fdiv:
  case Bytecodes::_dadd:
  case Bytecodes::_dmul:
  case Bytecodes::_dsub:
  case Bytecodes::_ddiv: {
    Item left(x->x());
    Item right(x->y());
    ValueGen l(&left,  HintItem::no_hint(), this);
    ValueGen r(&right, HintItem::no_hint(), this);
    load_item(&left);
    load_item(&right);
    RInfo reg = rlock_result_with_hint(x, hint());
    item_free(&left);
    item_free(&right);
    emit()->arithmetic_op_fpu(x->op(), item2lir(_result), item2lir(&left), item2lir(&right), x->is_strictfp());
  }
  break;

  case Bytecodes::_frem: 
  case Bytecodes::_drem: {
    spill_caller_save(); // spill caller save registers before making C call

    Item left (x->x());
    Item right(x->y());

    ValueGen l(&left,  HintItem::no_hint(),  this);
    ValueGen r(&right, HintItem::no_hint(), this);
    
#ifdef _LP64
    // The 64 bit ABI passed arguments in Float/Double registers.
    // There is no need to move them to int registers.
    const RInfo left_reg    = (x->op() == Bytecodes::_frem) ? FrameMap::_F1_RInfo : FrameMap::_F0_double_RInfo;
    const RInfo right_reg   = (x->op() == Bytecodes::_frem) ? FrameMap::_F3_RInfo : FrameMap::_F2_double_RInfo;
    const RInfo result_reg  = (x->op() == Bytecodes::_frem) ? FrameMap::_F0_RInfo : FrameMap::_F0_double_RInfo;

    load_item_force(&left,  left_reg);
    load_item_force(&right, right_reg);
#else
    // There are no sparc machine instructions to move a value from a
    // floating point register to an int register or vice-versa.
    // At this point left & might may reside in fp registers. They have to 
    // be moved to the appropiate O registers for the C call.
    const RInfo left_reg    = (x->op() == Bytecodes::_frem) ? FrameMap::_O0_RInfo : FrameMap::_O0_O1_RInfo;
    const RInfo right_reg   = (x->op() == Bytecodes::_frem) ? FrameMap::_O1_RInfo : FrameMap::_O2_O3_RInfo;
    const RInfo result_reg  = (x->op() == Bytecodes::_frem) ? FrameMap::_F0_RInfo : FrameMap::_F0_double_RInfo;

    dont_load_item(&left);
    dont_load_item(&right);
    lock_spill_temp(NULL, left_reg);
    lock_spill_temp(NULL, right_reg);
    emit()->move(item2lir(&left),  LIR_OprFact::rinfo(left_reg));
    emit()->move(item2lir(&right), LIR_OprFact::rinfo(right_reg));
#endif

    item_free(&left);
    item_free(&right);

    { HideReg temp(this, FrameMap::callee_save_regs());
      // require the result to use the first float registers
      ra()->lock_register(x, result_reg);
      set_result(x, result_reg);

      //emit()->arithmetic_call_op_check(left_reg, right_reg, item2lir(_result));
      emit()->arithmetic_call_op(x->op(), temp.reg());
    }

#ifndef _LP64
    ra()->free_reg(left_reg);
    ra()->free_reg(right_reg);
#endif
  }
  break;

  default: ShouldNotReachHere();
  }
}


// for  _ladd, _lmul, _lsub, _ldiv, _lrem
void ValueGen::do_ArithmeticOp_Long(ArithmeticOp* x) {
  // don't create any items before possible spill_caller_save
  switch (x->op()) {
  case Bytecodes::_lrem: 
  case Bytecodes::_lmul: 
  case Bytecodes::_ldiv: {
    spill_caller_save();     // spill caller save registers before making C call

    Item left(x->x());
    Item right(x->y());
    ValueGen l(&left,  HintItem::no_hint(), this);
    ValueGen r(&right, HintItem::no_hint(), this);

    const RInfo left_reg   = FrameMap::_O2_O3_RInfo;
    const RInfo right_reg  = FrameMap::_O0_O1_RInfo;
    const RInfo result_reg = FrameMap::_O0_O1_RInfo;
    load_item_force(&left,  left_reg); 
    load_item_force(&right, right_reg); 
    RInfoCollection* oop_regs = ra()->oops_in_registers();
    item_free(&left);
    item_free(&right);

    if (x->op() == Bytecodes::_ldiv || x->op() == Bytecodes::_lrem) {
      CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope(), oop_regs);
      emit()->explicit_div_by_zero_check(item2lir(&right), info);
    }

    { HideReg temp(this, FrameMap::callee_save_regs());
      ra()->lock_register(x, result_reg);
      set_result(x,result_reg);

      emit()->arithmetic_call_op(x->op(), temp.reg());
    }

    break;
  }
  case Bytecodes::_ladd:
  case Bytecodes::_lsub: {
    Item left(x->x());
    Item right(x->y());
    ValueGen l(&left,  HintItem::no_hint(), this);
    ValueGen r(&right, HintItem::no_hint(), this);
    load_item(&left);
    load_item(&right);
    RInfo reg = rlock_result_with_hint(x, hint());
    item_free(&left);
    item_free(&right);

    emit()->arithmetic_op_long(x->op(), item2lir(_result), item2lir(&left), item2lir(&right));
    break;
  }
  default: ShouldNotReachHere();
  }
}


// Returns if item is an int constant that can be represented by a simm13
bool ValueGen::is_simm13(Item* item) {
  if (item->is_constant() && item->type()->as_IntConstant() != NULL) {
    return Assembler::is_simm13(item->get_jint_constant());
  } else {
    return false;
  }
}


// for: _iadd, _imul, _isub, _idiv, _irem
void ValueGen::do_ArithmeticOp_Int(ArithmeticOp* x) {
  bool is_div_rem = x->op() == Bytecodes::_idiv || x->op() == Bytecodes::_irem;
  Item left(x->x());
  Item right(x->y());
  // missing test if instr is commutative and if we should swap
  ValueGen l(&left, HintItem::no_hint(), this);
  ValueGen r(&right, HintItem::no_hint(), this);
  if (!is_simm13(&right)) {
    load_item(&right);
  } else {
    dont_load_item(&right);
  }
  assert(right.is_constant() || right.is_register(), "wrong state of right");
  load_item(&left);
  RInfoCollection* oop_regs = ra()->oops_in_registers();
  // Note: division may destroy result before it is used; must add checks
  bool use_hint = true;
  if (is_div_rem && hint()->has_hint()) {
    RInfo hint_reg = hint()->get_register();
    if ( left.get_register().is_same(hint_reg) 
        || (right.is_register() && right.get_register().is_same(hint_reg)) )
          use_hint = false; // result register will be destroyed while computing division
  }
  RInfo reg = rlock_result_with_hint(x, use_hint? hint() : HintItem::no_hint());
  if (is_div_rem) {
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->lock_stack(), x->exception_scope(), oop_regs);
    emit()->arithmetic_idiv(x->op(), item2lir(_result), item2lir(&left), item2lir(&right), FrameMap::_G1_RInfo, info);
  } else {
    emit()->arithmetic_op_int(x->op(), item2lir(_result), item2lir(&left), item2lir(&right), FrameMap::_G1_RInfo);
  }
  item_free(&left);
  item_free(&right);

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
  // Long shift destroys count register
  if (value.type()->is_long()) {
    count.set_destroys_register();
  }
  ValueGen v(&value, HintItem::no_hint(), this);
  ValueGen c(&count, HintItem::no_hint(), this);
  load_item(&value);
  // the old backend doesn't support this
  if (count.is_constant() && count.type()->as_IntConstant() != NULL && value.type()->is_int()) {
    jint c = count.get_jint_constant() & 0x1f;
    assert(c >= 0 && c < 32, "should be small");
    dont_load_item(&count);
  } else {
    load_item(&count);
  }
  RInfo reg = rlock_result_with_hint(x, hint());
  item_free(&count);
  item_free(&value);
  emit()->shift_op(x->op(), reg, item2lir(&value), item2lir(&count), norinfo);
}


// _iand, _land, _ior, _lor, _ixor, _lxor
void ValueGen::do_LogicOp(LogicOp* x) {
  Item left(x->x());
  Item right(x->y());
  // missing test if instr is commutative and if we should swap
  ValueGen l(&left, HintItem::no_hint(), this);
  ValueGen r(&right, HintItem::no_hint(), this);
  load_item(&left);
  if (is_simm13(&right)) {
    // type is int and constant is small
    dont_load_item(&right);
  } else {
    load_item(&right);
  }
  RInfo reg = rlock_result_with_hint(x, hint());
  item_free(&left);
  item_free(&right);
  emit()->logic_op(x->op(), reg, item2lir(&left), item2lir(&right));
}



// _lcmp, _fcmpl, _fcmpg, _dcmpl, _dcmpg
void ValueGen::do_CompareOp(CompareOp* x) {
  Item left(x->x());
  Item right(x->y());
  ValueGen l(&left,  HintItem::no_hint(), this);
  ValueGen r(&right, HintItem::no_hint(), this);
  load_item(&left);
  load_item(&right);
  RInfo reg = rlock_result_with_hint(x, hint());
  item_free(&left);
  item_free(&right);
  emit()->compare_op(x->op(), reg, item2lir(&left), item2lir(&right));
}


// Code for  :  x->x() {x->cond()} x->y() ? x->tval() : x->fval()
void ValueGen::do_IfOp(IfOp* x) {
#ifdef ASSERT
  {
    ValueTag xtag = x->x()->type()->tag();
    ValueTag ttag = x->tval()->type()->tag();
    assert(xtag == intTag || xtag == objectTag, "cannot handle others");
    assert(ttag == addressTag || ttag == intTag || ttag == objectTag || ttag == longTag, "cannot handle others");
    assert(ttag == x->fval()->type()->tag(), "cannot handle others");
  }
#endif

  Item left(x->x());
  Item right(x->y());

  ValueGen l(&left,  HintItem::no_hint(), this);
  ValueGen r(&right, HintItem::no_hint(), this);

  load_item(&left);
  load_item(&right);
  item_free(&left);
  item_free(&right);

  emit()->ifop_phase1(x->cond(), item2lir(&left), item2lir(&right));

  Item t_val(x->tval());
  Item f_val(x->fval());

  // use hint() if we make sure that input is local or constant
  HintItem tval_hint(*HintItem::no_hint());
  if (x->tval()->as_LoadLocal() != NULL || x->tval()->as_Constant() != NULL) {
    tval_hint.set_from_item(hint());
  }

  HintItem fval_hint(*HintItem::no_hint());
  if (x->fval()->as_LoadLocal() != NULL || x->fval()->as_Constant() != NULL) {
    fval_hint.set_from_item(hint());
  }

  ValueGen tv(&t_val, &tval_hint, this);
  ValueGen fv(&f_val, &fval_hint, this);

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
      HideReg thread(this, FrameMap::callee_save_regs());
      spill_values_on_stack(x->state());
      set_with_result_register(x);
      emit()->lir()->call_runtime_leaf(CAST_FROM_FN_PTR(address, os::javaTimeMillis), thread.reg(), 0, 0);
      break;
    }
    case methodOopDesc::_nanoTime: {
      assert(x->number_of_arguments() == 0, "wrong type");
      HideReg thread(this, FrameMap::callee_save_regs());
      spill_values_on_stack(x->state());
      set_with_result_register(x);
      emit()->lir()->call_runtime_leaf(CAST_FROM_FN_PTR(address, os::javaTimeNanos), thread.reg(), 0, 0);
      break;
    }

    case methodOopDesc::_dsqrt: {
      assert(x->number_of_arguments() == 1, "wrong type");
      Item value(x->argument_at(0));
      ValueGen v(&value, HintItem::no_hint(), this);
      load_item(&value);
      item_free(&value);
      RInfo result_reg = rlock_result_with_hint(x, hint());
      emit()->math_intrinsic(x->id(), result_reg, item2lir(&value));
      break;
    }  
    case methodOopDesc::_dsin: // fall through
    case methodOopDesc::_dcos: {
      assert(x->number_of_arguments() == 1, "wrong type");
      // Note: spill caller save before setting the item
      spill_caller_save();  // spill caller save registers before making a C call

      Item value(x->argument_at(0));
      value.set_destroys_register();

      ValueGen v(&value, HintItem::no_hint(), this);
    
#ifdef _LP64
      // 64 bit Sparc ABI passes first double in D0
      RInfo value_reg  = FrameMap::_F0_double_RInfo;
      RInfo result_reg = FrameMap::_F0_double_RInfo;
      load_item_force(&value,  value_reg);
#else

      // There are no sparc machine instructions to move a value from a
      // floating point register to an int register or vice-versa.
      // At this point value may reside in an fp register. It has to 
      // be moved to the appropiate O registers for the C call.
      RInfo value_reg  = FrameMap::_O0_O1_RInfo;
      RInfo result_reg = FrameMap::_F0_double_RInfo;

      dont_load_item(&value);
      lock_spill_temp(NULL, value_reg);
      emit()->move(item2lir(&value), LIR_OprFact::rinfo(value_reg));
#endif

      item_free(&value);

      // require the result to use the first float registers
      ra()->lock_register(x, result_reg);
      set_result(x, result_reg);
      {
        HideReg thread(this, FrameMap::callee_save_regs());
        emit()->math_intrinsic(x->id(), result_reg, item2lir(&value), thread.reg());
      }
#ifndef _LP64
      ra()->free_reg(value_reg);
#endif
      break;
    }
    case methodOopDesc::_arraycopy:
      { assert(x->number_of_arguments() == 5, "wrong type");
        // Note: spill caller save before setting the item
        spill_caller_save();  // spill caller save registers before making a C call
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
        // load all values in callee_save_registers, as this makes the 
        // parameter passing to the fast case simpler
        load_item_with_reg_mask(&src,     FrameMap::callee_save_regs());
        load_item_with_reg_mask(&src_pos, FrameMap::callee_save_regs()); 
        load_item_with_reg_mask(&dst,     FrameMap::callee_save_regs());
        load_item_with_reg_mask(&dst_pos, FrameMap::callee_save_regs());
        load_item_with_reg_mask(&length,  FrameMap::callee_save_regs());

        int flags;
        ciArrayKlass* expected_type;
        arraycopy_helper(x, &flags, &expected_type);

        {
          HideReg thread(this, FrameMap::callee_save_regs());
          // we may want to have stack (deoptimization?)
          CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(),
                                                x->state(), x->exception_scope(), ra()->oops_in_registers());
          __ arraycopy(item2lir(&src), item2lir(&src_pos), item2lir(&dst), item2lir(&dst_pos),
                       item2lir(&length), LIR_OprFact::rinfo(thread.reg(), T_ADDRESS),
                       expected_type, flags, info);
        }
        // makes sure that the thread register is not one of the parameter register,
        // as it gets destroyed
        item_free(&src);
        item_free(&src_pos);
        item_free(&dst);
        item_free(&dst_pos);
        item_free(&length);
        set_no_result(x);
      }
      break;
      
    case methodOopDesc::_getClass:       do_getClass(x);                break;
    case methodOopDesc::_currentThread:  do_currentThread(x);           break;

    // java.nio.Buffer.checkIndex
    case methodOopDesc::_checkIndex:     do_NIOCheckIndex(x);           break;

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

      ValueGen obj_v(&obj, HintItem::no_hint(), this);  
      ValueGen cmp_value_v(&cmp_value, HintItem::no_hint(), this);
      ValueGen new_value_v(&new_value, HintItem::no_hint(), this);

      load_item(&obj);
      load_item(&cmp_value);
      load_item(&new_value);

      item_free(&obj);
      item_free(&cmp_value);
      item_free(&new_value);

      // generate compare-and-swap and produce zero condition if swap occurs
      int value_offset = sun_misc_AtomicLongCSImpl::value_offset();
      LIR_Opr addr = LIR_OprFact::rinfo(FrameMap::_O7_RInfo);
      emit()->lir()->add(item2lir(&obj), LIR_OprFact::intConst(value_offset), addr);
      LIR_Opr t1 = LIR_OprFact::rinfo(FrameMap::_G1_RInfo);  // temp for 64-bit value
      LIR_Opr t2 = LIR_OprFact::rinfo(FrameMap::_G3_RInfo);  // temp for 64-bit value
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

    default:
      Unimplemented();
      break;
  }
}


// _i2l, _i2f, _i2d, _l2i, _l2f, _l2d, _f2i, _f2l, _f2d, _d2i, _d2l, _d2f
// _i2b, _i2c, _i2s
void ValueGen::do_Convert(Convert* x) {
  // don't create any items before possible spill_caller_save
  switch (x->op()) {
    case Bytecodes::_f2l: 
    case Bytecodes::_d2l: 
    case Bytecodes::_d2i: { 
      spill_caller_save();  // spill caller save registers before making a C call

      Item value(x->value());
      ValueGen v(&value, HintItem::no_hint(), this);
    
#ifdef _LP64
      // Make sure value is in correct floating point register for call
      RInfo value_reg  = (value.type()->tag() == floatTag ? FrameMap::_F1_RInfo : 
                              FrameMap::_F0_double_RInfo);
      load_item_force(&value,  value_reg);
      item_free(&value);
#else
      // There are no sparc machine instructions to move a value from a
      // floating point register to an int register or vice-versa.
      // At this point value may reside in an fp register. It has to 
      // be moved to the appropiate O registers for the C call.
      RInfo value_reg  = (value.type()->tag() == floatTag ? FrameMap::_O0_RInfo : 
                        FrameMap::_O0_O1_RInfo);
      dont_load_item(&value);

      lock_spill_temp(NULL, value_reg);
      emit()->move(item2lir(&value), LIR_OprFact::rinfo(value_reg));
      item_free(&value);
      ra()->free_reg(value_reg);
#endif
      RInfo result_reg = (x->op() == Bytecodes::_d2i ? FrameMap::_O0_RInfo : FrameMap::_O0_O1_RInfo);
      ra()->lock_register(x, result_reg);
      set_result(x, result_reg);
      {
        HideReg thread(this, FrameMap::callee_save_regs());
        emit()->call_convert_op(x->op(), thread.reg());
      }
      break;
    }

    case Bytecodes::_l2f: 
    case Bytecodes::_l2d: {
      spill_caller_save();  // spill caller save registers before making C call

      Item value(x->value());
      HintItem value_hint(x->value()->type(), FrameMap::_O0_O1_RInfo);
      ValueGen v(&value, &value_hint, this);

      // force long value into o0/o1 for the C call
      load_item_force(&value, FrameMap::_O0_O1_RInfo);
      item_free(&value);

      { // thread register must be callee saved
        HideReg thread(this, FrameMap::callee_save_regs());

        // require the result to use the first float registers
        RInfo result_reg;
        switch (x->type()->tag()) {
          case floatTag:   result_reg = FrameMap::_F0_RInfo;        break;
          case doubleTag:  result_reg = FrameMap::_F0_double_RInfo; break;
          default: ShouldNotReachHere();
        }

        ra()->lock_register(x, result_reg);
        set_result(x, result_reg);
        emit()->call_convert_op(x->op(), thread.reg());
      }
    }
    break;

    case Bytecodes::_i2f:
    case Bytecodes::_i2d: {
      Item value(x->value());
      ValueGen v(&value, HintItem::no_hint(), this);

      dont_load_item(&value);
      RInfo reg = rlock_result_with_hint(x, hint());
      if (x->op() == Bytecodes::_i2d) {
        // To convert an int to double, we need to load the 32-bit int
        // from memory into a single precision floating point register
        // (even numbered). Then the sparc fitod instruction takes care
        // of the conversion. This is a bit ugly, but is the best way to
        // get the int value in a single precision floating point register
        RInfo float_reg = as_RInfo(reg.as_double_reg(), false);
        emit()->move(item2lir(&value), LIR_OprFact::rinfo(float_reg));
        emit()->convert_op(x->op(), LIR_OprFact::rinfo(float_reg), reg);
      } else {
        emit()->move(item2lir(&value), LIR_OprFact::rinfo(reg));
        emit()->convert_op(x->op(), LIR_OprFact::rinfo(reg), reg);
      }
      item_free(&value);
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
      Item value(x->value());
      ValueGen v(&value,  HintItem::no_hint(), this);

      load_item(&value);    
      RInfo reg = rlock_result_with_hint(x, hint());
      item_free(&value);
      emit()->convert_op(x->op(), item2lir(&value), reg);
    }
    break;
    
    case Bytecodes::_f2i: {
      Item value (x->value()); value.set_destroys_register();
      ValueGen v(&value, HintItem::no_hint(), this);
      load_item(&value);
      RInfo reg = rlock_result_with_hint(x, hint());
      item_free(&value);

      emit()->convert_op(x->op(), item2lir(&value), reg);
    }
    break;
    
    default: ShouldNotReachHere();
  }
}


void ValueGen::do_CachingChange(CachingChange* x) {

}

// Visits all arguments, returns appropriate items without loading them
ItemArray* ValueGen::invoke_visit_arguments(Invoke* x, CallingConvention* arg_loc) {
  ItemList* argument_items = new ItemList();
  if (x->has_receiver()) {
    Item* receiver = new Item(x->receiver());
    argument_items->append(receiver);
    ArgumentLocation loc = arg_loc->arg_at(0);
    assert(loc.is_register_arg() && loc.outgoing_reg_location().is_same(FrameMap::_O0_RInfo), "receiver must be in O0");
    HintItem hint(objectType, FrameMap::_O0_RInfo);
    ValueGen p(receiver, &hint, this);
  }
  int idx = x->has_receiver() ? 1 : 0;
  for (int i = 0; i < x->number_of_arguments(); i++) {
    Item* param = new Item(x->argument_at(i));
    argument_items->append(param);
    ArgumentLocation loc = arg_loc->arg_at(idx);
    idx += (param->type()->is_double_word() ? 2 : 1);
    HintItem hint(x->argument_at(i)->type());
    if (loc.is_register_arg()) {
      hint.set_register(loc.outgoing_reg_location());
    } else {
      hint.set_from_item(HintItem::no_hint());
    }
    ValueGen p(param, &hint, this);
  }
  return argument_items;
}


void ValueGen::invoke_load_arguments(Invoke* x, ItemArray* args, CallingConvention* arg_loc) {
  ItemList register_args(FrameMap::nof_reg_args);
  int idx = x->has_receiver() ? 1 : 0;
  for (int i = 0; i < x->number_of_arguments(); i++) {
    Item* param = args->at(x->has_receiver() ? i+1 : i);
    ArgumentLocation loc = arg_loc->arg_at(idx);
    if (loc.is_register_arg()) {
      load_item_force(param, loc.outgoing_reg_location());
      // don't free items for register arguments until all other arguments are loaded;
      // otherwise an argument register could be overwritten while initializing a stack argument
      register_args.append(param);
    } else {
      assert(loc.is_stack_arg(), "just checking");
      load_item(param);
      emit()->store_stack_parameter (item2lir(param), loc.stack_offset_in_words());
      item_free(param);
    }
    idx += (param->type()->is_double_word() ? 2 : 1);
  }
  if (x->has_receiver()) {
    Item* receiver = args->at(0);
    ArgumentLocation loc = arg_loc->arg_at(0);
    assert(loc.is_register_arg(), "the receiver must be in O0");
    load_item_force(receiver, loc.outgoing_reg_location());
    item_free(receiver);
  }
  for (int i = 0; i < register_args.length(); i++) {
    item_free(register_args[i]);
  }
}


// handle arguments, without receiver
void ValueGen::invoke_do_arguments(Invoke* x) {
  ShouldNotReachHere();
}



void ValueGen::invoke_do_spill(Invoke* x, RInfo receiver_reg) {
  // move caller save registers to callee save registers (saves the spill)
  // make sure that caller save registers are spilled
  spill_values_on_stack(x->state(), receiver_reg, true);
}


void ValueGen::invoke_do_result(Invoke* x, bool needs_null_check, Item* receiver) {
  // assert(ra()->are_all_registers_free(), "all registers must be free across calls");
  // setup result register 
  RInfoCollection* oop_regs = ra()->oops_in_registers();
  if (x->type()->is_void()) {
    set_no_result(x);
  } else {
    RInfo reg = result_register_for(x->type());
    lock_spill_rinfo(x, reg);
    set_result(x, reg);
  }
  // emit invoke code
  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope(), oop_regs);
  bool optimized = x->target_is_loaded() && x->target_is_final();
  emit()->call_op(x->code(), x->signature(), info, x->vtable_index(), optimized, needs_null_check, receiver == NULL ? norinfo : receiver->get_register(), item2lir(_result)); // does add_safepoint
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
void ValueGen::do_Invoke(Invoke* x) {
  CallingConvention* arg_loc = FrameMap::calling_convention(!x->has_receiver(), *x->signature());
  ItemArray* args = invoke_visit_arguments(x, arg_loc);
  if (x->has_receiver()) {
    Item* receiver = args->at(0);
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
    invoke_do_result(x, needs_null_check, receiver);
  } else {
    invoke_load_arguments(x, args, arg_loc);
    invoke_do_spill(x);
    invoke_do_result(x, false);
  }
}


void ValueGen::do_NewInstance(NewInstance* x) {
  NEEDS_CLEANUP;
  // Note: change this to require any register as result of a new,
  //       instead of specifically %o0; do not spill all values.

  // make sure registers are spilled
  spill_values_on_stack(x->state());
  RInfoCollection* oop_regs = ra()->oops_in_registers();

  // This instruction can be deoptimized in the slow path : use
  // O0 as result register.
  const RInfo reg = set_with_result_register(x);
    
  if (PrintNotLoaded && !x->klass()->is_loaded()) {
    tty->print_cr("   ###class not loaded at new bci %d", x->bci());
  }
  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope(), oop_regs);
  emit()->new_instance(reg, x->klass(), FrameMap::_G1_RInfo, FrameMap::_G3_RInfo, FrameMap::_G4_RInfo, FrameMap::_G5_RInfo, info);
}


void ValueGen::do_NewTypeArray(NewTypeArray* x) {
  // make sure registers are spilled
  spill_values_on_stack(x->state());

  Item length(x->length());
  ValueGen l(&length, HintItem::no_hint(), this);
  load_item(&length);
  RInfoCollection* oop_regs = ra()->oops_in_registers();

  HintItem result_hint(x->type(), result_register_for(x->type()));
  RInfo reg = rlock(x, &result_hint);
  set_result(x, reg);

  SpillLockReg sl(this, reg);
  {
    RInfo t1 = FrameMap::_G1_RInfo;
    RInfo t2 = FrameMap::_G3_RInfo;
    RInfo t3 = FrameMap::_G4_RInfo;
    RInfo klass_reg = FrameMap::_G5_RInfo;
    item_free(&length);
    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope(), oop_regs);
    emit()->new_type_array(reg, x->elt_type(), item2lir(&length), t1, t2, t3, norinfo, klass_reg, info);
  }
}


void ValueGen::do_NewObjectArray(NewObjectArray* x) {
  // make sure registers are spilled
  NEEDS_CLEANUP // factor out, very similar
  spill_values_on_stack(x->state());

  Item length(x->length());
  ValueGen l(&length, HintItem::no_hint(), this);
  // in case of patching (i.e., object class is not yet loaded), we need to reexecute the instruction
  // and therefore provide the state before the parameters have been consumed
  RInfoCollection* oop_regs = ra()->oops_in_registers();
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), oop_regs);
  }

  load_item(&length);

  HintItem result_hint(x->type(), result_register_for(x->type()));
  RInfo reg = rlock(x, &result_hint);
  set_result(x, reg);

  SpillLockReg sl(this, reg);
  {
    RInfo t1 = FrameMap::_G1_RInfo;
    RInfo t2 = FrameMap::_G3_RInfo;
    RInfo t3 = FrameMap::_G4_RInfo;
    RInfo klass_reg = FrameMap::_G5_RInfo;
    item_free(&length);

    CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope(), oop_regs);
    emit()->new_object_array(reg, x->klass(), item2lir(&length), t1, t2, t3, norinfo,
                             klass_reg, info, patching_info);
  }
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
  }

  // need to get the info before, as the items may become invalid through item_free
  RInfoCollection* oop_regs = ra()->oops_in_registers();
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), oop_regs);
  }

  intStack* free_after = new intStack();
  i = dims->length();
  while (i-- > 0) {
    Item* size = items->at(i);
    // if a patching_info was generated above then debug information for the state before
    // the call is going to be emitted.  The ValueGen calls above may have left some values
    // in registers and that's been recorded in the CodeEmitInfo.  In that case the items
    // for those values can't simply be freed if they are registers because the values
    // might be destroyed by store_stack_parameter.  So in the case of patching, delay the
    // freeing of the items that already were in registers
    bool was_register = size->is_register();
    load_item(size);
    emit()->store_stack_parameter (item2lir(size), i + frame::memory_parameter_word_sp_offset);

    if (was_register && patching_info != NULL) {
      free_after->append(i);
    } else {
      item_free(size);
    }
  }

  // This instruction can be deoptimized in the slow path : use
  // O0 as result register.
  const RInfo reg = set_with_result_register(x);
  SpillLockReg sl(this, reg);
  CodeEmitInfo* info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state(), x->exception_scope(), oop_regs);
  emit()->new_multi_array(reg, x->klass(), x->rank(), norinfo, info, patching_info);

  // free any items which were referenced from the x->state_before() debug information
  while (free_after->length() > 0) {
    int index = free_after->pop();
    Item* size = items->at(index);
    item_free(size);
  }
}


void ValueGen::do_BlockBegin(BlockBegin* x) {
}


void ValueGen::do_CheckCast(CheckCast* x) {
  spill_values_on_stack(x->state());
  Item obj(x->obj());
  ValueGen o(&obj, HintItem::no_hint(), this);
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || (PatchALot && !x->is_incompatible_class_change_check())) {
    // must do this before locking the destination register as an oop register,
    // and before the obj is loaded (so x->obj()->item() is valid for creating a debug info location)
    patching_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
  }
  load_item(&obj);
  const RInfo in_reg = obj.get_register();
  const RInfo out_reg = rlock_result_with_hint(x, hint());
  item_free(&obj);
      
  if (patching_info != NULL) {
    patching_info->add_register_oop(in_reg);
  }
  CodeEmitInfo* info_for_exception = new CodeEmitInfo(emit(), x->bci(), NULL,
                                                      x->state()->copy_locks(), x->exception_scope(), NULL);
  CodeStub* stub;
  LIR_Opr obj_opr = item2lir(&obj);
  if (x->is_incompatible_class_change_check()) {
    assert(patching_info == NULL, "can't patch this");
    stub = new SimpleExceptionStub(Runtime1::throw_incompatible_class_change_error_id, norinfo, info_for_exception);
  } else {
    stub = new SimpleExceptionStub(Runtime1::throw_class_cast_exception_id, obj_opr->rinfo(), info_for_exception);
  }
  __ checkcast(LIR_OprFact::rinfo(out_reg, T_OBJECT), obj_opr, x->klass(),
               LIR_OprFact::rinfo(FrameMap::_G1_RInfo), LIR_OprFact::rinfo(FrameMap::_G3_RInfo),
               x->direct_compare(), info_for_exception, patching_info, stub);
}


void ValueGen::do_InstanceOf(InstanceOf* x) {
  spill_values_on_stack(x->state());
  Item obj(x->obj());
  ValueGen o(&obj, HintItem::no_hint(), this);
  CodeEmitInfo* patching_info = NULL;
  if (!x->klass()->is_loaded() || PatchALot) {
    patching_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
  }
  // ensure the result register is not the input register because the result is initialized before the patching safepoint
  load_item(&obj);
  const RInfo in_reg = obj.get_register();
  const RInfo out_reg = rlock_result_with_hint(x, hint()); 
  item_free(&obj);    
  if (patching_info != NULL) {
    // object must be part of the oop map
    patching_info->add_register_oop(in_reg);
  }
  emit()->instanceof_op(LIR_OprFact::rinfo(out_reg, T_OBJECT), item2lir(&obj), x->klass(), FrameMap::_G1_RInfo, FrameMap::_G3_RInfo,  x->direct_compare(), patching_info);
}


void ValueGen::do_If(If* x) {
  assert(x->number_of_sux() == 2, "inconsistency");
  ValueTag tag = x->x()->type()->tag();
  Item xitem(x->x());
  Item yitem(x->y());
  Item* xin = &xitem;
  Item* yin = &yitem;
  If::Condition cond = x->cond();
  {  // call destructors before doing move to phi
    ValueGen xg(xin, HintItem::no_hint(), this);
    ValueGen yg(yin, HintItem::no_hint(), this);
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
    load_item(xin);
    if (is_simm13(yin)) {
      // inline int constants which are small enough to be immediate operands
      dont_load_item(yin);
    } else if (tag == longTag && yin->is_constant() && yin->get_jlong_constant() == 0 && (cond == If::eql || cond == If::neq)) {
      // inline long zero
      dont_load_item(yin);
    } else if (tag == objectTag && yin->is_constant() && (yin->get_jobject_constant()->is_null_object())) {
      dont_load_item(yin);
    } else {
      load_item(yin);
    }
  }  

  // note that the condition test must happen before the
  // moves into Phi area happen, and that the control flow
  // jump must happen after the moves into the phi area

  // add safepoint before generating condition code so it can be recomputed
  CodeEmitInfo* safepoint_info = NULL;
  if (x->is_safepoint()) {
    safepoint_info = new CodeEmitInfo(emit(), x->bci(), ra()->oops_in_spill(), x->state_before(), x->exception_scope(), ra()->oops_in_registers());
    if (SafepointPolling) {
      // Get a temporary register for the poll check
      RInfo tmp = lock_free_rinfo(NULL, objectType);
      ra()->free_reg(tmp);
      __ safepoint(tmp, safepoint_info);
      safepoint_info = NULL;
    } else {
      safepoint_info->set_is_compiled_safepoint();
    }
  }
  item_free(yin);
  item_free(xin);
  set_no_result(x);

  // Note that in phase 2, the registers holding the testing values may have been destroyed
  emit()->if_op(1, cond, item2lir(xin), item2lir(yin), x->tsux(), x->fsux(), x->usux(), safepoint_info);
  move_to_phi(x->state());
  emit()->if_op(2, cond, item2lir(xin), item2lir(yin), x->tsux(), x->fsux(), x->usux(), NULL);

  goto_default_successor(x, NULL);
}


void ValueGen::do_IfInstanceOf(IfInstanceOf* x) {
  Unimplemented();
}


// May change the content of tag
void ValueGen::setup_phis_for_switch(Item* tag, ValueStack* stack) {
  // if stack() exists, then move_to_phi may destroy the tag register as
  // the tag register is not locked anymore.
  if (stack != NULL && stack->stack_size() > 0 ) {
    // stack exists, move values into phi locations
    // preserve the tag

    // save in G1
    emit()->move(item2lir(tag), FrameMap::_G1_RInfo);
    move_to_phi(stack);
    { HideReg hr(this, nonReturnRInfo()); // use one register that is not a return register
      tag->set_register(hr.reg());
      // restore from G1(do not use return registers)
      emit()->move(LIR_OprFact::rinfo(FrameMap::_G1_RInfo), item2lir(tag));
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
      // hide the tag register, then get a temporary register for the poll check
      HideReg tag_reg(this, tag.get_register());
      RInfo tmp = lock_free_rinfo(NULL, objectType);
      ra()->free_reg(tmp);
      __ safepoint(tmp, info_before);
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
      // hide the tag register, then get a temporary register for the poll check
      HideReg tag_reg(this, tag.get_register());
      RInfo tmp = lock_free_rinfo(NULL, objectType);
      ra()->free_reg(tmp);
      __ safepoint(tmp, info_before);
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
    RInfo reg = result_register_for(x->type(), /*callee=*/true);
    Item result(x->result());
    HintItem result_hint(x->type());

    // for synchronized methods we don't want to force the result into
    // the return register yet since we are probably caching the receiver
    // there.
    if (x->is_synchronized()) {
      result_hint.set_from_item(HintItem::no_hint());
    } else {
      result_hint.set_register(reg);
    }
    ValueGen r(&result, &result_hint, this);
    if (x->is_synchronized()) {
      // we do not need this, as the ??
      if (result.is_register() && is_caller_save_register(result.get_register())) {
        spill_value(x->result());
        result.update();
      }
      emit()->return_op_prolog(x->monitor_no());
    }
    load_item_force(&result, reg);
    item_free(&result);
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
  assert(G2_thread == FrameMap::_G2_RInfo.as_register(), "update this");
  emit()->lir()->load_mem_reg(FrameMap::_G2_RInfo, in_bytes(JavaThread::threadObj_offset()), result, T_OBJECT, NULL);
}


void ValueGen::do_CompareAndSwap(Intrinsic* x, ValueType* type) {
  assert(x->number_of_arguments() == 4, "wrong type");
  Item obj   (x->argument_at(0));  // object
  Item offset(x->argument_at(1));  // offset of field
  Item cmp   (x->argument_at(2));  // value to compare with field
  Item val   (x->argument_at(3));  // replace field with val if matches cmp

  // Use temps to avoid kills
  LIR_Opr t1 = LIR_OprFact::rinfo(FrameMap::_G1_RInfo); 
  LIR_Opr t2 = LIR_OprFact::rinfo(FrameMap::_G3_RInfo); 
  LIR_Opr addr = LIR_OprFact::rinfo(FrameMap::_O7_RInfo);

  // get address of field
  ValueGen obj_v(&obj, HintItem::no_hint(), this);  
  ValueGen offset_v(&offset, HintItem::no_hint(), this);  
  load_item(&obj);
  load_item(&offset);
 
  ValueGen cmp_v(&cmp, HintItem::no_hint(), this);
  ValueGen val_v(&val, HintItem::no_hint(), this);
  load_item(&cmp);
  load_item(&val);

  item_free(&offset);
  item_free(&cmp);
  item_free(&val);
  // don't free obj yet, to make sure we have reg for write barrier

  emit()->lir()->add(item2lir(&obj), item2lir(&offset), addr);

  if (type == objectType) 
    emit()->lir()->cas_obj(addr, item2lir(&cmp), item2lir(&val), t1, t2);
  else if (type == intType)
    emit()->lir()->cas_int(addr, item2lir(&cmp), item2lir(&val), t1, t2);
  else if (type == longType)
    emit()->lir()->cas_long(addr, item2lir(&cmp), item2lir(&val), t1, t2);
  else {
    ShouldNotReachHere();
  }
  
  // generate conditional move of boolean result
  RInfo result = rlock_result_with_hint(x, hint());
  LabelObj* L = new LabelObj();
  emit()->move(LIR_OprFact::intConst(1), result);
  emit()->lir()->branch(LIR_OpBranch::equal, L->label());
  emit()->move(LIR_OprFact::intConst(0), result);
  emit()->lir()->branch_destination(L->label());

  item_free(&obj);
  if (type == objectType)   // Write-barrier needed for Object fields.
    emit()->write_barrier(addr, t2);
}

