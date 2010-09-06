#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIR.cpp	1.103 04/04/20 15:56:15 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_LIR.cpp.incl"


LIR_Opr LIR_OprFact::illegalOpr = LIR_OprFact::illegal();

LIR_Opr LIR_OprFact::value_type(ValueType* type) {
  ValueTag tag = type->tag();
  switch (tag) {
  case objectTag : {
    ClassConstant* c = type->as_ClassConstant();
    if (c != NULL && !c->value()->is_loaded()) {
      return LIR_OprFact::oopConst(NULL);
    } else {
      return LIR_OprFact::oopConst(type->as_ObjectType()->encoding());
    }
  }
  case addressTag: return LIR_OprFact::intConst(type->as_AddressConstant()->value());
  case intTag    : return LIR_OprFact::intConst(type->as_IntConstant()->value());
  case floatTag  : return LIR_OprFact::floatConst(type->as_FloatConstant()->value());
  case longTag   : return LIR_OprFact::longConst(type->as_LongConstant()->value());
  case doubleTag : return LIR_OprFact::doubleConst(type->as_DoubleConstant()->value());
  default: ShouldNotReachHere();
  }
}


LIR_Opr LIR_OprFact::dummy_value_type(ValueType* type) {
  switch (type->tag()) {
    case objectTag: return LIR_OprFact::oopConst(NULL);
    case addressTag:
    case intTag:    return LIR_OprFact::intConst(0);
    case floatTag:  return LIR_OprFact::floatConst(0.0);
    case longTag:   return LIR_OprFact::longConst(0);
    case doubleTag: return LIR_OprFact::doubleConst(0.0);
    default:        ShouldNotReachHere();
  }
  return illegalOpr;
}



//---------------------------------------------------


LIR_OprRefCount::LIR_OprRefCount(LIR_OprRefCountType type) {
  _type = type;
  int size = 0;
  switch (type) {
    case cpu_reg_type: size = FrameMap::nof_cpu_regs; break;
    case fpu_reg_type: size = FrameMap::nof_fpu_regs; break;
    case stack_type:   size = 6; break;
    default: ShouldNotReachHere();
  }
  _ref_count_data = new intStack(size);
  _oprs = new LIR_OprList(size);
}


void LIR_OprRefCount::increment(int index, LIR_Opr opr, int count) {
  int old = _ref_count_data->at_grow(index, 0);
  _ref_count_data->at_put(index, old + count);
  if (old == 0) {
    // if this is the first time we've seen this opr, then record
    // in it our list for later.
    _oprs->at_put_grow(index, opr, LIR_OprFact::illegalOpr);
  }
}


bool LIR_OprRefCount::is_correct_type(LIR_Opr opr) const {
  switch (type()) {
  case cpu_reg_type: return opr->is_register() && (opr->is_single_cpu() || opr->is_double_cpu());
  case fpu_reg_type: return opr->is_float_kind();
  case stack_type:   return opr->is_stack();
  default: ShouldNotReachHere();
  }
  return false;
}


void LIR_OprRefCount::incr_ref(LIR_Opr opr, int count) {
  assert(is_correct_type(opr), "type check failed");
  if (opr->is_double_word()) {
    switch (type()) {
    case cpu_reg_type: increment(opr->cpu_regnrLo(), opr, count);increment(opr->cpu_regnrHi(), opr, count); break;
    case fpu_reg_type: increment(opr->fpu_regnrLo(), opr, count);increment(opr->fpu_regnrHi(), opr, count); break;
    case stack_type:   increment(opr->double_stack_ix(), opr, count);increment(opr->double_stack_ix() + 1, opr, count); break;
    default: ShouldNotReachHere();
    }
  } else {
    switch (type()) {
    case cpu_reg_type: increment(opr->cpu_regnr(), opr, count); break;
    case fpu_reg_type: increment(opr->fpu_regnr(), opr, count); break;
    case stack_type:   increment(opr->single_stack_ix(), opr, count); break;
    default: ShouldNotReachHere();
    }
  }
}


int LIR_OprRefCount::ref_count(LIR_Opr opr) const {
  assert(is_correct_type(opr), "type check failed");
  assert(opr->is_single_word(), "should only check ref count for single word");
  int index = 0;
  switch (type()) {
  case cpu_reg_type: index = opr->cpu_regnr(); break;
  case fpu_reg_type: index = opr->fpu_regnr(); break;
  case stack_type:   index = opr->single_stack_ix(); break;
  default: ShouldNotReachHere();
  }

  if (index < data()->length()) {
    return data()->at(index);
  } else {
    return 0;
  }
}


void LIR_OprRefCount::merge(LIR_OprRefCount* other) {
  intStack* other_data = other->data();
  LIR_OprList* other_oprs = other->_oprs;
  for (int i = 0; i < other_data->length(); i++) {
    int count = other_data->at(i);
    if (count) {
      incr_ref(other_oprs->at(i), count);
    }
  }
}


#ifndef PRODUCT
void LIR_OprRefCount::print() {
  tty->print("[ ");
  for (int i = 0; i < data()->length(); i ++) {
    int x = data()->at(i);
    if (x > 0) {
      LIR_Opr opr = _oprs->at(i);
      opr->print();
      tty->print("%d ", x);
    }
  }
  tty->print_cr("]");
}
#endif



LIR_Address::Scale LIR_Address::scale(BasicType type) {
  int elem_size = type2aelembytes[type];
  switch (elem_size) {
  case 1: return LIR_Address::times_1;
  case 2: return LIR_Address::times_2;
  case 4: return LIR_Address::times_4;
  case 8: return LIR_Address::times_8;
  }
  ShouldNotReachHere();
  return LIR_Address::times_1;
}


//---------------------------------------------------

char LIR_OprDesc::type_char(BasicType t) {
  switch (t) {
    case T_ARRAY:
      t = T_OBJECT;
    case T_BOOLEAN:
    case T_CHAR:
    case T_FLOAT:
    case T_DOUBLE:
    case T_BYTE:
    case T_SHORT:
    case T_INT:
    case T_LONG:
    case T_OBJECT:
    case T_ADDRESS:
      return ::type2char(t);

    case T_ILLEGAL:
      return '?';

    default:
      ShouldNotReachHere();
  }
}


bool LIR_OprDesc::is_oop() const {
  if (is_pointer()) {
    return pointer()->is_oop_pointer();
  } else {
    OprType t= type_field();
    assert(t != unknown_type, "not set");
    return t == object_type || t == array_type; 
  }
}


LIR_OpBranch::LIR_OpBranch(LIR_Condition cond, BlockBegin* block, CodeEmitInfo* info)
  : LIR_Op1(lir_branch, LIR_OprFact::illegalOpr, info)
  , _cond(cond)
  , _label(block->label())
  , _block(block)
  , _stub(NULL)
  , _ulabel(NULL) {
  assert(info == NULL || cond == LIR_OpBranch::always, "shouldn't have infos on conditional branches");
}

LIR_OpBranch::LIR_OpBranch(LIR_Condition cond, CodeStub* stub, CodeEmitInfo* info) :
  LIR_Op1(lir_branch, LIR_OprFact::illegalOpr, info)
  , _cond(cond)
  , _label(stub->entry())
  , _block(NULL)
  , _stub(stub)
  , _ulabel(NULL) {
  assert(info == NULL || cond == LIR_OpBranch::always, "shouldn't have infos on conditional branches");
}



LIR_OpTypeCheck::LIR_OpTypeCheck(LIR_Code code, LIR_Opr result, LIR_Opr object, ciKlass* klass,
                                 LIR_Opr tmp1, LIR_Opr tmp2, bool fast_check,
                                 CodeEmitInfo* info_for_exception, CodeEmitInfo* info_for_patch,
                                 CodeStub* stub)
  : LIR_Op(code, result, NULL)
  , _object(object)
  , _array(LIR_OprFact::illegalOpr)
  , _klass(klass)
  , _tmp1(tmp1)
  , _tmp2(tmp2)
  , _tmp3(LIR_OprFact::illegalOpr)
  , _fast_check(fast_check)
  , _stub(stub)
  , _info_for_patch(info_for_patch)
  , _info_for_exception(info_for_exception) {
  if (code == lir_checkcast) {
    assert(info_for_exception != NULL, "checkcast throws exceptions");
  } else if (code == lir_instanceof) {
    assert(info_for_exception == NULL, "instanceof throws no exceptions");
  } else {
    ShouldNotReachHere();
  }
}



LIR_OpTypeCheck::LIR_OpTypeCheck(LIR_Code code, LIR_Opr object, LIR_Opr array, LIR_Opr tmp1, LIR_Opr tmp2, LIR_Opr tmp3, CodeEmitInfo* info_for_exception)
  : LIR_Op(code, LIR_OprFact::illegalOpr, NULL)
  , _object(object)
  , _array(array)
  , _klass(NULL)
  , _tmp1(tmp1)
  , _tmp2(tmp2)
  , _tmp3(tmp3)
  , _fast_check(false)
  , _stub(NULL)
  , _info_for_patch(NULL)
  , _info_for_exception(info_for_exception) {
  if (code == lir_store_check) {
    _stub = new ArrayStoreExceptionStub(info_for_exception);
    assert(info_for_exception != NULL, "store_check throws exceptions");
  } else {
    ShouldNotReachHere();
  }
}



//-------------------visits--------------------------

void LIR_Op::visit(LIR_OpVisitState* visitor) {
  if (_result->is_valid())       visitor->do_output(_result);
  if (_info)                     visitor->do_info(_info);
}


void LIR_OpArrayCopy::visit(LIR_OpVisitState* visitor) {
  assert(_result->is_illegal(), "unused");
  assert(_src->is_valid(), "used");          visitor->do_temp(_src);
  assert(_src_pos->is_valid(), "used");      visitor->do_temp(_src_pos);
  assert(_dst->is_valid(), "used");          visitor->do_temp(_dst);
  assert(_dst_pos->is_valid(), "used");      visitor->do_temp(_dst_pos);
  assert(_length->is_valid(), "used");       visitor->do_temp(_length);
  assert(_tmp->is_valid(), "used");          visitor->do_temp(_tmp);
  if (_info)                     visitor->do_info(_info);
  visitor->do_call();
}


void LIR_OpAllocObj::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  if (_opr->is_valid())          visitor->do_input(_opr);
  if (_tmp1->is_valid())         visitor->do_temp(_tmp1);
  if (_tmp2->is_valid())         visitor->do_temp(_tmp2);
  if (_tmp3->is_valid())         visitor->do_temp(_tmp3);
  if (_result->is_valid())       visitor->do_output(_result);
  if (_stub)                     _stub->visit(visitor);
}


void LIR_OpAllocArray::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  if (_opr1->is_valid())         visitor->do_input(_opr1);
  if (_opr2->is_valid())         visitor->do_input(_opr2);
  if (_tmp1->is_valid())         visitor->do_temp(_tmp1);
  if (_tmp2->is_valid())         visitor->do_temp(_tmp2);
  if (_tmp3->is_valid())         visitor->do_temp(_tmp3);
  if (_tmp4->is_valid())         visitor->do_temp(_tmp4);
  if (_result->is_valid())       visitor->do_output(_result);
  if (_stub)                     _stub->visit(visitor);
}


void LIR_OpTypeCheck::visit(LIR_OpVisitState* visitor) {
  if (_info_for_exception)       visitor->do_info(_info_for_exception);
  if (_info_for_patch)           visitor->do_info(_info_for_patch);
  if (_object->is_valid())       visitor->do_input(_object);
  if (_array->is_valid())        visitor->do_input(_array);
  if (_tmp1->is_valid())         visitor->do_temp(_tmp1);
  if (_tmp2->is_valid())         visitor->do_temp(_tmp2);
  if (_tmp3->is_valid())         visitor->do_temp(_tmp3);
  if (_result->is_valid())       visitor->do_output(_result);
  if (_stub)                     _stub->visit(visitor);
}


void LIR_OpCompareAndSwap::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  if (_addr->is_valid())         visitor->do_input(_addr);
  if (_cmp_value->is_valid())    visitor->do_input(_cmp_value);
  if (_new_value->is_valid())    visitor->do_input(_new_value);
  if (_tmp1->is_valid())         visitor->do_temp(_tmp1);
  if (_tmp2->is_valid())         visitor->do_temp(_tmp2);
  if (_result->is_valid())       visitor->do_output(_result);
}


void LIR_Op1::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  if (code() == lir_new_multi) {
    if (_opr->is_valid())          visitor->do_input(_opr);
    if (_result->is_valid())       visitor->do_input(_result);
    visitor->do_call();
    if (_result->is_valid())       visitor->do_output(_result);
  } else if (code() == lir_return ||
             code() == lir_safepoint) {
    assert(_result->is_illegal(), "unused");
    if (_opr->is_valid())          visitor->do_temp(_opr);
  } else {
    if (_opr->is_valid())          visitor->do_input(_opr);
    if (_result->is_valid())       visitor->do_output(_result);
  }
}

void LIR_Op2::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  if (code() == lir_logic_orcc) {
    assert(_opr1->is_valid(), "used");      visitor->do_temp(_opr1);
    assert(_opr2->is_valid(), "used");      visitor->do_temp(_opr2);
    assert(_result->is_valid(), "used");    visitor->do_temp(_result);
  } else if (code() == lir_throw || code() == lir_unwind) {
    if (_opr1->is_valid())         visitor->do_temp(_opr1);
    if (_opr2->is_valid())         visitor->do_temp(_opr2);
    assert(_result->is_illegal(), "no result");
  } else {
    if (_opr1->is_valid())         visitor->do_input(_opr1);
    if (_opr2->is_valid())         visitor->do_input(_opr2);
    if (_tmp->is_valid())          visitor->do_temp(_tmp);
    if (_result->is_valid())       visitor->do_output(_result);
  }
}

void LIR_Op3::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  if (_opr1->is_valid())         visitor->do_input(_opr1);
  if (_opr2->is_valid())         visitor->do_input(_opr2);
  if (_opr3->is_valid())         visitor->do_input(_opr3);
  if (_result->is_valid())       visitor->do_output(_result);
}

void LIR_OpLabel::visit(LIR_OpVisitState* visitor) {
  assert(_result->is_illegal(), "unused");
  if (_info)                     visitor->do_info(_info);
}

void LIR_OpCall::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  visitor->do_call();
  if (_result->is_valid())       visitor->do_output(_result);
}

void LIR_OpJavaCall::visit(LIR_OpVisitState* visitor) {
  if (_receiver.is_valid())      visitor->do_rinfo(_receiver);
  if (_info)                     visitor->do_info(_info);
  if (_stub != NULL)             _stub->visit(visitor);
  visitor->do_call();
  if (_result->is_valid())       visitor->do_output(_result);
}

void LIR_OpRTCall::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  visitor->do_call();
  if (_tmp->is_valid())          visitor->do_temp(_tmp);
  if (_result->is_valid())       visitor->do_output(_result);
}

void LIR_OpBranch::visit(LIR_OpVisitState* visitor) {
  LIR_Op1::visit(visitor);
  if (stub())                    stub()->visit(visitor);
}

void LIR_OpDelay::visit(LIR_OpVisitState* visitor) {
  delay_op()->visit(visitor);
}

void LIR_OpLock::visit(LIR_OpVisitState* visitor) {
  if (_info)                     visitor->do_info(_info);
  if (_hdr->is_valid())          visitor->do_input(_hdr);
  if (_obj->is_valid())          visitor->do_input(_obj);
  if (_lock->is_valid())         visitor->do_input(_lock);
  if (_scratch->is_valid())      visitor->do_temp(_scratch);
  if (_result->is_valid())       visitor->do_output(_result);
  if (_stub)                     _stub->visit(visitor);
}

//---------------------------------------------------


void LIR_OpJavaCall::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_call(this);
}

void LIR_OpRTCall::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_rtcall(this);
}

void LIR_OpLabel::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_opLabel(this); 
}

void LIR_OpArrayCopy::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_arraycopy(this);
}

void LIR_Op0::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_op0(this); 
}

void LIR_Op1::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_op1(this); 
}

void LIR_OpAllocObj::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_alloc_obj(this); 
  if (stub()) {
    masm->emit_code_stub(stub());
  }
}

void LIR_OpBranch::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_opBranch(this); 
  if (stub()) {
    masm->emit_code_stub(stub());
  }
}

void LIR_OpConvert::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_opConvert(this); 
}

void LIR_Op2::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_op2(this); 
}

void LIR_OpAllocArray::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_alloc_array(this); 
  if (stub()) {
    masm->emit_code_stub(stub());
  }
}

void LIR_OpTypeCheck::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_opTypeCheck(this); 
  if (stub()) {
    masm->emit_code_stub(stub());
  }
}

void LIR_OpCompareAndSwap::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_compare_and_swap(this);
}

void LIR_Op3::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_op3(this); 
}

void LIR_OpLock::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_lock(this);
  if (stub()) {
    masm->emit_code_stub(stub());
  }
}


void LIR_OpDelay::emit_code(LIR_AbstractAssembler* masm) {
  masm->emit_delay(this);
}


// LIR_List
LIR_List::LIR_List(Compilation* compilation) {
  _compilation = compilation;
  _insts = new LIR_OpList();
  // NEEDS_CLEANUP
  // is it needed to allocate stubs every time that we allocate list ?
  _stubs = new CodeStubList();
#ifdef SPARC
  _delayed = false;
#endif // SPARC
}


void LIR_List::oop2reg_patch(jobject o, RInfo reg, CodeEmitInfo* info) {
  append(new LIR_Op1(lir_move, LIR_OprFact::oopConst(o),  LIR_OprFact::rinfo(reg, T_OBJECT), T_OBJECT, LIR_Op1::patch_normal, info));
}


void LIR_List::load_mem_reg(RInfo base, int offset_in_bytes, RInfo dst, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, offset_in_bytes)), 
            LIR_OprFact::rinfo(dst, type),
            type,
            patch_code, 
            info));
}


void LIR_List::load_mem_reg(LIR_Address* addr, RInfo dst,  BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            LIR_OprFact::address(addr),
            LIR_OprFact::rinfo(dst, type),
            type,
            patch_code, 
            info));
}


void LIR_List::load(LIR_Address* addr, LIR_Opr src, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            LIR_OprFact::address(addr),
            src,
            type,
            patch_code, 
            info));
}


void LIR_List::volatile_load_mem_reg(RInfo base, int offset_in_bytes, RInfo dst, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_volatile_move, 
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, offset_in_bytes)), 
            LIR_OprFact::rinfo(dst, type),
            type,
            patch_code, 
            info));
}

void LIR_List::volatile_load_unsafe_reg(RInfo base, RInfo offset, RInfo dst, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_volatile_move, 
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), 
                                                 LIR_OprFact::rinfo(offset),
                                                 0)), 
            LIR_OprFact::rinfo(dst, type),
            type,
            patch_code, 
            info));
}


void LIR_List::store_mem_int(jint v, RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            LIR_OprFact::intConst(v),
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, offset_in_bytes)),
            type,
            patch_code, 
            info));
}


void LIR_List::store_mem_oop(jobject o, RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            LIR_OprFact::oopConst(o),
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, offset_in_bytes)),
            type,
            patch_code, 
            info));
}


void LIR_List::store_mem_reg(RInfo src, RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            LIR_OprFact::rinfo(src, type),
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, offset_in_bytes)),
            type,
            patch_code, 
            info));
}


void LIR_List::store_mem_reg(RInfo src, LIR_Address* addr, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            LIR_OprFact::rinfo(src, type),
            LIR_OprFact::address(addr),
            type,
            patch_code, 
            info));
}


void LIR_List::store(LIR_Opr src, LIR_Address* addr, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_move, 
            src,
            LIR_OprFact::address(addr),
            type,
            patch_code, 
            info));
}


void LIR_List::volatile_store_mem_reg(RInfo src, RInfo base, int offset_in_bytes, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_volatile_move,
            LIR_OprFact::rinfo(src, type),
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, offset_in_bytes)),
            type,
            patch_code,
            info));
}

void LIR_List::volatile_store_unsafe_reg(RInfo src, RInfo base, RInfo offset, BasicType type, CodeEmitInfo* info, LIR_Op1::LIR_PatchCode patch_code) {
  append(new LIR_Op1(
            lir_volatile_move,
            LIR_OprFact::rinfo(src, type),
            LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), 
                                                 LIR_OprFact::rinfo(offset),
                                                 0)),
            type,
            patch_code,
            info));
}


void LIR_List::jump(BlockBegin* block, CodeEmitInfo* info) {
  append(new LIR_OpBranch(LIR_OpBranch::always, block, info));
}

void LIR_List::branch(LIR_OpBranch::LIR_Condition cond, Label* lbl) {
  append(new LIR_OpBranch(cond, lbl, NULL));
}

void LIR_List::branch(LIR_OpBranch::LIR_Condition cond, BlockBegin* block) {
  append(new LIR_OpBranch(cond, block, NULL));
}

void LIR_List::branch(LIR_OpBranch::LIR_Condition cond, CodeStub* stub) {
  append(new LIR_OpBranch(cond, stub, NULL));
}

void LIR_List::branch_float(LIR_OpBranch::LIR_Condition cond, Label* lbl, Label* ulbl) {
  append(new LIR_OpBranch(cond, lbl, ulbl, NULL));
}


void LIR_List::load_array(LIR_Address* addr, BasicType t, RInfo dst, CodeEmitInfo* info) {
  append(new LIR_Op1(lir_array_move, LIR_OprFact::address(addr), LIR_OprFact::rinfo(dst), t, LIR_Op1::patch_none, info)); 
}


void LIR_List::store_array(RInfo src, LIR_Address* addr, BasicType t, CodeEmitInfo* info) {
  append(new LIR_Op1(lir_array_move, LIR_OprFact::rinfo(src), LIR_OprFact::address(addr), t, LIR_Op1::patch_none, info)); 
}


void LIR_List::store_array(jint src, LIR_Address* addr, BasicType t, CodeEmitInfo* info) {
  append(new LIR_Op1(lir_array_move, LIR_OprFact::intConst(src), LIR_OprFact::address(addr), t, LIR_Op1::patch_none, info)); 
}


void LIR_List::store_array(jobject src, LIR_Address* addr, BasicType t, CodeEmitInfo* info) {
  append(new LIR_Op1(lir_array_move, LIR_OprFact::oopConst(src), LIR_OprFact::address(addr), t, LIR_Op1::patch_none, info)); 
}


void LIR_List::idiv(RInfo left, RInfo right, RInfo res, RInfo tmp, CodeEmitInfo* info) {
  append(new LIR_Op3(
                    lir_idiv,
                    LIR_OprFact::rinfo(left),
                    LIR_OprFact::rinfo(right),
                    LIR_OprFact::rinfo(tmp),
                    LIR_OprFact::rinfo(res),
                    info));
}


void LIR_List::idiv(RInfo left, int right, RInfo res, RInfo tmp, CodeEmitInfo* info) {
  append(new LIR_Op3(
                    lir_idiv,
                    LIR_OprFact::rinfo(left),
                    LIR_OprFact::intConst(right),
                    LIR_OprFact::rinfo(tmp),
                    LIR_OprFact::rinfo(res),
                    info));
}


void LIR_List::irem(RInfo left, RInfo right, RInfo res, RInfo tmp, CodeEmitInfo* info) {
  append(new LIR_Op3(
                    lir_irem,
                    LIR_OprFact::rinfo(left),
                    LIR_OprFact::rinfo(right),
                    LIR_OprFact::rinfo(tmp),
                    LIR_OprFact::rinfo(res),
                    info));
}


void LIR_List::irem(RInfo left, int right, RInfo res, RInfo tmp, CodeEmitInfo* info) {
  append(new LIR_Op3(
                    lir_irem,
                    LIR_OprFact::rinfo(left),
                    LIR_OprFact::intConst(right),
                    LIR_OprFact::rinfo(tmp),
                    LIR_OprFact::rinfo(res),
                    info));
}


void LIR_List::cmp_mem_int(LIR_OpBranch::LIR_Condition condition, RInfo base, int disp, int c, CodeEmitInfo* info) { 
  append(new LIR_Op2(
                    lir_cmp,
                    condition,
                    LIR_OprFact::address(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, disp)), 
                    LIR_OprFact::intConst(c), 
                    info, T_INT)); 
}


void LIR_List::cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, LIR_Address* addr, BasicType type, CodeEmitInfo* info) { 
  append(new LIR_Op2(
                    lir_cmp, 
                    condition,
                    LIR_OprFact::rinfo(reg, type), 
                    LIR_OprFact::address(addr),
                    info, type));
}

void LIR_List::allocate_object(RInfo dst, RInfo t1, RInfo t2, RInfo t3, int header_size, int object_size, RInfo klass, CodeStub* stub) {
  append(new LIR_OpAllocObj(
                           LIR_OprFact::rinfo(klass),
                           LIR_OprFact::rinfo(dst),
                           LIR_OprFact::rinfo(t1),
                           LIR_OprFact::rinfo(t2),
                           LIR_OprFact::rinfo(t3),
                           header_size,
                           object_size,
                           stub));
}

void LIR_List::allocate_array(RInfo dst, RInfo len, RInfo t1,RInfo t2, RInfo t3,RInfo t4, BasicType type, RInfo klass, CodeStub* stub) {
  append(new LIR_OpAllocArray(
                           LIR_OprFact::rinfo(klass),
                           LIR_OprFact::rinfo(len),
                           LIR_OprFact::rinfo(dst),
                           LIR_OprFact::rinfo(t1),
                           LIR_OprFact::rinfo(t2),
                           LIR_OprFact::rinfo(t3),
                           LIR_OprFact::rinfo(t4),
                           type,
                           stub));
}

void LIR_List::shift_left(LIR_Opr value, LIR_Opr count, LIR_Opr dst, LIR_Opr tmp) {
 append(new LIR_Op2(
                    lir_shl, 
                    value, 
                    count, 
                    dst,
                    tmp));
}

// Shifts a 64 bit register left
void LIR_List::shift_left_long(RInfo value, RInfo count, RInfo dst, RInfo tmp) {
 append(new LIR_Op2(
                    lir_shlx, 
                    LIR_OprFact::rinfo(value), 
                    LIR_OprFact::rinfo(count), 
                    LIR_OprFact::rinfo(dst),
                    LIR_OprFact::rinfo(tmp)));
}


void LIR_List::shift_right(LIR_Opr value, LIR_Opr count, LIR_Opr dst, LIR_Opr tmp) {
 append(new LIR_Op2(
                    lir_shr, 
                    value, 
                    count, 
                    dst,
                    tmp));
}


void LIR_List::unsigned_shift_right(LIR_Opr value, LIR_Opr count, LIR_Opr dst, LIR_Opr tmp) {
 append(new LIR_Op2(
                    lir_ushr, 
                    value, 
                    count, 
                    dst,
                    tmp));
}

void LIR_List::fcmp2int(RInfo left, RInfo right, RInfo dst, bool is_unordered_less) {
  append(new LIR_Op2(is_unordered_less ? lir_ucmp_fd2i : lir_cmp_fd2i,  
                     LIR_OprFact::rinfo(left),
                     LIR_OprFact::rinfo(right),
                     LIR_OprFact::rinfo(dst)));
}

void LIR_List::lock_object(RInfo hdr, RInfo obj, RInfo lock, RInfo scratch, CodeStub* stub, CodeEmitInfo* info) {
  append(new LIR_OpLock(
                    lir_lock, 
                    LIR_OprFact::rinfo(hdr),
                    LIR_OprFact::rinfo(obj),
                    LIR_OprFact::rinfo(lock),
                    LIR_OprFact::rinfo(scratch),
                    stub,
                    info));
}

void LIR_List::unlock_object(RInfo hdr, RInfo obj, RInfo lock, CodeStub* stub) {
  append(new LIR_OpLock(
                    lir_unlock, 
                    LIR_OprFact::rinfo(hdr),
                    LIR_OprFact::rinfo(obj),
                    LIR_OprFact::rinfo(lock),
                    LIR_OprFact::rinfo(norinfo),
                    stub,
                    NULL));
}


void LIR_List::single_stack2reg(int locIx, RInfo reg, BasicType type)  { 
  assert(!reg.is_long(), "cannot be long");
  assert(!((type == T_FLOAT) ^ (reg.is_float())), "type mismatch");
  append(new LIR_Op1(lir_move, LIR_OprFact::single_stack(locIx, type),   LIR_OprFact::rinfo(reg, type), type)); 
}

void LIR_List::double_stack2reg(int locIx, RInfo reg, BasicType type)  { 
//  assert(!reg.is_long(), "cannot be long");
  append(new LIR_Op1(lir_move, LIR_OprFact::double_stack(locIx, type),   LIR_OprFact::rinfo(reg, type), type)); 
}


void check_LIR() {
  // cannot do the proper checking as PRODUCT and other modes return different results
  // guarantee(sizeof(LIR_OprDesc) == wordSize, "may not have a v-table");
}



void LIR_List::checkcast (LIR_Opr result, LIR_Opr object, ciKlass* klass,
                          LIR_Opr tmp1, LIR_Opr tmp2, bool fast_check,
                          CodeEmitInfo* info_for_exception, CodeEmitInfo* info_for_patch, CodeStub* stub) {
  append(new LIR_OpTypeCheck(lir_checkcast, result, object, klass,
                             tmp1, tmp2, fast_check, info_for_exception, info_for_patch, stub));
}


void LIR_List::instanceof(LIR_Opr result, LIR_Opr object, ciKlass* klass, LIR_Opr tmp1, LIR_Opr tmp2, bool fast_check, CodeEmitInfo* info_for_patch) {
  append(new LIR_OpTypeCheck(lir_instanceof, result, object, klass, tmp1, tmp2, fast_check, NULL, info_for_patch, NULL));
}


void LIR_List::store_check(LIR_Opr object, LIR_Opr array, LIR_Opr tmp1, LIR_Opr tmp2, LIR_Opr tmp3, CodeEmitInfo* info_for_exception) {
  append(new LIR_OpTypeCheck(lir_store_check, object, array, tmp1, tmp2, tmp3, info_for_exception));
}


void LIR_List::cas_long(LIR_Opr addr, LIR_Opr cmp_value, LIR_Opr new_value, LIR_Opr t1, LIR_Opr t2) {
  // Compare and swap produces condition code "zero" if contents_of(addr) == cmp_value,
  // implying successful swap of new_value into addr
  append(new LIR_OpCompareAndSwap(lir_cas_long, addr, cmp_value, new_value, t1, t2));
}

void LIR_List::cas_obj(LIR_Opr addr, LIR_Opr cmp_value, LIR_Opr new_value, LIR_Opr t1, LIR_Opr t2) {
  // Compare and swap produces condition code "zero" if contents_of(addr) == cmp_value,
  // implying successful swap of new_value into addr
  append(new LIR_OpCompareAndSwap(lir_cas_obj, addr, cmp_value, new_value, t1, t2));
}

void LIR_List::cas_int(LIR_Opr addr, LIR_Opr cmp_value, LIR_Opr new_value, LIR_Opr t1, LIR_Opr t2) {
  // Compare and swap produces condition code "zero" if contents_of(addr) == cmp_value,
  // implying successful swap of new_value into addr
  append(new LIR_OpCompareAndSwap(lir_cas_int, addr, cmp_value, new_value, t1, t2));
}


#ifdef PRODUCT

void print_LIR(BlockList* blocks) {
}

#else
// LIR_OprDesc
void LIR_OprDesc::print() {
  if (is_illegal()) {
    return;
  }

  tty->print("[");
  if (is_pointer()) {
    pointer()->print();
  } else if (is_single_stack()) {
    tty->print("stack:%d", single_stack_ix());
  } else if (is_double_stack()) {
    tty->print("dbl_stack:%d",double_stack_ix());
  } else if (is_single_cpu()) {
    rinfo().print();
  } else if (is_double_cpu()) {
    tty->print("%d:", cpu_regnrHi());rinfo().as_rinfo_hi().print();
    tty->print(",%d:",cpu_regnrLo());rinfo().as_rinfo_lo().print();
  } else if (is_single_fpu()) {
    rinfo().print();
  } else if (is_double_fpu()) {
    tty->print("%d", fpu_regnrHi());
    tty->print(",%d:",fpu_regnrLo());rinfo().print();
  } else if (is_illegal()) {
    tty->print("-");
  } else {
    tty->print("Unknown Operand");
  }
  if (!is_illegal()) {
    tty->print("|%c", type_char());
  }
  tty->print("]");
}


// LIR_Address
void LIR_Const::print() {
  switch (type()) {
    case T_INT:    tty->print("int:%d",   as_jint());           break;
    case T_LONG:   tty->print("lng:%lld", as_jlong());          break;
    case T_FLOAT:  tty->print("flt:%f",   as_jfloat());         break;
    case T_DOUBLE: tty->print("dbl:%f",   as_jdouble());        break;
    case T_OBJECT: tty->print("obj:0x%x", as_jobject());        break;
    default:       tty->print("%3d:0x%x",type(), as_jdouble()); break;
  }
}

// LIR_Address
void LIR_Address::print() {
  tty->print("Base:"); _base->print();
  if (!_index->is_illegal()) {
    tty->print(" Index:"); _index->print();
    switch (scale()) {
    case times_1: break;
    case times_2: tty->print(" * 2"); break;
    case times_4: tty->print(" * 4"); break;
    case times_8: tty->print(" * 8"); break;
    }
  }
  tty->print(" Disp: %d", _disp); 
}

void print_LIR(BlockList* blocks) {
  tty->print_cr("LIR:");
  for (int i = 0; i < blocks->length(); i++) {
    InstructionPrinter ip;
    BlockBegin* bb = blocks->at(i);
    ip.print_instr(bb); tty->cr();
    tty->print("__address__Instruction___________________________________________"); tty->cr();
    bb->lir()->print_instructions();
  }
}

void LIR_List::print_instructions() {
  for (int i = 0; i < _insts->length(); i++) {
    _insts->at(i)->print();
  }
  tty->cr();
}

// LIR_Ops printing routines
// LIR_Op
void LIR_Op::print_on(outputStream* out) const {
  tty->print("0x%x: ", this); 
  print_code(code()); tty->print(" ");
  print_instr();
  if (info() != NULL) tty->print(" [bci:%d]", info()->bci());
  tty->cr();
}

void LIR_Op::print_code(LIR_Code code) {
  const char* s = NULL;
  switch(code) {
     // LIR_Op0
     case lir_membar:                s = "membar";        break;
     case lir_membar_acquire:        s = "membar_acquire"; break;
     case lir_membar_release:        s = "membar_release"; break;
     case lir_word_align:            s = "word_align";    break;
     case lir_label:                 s = "label";         break;
     case lir_nop:                   s = "nop";           break;
     case lir_backwardbranch_target: s = "backbranch";    break;
     case lir_align_entry:           s = "align_entr";    break;
     case lir_verified_entry:        s = "vep";           break;
     case lir_build_frame:           s = "build_frm";     break;
     case lir_fpop_raw:              s = "fpop_raw";      break;
     case lir_empty_fpu:             s = "empty_fpu";     break;
     case lir_24bit_FPU:             s = "24bit_FPU";     break;
     case lir_reset_FPU:             s = "reset_FPU";     break;
     case lir_breakpoint:            s = "breakpoint";    break;
     case lir_get_thread:            s = "get_thread";    break;
     // LIR_Op1
     case lir_fpu_push:              s = "fpu_push";      break;
     case lir_fpu_pop:               s = "fpu_pop";       break;
     case lir_fpu_dup:               s = "fpu_dup";       break;
     case lir_push:                  s = "push";          break;
     case lir_pop:                   s = "pop";           break;
     case lir_null_check:            s = "null_check";    break;
     case lir_return:                s = "return";        break;
     case lir_safepoint:             s = "safepoint";     break;
     case lir_neg:                   s = "neg";           break;
     case lir_leal:                  s = "leal";          break;
     case lir_branch:                s = "branch";        break;
     case lir_cond_float_branch:     s = "flt_cond_br";   break;
     case lir_move:                  s = "move";          break;
     case lir_volatile_move:         s = "volatile_move"; break;
     case lir_round32:               s = "round32";       break;
     case lir_array_move:            s = "array_move";    break;
     case lir_rtcall:                s = "rtcall";        break;
     case lir_jvmpi_method_enter:    s = "jvmpi_enter";   break;
     case lir_jvmpi_method_exit:     s = "jvmpi_exit";    break;
     case lir_throw:                 s = "throw";         break;
     case lir_unwind:                s = "unwind";        break;
     case lir_convert:               s = "convert";       break;
     case lir_fast_convert:          s = "fast_convert";  break;
     case lir_alloc_object:          s = "alloc_obj";     break;
     case lir_monaddr:               s = "mon_addr";      break;
     case lir_new_multi:             s = "new_multi";     break;
     // LIR_Op2
     case lir_cmp:                   s = "cmp";           break;
     case lir_cmp_l2i:               s = "cmp_l2i";       break;
     case lir_ucmp_fd2i:             s = "ucomp_fd2i";    break;
     case lir_cmp_fd2i:              s = "comp_fd2i";     break;
     case lir_add:                   s = "add";           break;
     case lir_sub:                   s = "sub";           break;
     case lir_mul:                   s = "mul";           break;
     case lir_mul_strictfp:          s = "mul_strictfp";  break;
     case lir_div:                   s = "div";           break;
     case lir_div_strictfp:          s = "div_strictfp";  break;
     case lir_rem:                   s = "rem";           break;
     case lir_sqrt:                  s = "sqrt";          break;
     case lir_sin:                   s = "sin";           break;
     case lir_cos:                   s = "cos";           break;
     case lir_unverified_entry:      s = "uep";           break;
     case lir_logic_and:             s = "logic_and";     break;
     case lir_logic_or:              s = "logic_or";      break;
     case lir_logic_orcc:            s = "logic_orcc";    break;
     case lir_logic_xor:             s = "logic_xor";     break;
     case lir_shl:                   s = "shift_left";    break;
     case lir_shlx:                  s = "shift_left_long";break;
     case lir_shr:                   s = "shift_right";   break;
     case lir_ushr:                  s = "ushift_right";  break;
     case lir_alloc_array:           s = "alloc_array";   break;
     // LIR_Op3
     case lir_idiv:                  s = "idiv";          break;
     case lir_irem:                  s = "irem";          break;
     // LIR_OpJavaCall
     case lir_static_call:           s = "static";        break;
     case lir_optvirtual_call:       s = "optvirtual";    break;
     case lir_icvirtual_call:        s = "icvirtual";     break;
     case lir_virtual_call:          s = "virtual";       break;
     // LIR_OpArrayCopy
     case lir_arraycopy:             s = "arraycopy";     break;
     // LIR_OpLock
     case lir_lock:                  s = "lock";          break;
     case lir_unlock:                s = "unlock";        break;
     // LIR_OpDelay
     case lir_delay_slot:            s = "delay";         break;
     // LIR_OpTypeCheck
     case lir_instanceof:            s = "instanceof";    break;
     case lir_checkcast:             s = "checkcast";     break;
     case lir_store_check:           s = "store_check";   break;
     // LIR_OpCompareAndSwap
     case lir_cas_long:              s = "cas_long";      break;
     case lir_cas_obj:               s = "cas_obj";      break;
     case lir_cas_int:               s = "cas_int";      break;

     case lir_none:                  ShouldNotReachHere();break;
    default:                         s = "illegal_op";    break;
  }
  tty->print("%12s", s);
}

// LIR_OpJavaCall
void LIR_OpJavaCall::print_instr() const {
  tty->print("call: ");
  tty->print("[addr: 0x%x]", address());
  if (receiver().is_valid()) {
    tty->print(" [recv: ");   receiver().print();    tty->print("]");
  }
  if (result_opr()->is_valid()) {
    tty->print(" [result: "); result_opr()->print(); tty->print("]");
  }
}

// LIR_OpLabel
void LIR_OpLabel::print_instr() const {
  tty->print("[label:0x%x]", _label);
}

// LIR_OpArrayCopy
void LIR_OpArrayCopy::print_instr() const {
  src()->print();     tty->print(" ");
  src_pos()->print(); tty->print(" ");
  dst()->print();     tty->print(" ");
  dst_pos()->print(); tty->print(" ");
  length()->print();  tty->print(" ");
  tmp()->print();     tty->print(" ");
}

// LIR_OpCompareAndSwap
void LIR_OpCompareAndSwap::print_instr() const {
  addr()->print();      tty->print(" ");
  cmp_value()->print(); tty->print(" ");
  new_value()->print(); tty->print(" ");
  tmp1()->print();      tty->print(" ");
  tmp2()->print();      tty->print(" ");

}

// LIR_Op0
void LIR_Op0::print_instr() const {
  result_opr()->print(); 
}

// LIR_Op1
void LIR_Op1::print_instr() const {
  _opr->print();         tty->print(" ");
  result_opr()->print(); tty->print(" ");
  print_patch_code(patch_code());
}


#define FUNCTION_CASE(a, f) \
  if (a == CAST_FROM_FN_PTR(intx, f))  { tty->print(#f); } else 

// LIR_Op1
void LIR_OpRTCall::print_instr() const {
  tty->print("RTCall ");
  intx a = (intx)addr();
  FUNCTION_CASE(a, SharedRuntime::dsin)
  FUNCTION_CASE(a, SharedRuntime::dcos)
  tty->print("0x%x ", a);
  tmp()->print();
}

void LIR_Op1::print_patch_code(LIR_PatchCode code) {
  switch(code) {
    case patch_none:                                 break;
    case patch_low:    tty->print("[patch_low]");    break;
    case patch_high:   tty->print("[patch_high]");   break;
    case patch_normal: tty->print("[patch_normal]"); break;
    default: ShouldNotReachHere();
  }
}

// LIR_OpBranch
void LIR_OpBranch::print_instr() const {
  print_condition(cond());             tty->print(" ");
  if (cond_reg()->is_valid()) {
    cond_reg()->print();                 tty->print(" ");
  }
  if (block() != NULL) {
    tty->print("[B%d] ", block()->block_id());
  } else if (stub() != NULL) {
    tty->print("[");
    stub()->print_name();
    tty->print(": 0x%x]", stub());
    if (stub()->info() != NULL) tty->print(" [bci:%d]", stub()->info()->bci());
  } else {
    tty->print("[label:0x%x] ", label());
  }
  if (ulabel() != NULL) {
    tty->print("[ulabel:0x%x] ", ulabel());
  }
}

void LIR_OpBranch::print_condition(LIR_Condition cond) {
  switch(cond) {
    case equal:           tty->print("[EQ]");      break; 
    case notEqual:        tty->print("[NE]");      break;
    case less:            tty->print("[LT]");      break; 
    case lessEqual:       tty->print("[LE]");      break; 
    case greaterEqual:    tty->print("[GE]");      break; 
    case greater:         tty->print("[GT]");      break; 
    case belowEqual:      tty->print("[BE]");      break; 
    case aboveEqual:      tty->print("[AE]");      break; 
    case always:          tty->print("[AL]");      break;
    case intrinsicFailed: tty->print("[??]");      break;
    default:              tty->print("[%d]",cond); break;
  }
}

// LIR_OpConvert
void LIR_OpConvert::print_instr() const {
  print_bytecode(bytecode());
  in_opr()->print();                  tty->print(" ");
  result_opr()->print();              tty->print(" ");
}

void LIR_OpConvert::print_bytecode(Bytecodes::Code code) {
  switch(code) {
    case Bytecodes::_d2f: tty->print("[d2f] "); break; 
    case Bytecodes::_d2i: tty->print("[f2i] "); break; 
    case Bytecodes::_d2l: tty->print("[d2l] "); break;
    case Bytecodes::_f2d: tty->print("[f2d] "); break; 
    case Bytecodes::_f2i: tty->print("[f2i] "); break; 
    case Bytecodes::_f2l: tty->print("[f2l] "); break;
    case Bytecodes::_i2b: tty->print("[i2b] "); break;
    case Bytecodes::_i2c: tty->print("[i2c] "); break;
    case Bytecodes::_i2d: tty->print("[i2d] "); break;
    case Bytecodes::_i2f: tty->print("[i2f] "); break; 
    case Bytecodes::_i2l: tty->print("[i2l] "); break; 
    case Bytecodes::_i2s: tty->print("[i2s] "); break; 
    case Bytecodes::_l2i: tty->print("[l2i] "); break; 
    default:
      tty->print("[?%d]",code);
    break;
  }
}

void LIR_OpAllocObj::print_instr() const {
  klass()->print();                      tty->print(" ");
  obj()->print();                        tty->print(" ");
  tmp1()->print();                       tty->print(" ");
  tmp2()->print();                       tty->print(" ");
  tmp3()->print();                       tty->print(" ");
  tty->print("[hdr:%d]", header_size()); tty->print(" ");
  tty->print("[obj:%d]", object_size()); tty->print(" ");
  tty->print("[lbl:0x%x]", stub()->entry());
}

// LIR_Op2
void LIR_Op2::print_instr() const {
  in_opr1()->print();    tty->print(" ");
  in_opr2()->print();    tty->print(" ");
  result_opr()->print(); 
}

void LIR_OpAllocArray::print_instr() const {
  klass()->print();                   tty->print(" ");
  len()->print();                     tty->print(" ");
  obj()->print();                     tty->print(" ");
  tmp1()->print();                    tty->print(" ");
  tmp2()->print();                    tty->print(" ");
  tmp3()->print();                    tty->print(" ");
  tmp4()->print();                    tty->print(" ");
  tty->print("[ type:0x%x]", type()); tty->print(" ");
  tty->print("[label:0x%x]", stub()->entry());
}


void LIR_OpTypeCheck::print_instr() const {
  object()->print();                  tty->print(" ");
  if (code() == lir_store_check) {
    array()->print();                 tty->print(" ");
  }
  if (code() != lir_store_check) {
    klass()->print_name();            tty->print(" ");
    if (fast_check())                 tty->print("fast_check ");
  }
  tmp1()->print();                    tty->print(" ");
  tmp2()->print();                    tty->print(" ");
  tmp3()->print();                    tty->print(" ");
  result_opr()->print();              tty->print(" ");
}


// LIR_Op3
void LIR_Op3::print_instr() const {
  in_opr1()->print();    tty->print(" ");
  in_opr2()->print();    tty->print(" ");
  in_opr3()->print();    tty->print(" ");
  result_opr()->print();
}


void LIR_OpLock::print_instr() const {
  hdr_opr()->print();   tty->print(" ");
  obj_opr()->print();   tty->print(" ");
  lock_opr()->print();  tty->print(" ");
  tty->print("[lbl:0x%x]", stub()->entry());
}


void LIR_OpDelay::print_instr() const {
  _op->print();
}


#endif // PRODUCT

