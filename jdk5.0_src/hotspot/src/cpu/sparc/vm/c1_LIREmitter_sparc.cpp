#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIREmitter_sparc.cpp	1.49 03/12/23 16:37:02 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_LIREmitter_sparc.cpp.incl"

#define __ _lir->

int LIR_Emitter::frame_size() {
  assert(frame_map()->framesize() >= 0, "wrong frame_size");
  return frame_map()->framesize();
}

int LIR_Emitter::hi_word_offset_in_bytes() const {
  return 0;
}


int LIR_Emitter::lo_word_offset_in_bytes() const {
  return BytesPerInt; // big endian
}



void LIR_Emitter::monitorenter_at_entry(RInfo receiver, CodeEmitInfo* info) {
  CallingConvention* args = FrameMap::calling_convention(method());
  RInfo obj; // object to lock
  if (method()->is_static()) {
    obj = FrameMap::_L0_RInfo;
    __ oop2reg(method()->holder()->java_mirror()->encoding(), obj);
  } else {
    ArgumentLocation receiver = args->arg_at(0);
    if (receiver.is_register_arg()) {
      obj = receiver.incoming_reg_location();
      // obj is already loaded
    } else {
      Unimplemented();
      // we must pass receiver in register
    }
  }

  RInfo lock_reg = FrameMap::_L1_RInfo;
  CodeEmitInfo* info_for_exception = NULL;  // synchronized method entry cannot cause a NullPointerException
  // sync. slot is 0, however should really use a function here
  monitor_enter(obj, lock_reg, FrameMap::_G4_RInfo, FrameMap::_G3_RInfo, 0, info_for_exception, info);
}


void LIR_Emitter::pop_item(LIR_Opr item) {
  assert(item->is_float_kind(), "Only used for gross store elimination bug fix");
  // Do nothing
}


void LIR_Emitter::return_op_prolog (int monitor_no) {
  assert(method()->is_synchronized(), "must be synchronized");
  // do not use eax/edx
  RInfo obj_info  = FrameMap::_O0_RInfo;
  RInfo lock_info = FrameMap::_O1_RInfo;
  assert(monitor_no >= 0, "monitor slot must exist");
  // pass lock_reg, obj_reg as parameters to this function
  monitor_exit(obj_info, lock_info, FrameMap::_G5_RInfo, monitor_no);
}

void LIR_Emitter::new_type_array(RInfo dst, BasicType elem_type, LIR_Opr lengthItem, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo scratch4, RInfo klass_reg, CodeEmitInfo* info) {

  RInfo len = lengthItem->as_rinfo();
  CodeStub* slow_path = new NewTypeArrayStub(klass_reg, len, dst, info);
  __ oop2reg(ciTypeArrayKlass::make(elem_type)->encoding(), klass_reg);
  if (UseFastNewTypeArray && !UseSlowPath) {
    __ allocate_array(dst, len, scratch1, scratch2, scratch3, scratch4, elem_type, klass_reg, slow_path);
  } else {
    __ branch(LIR_OpBranch::always, slow_path);
    __ branch_destination(slow_path->continuation());
  }
}


void LIR_Emitter::new_object_array(RInfo dst, ciKlass* elem_klass, LIR_Opr length, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo scratch4, RInfo klass_reg, CodeEmitInfo* info, CodeEmitInfo* patching_info) {
  NewObjectArrayStub* slow_path = new NewObjectArrayStub(klass_reg, length->as_rinfo(), dst, info);
  ciObject* obj = (ciObject*) ciObjArrayKlass::make(elem_klass);
  if (obj == ciEnv::unloaded_ciobjarrayklass()) {
      set_bailout("encountered unloaded_ciobjarrayklass due to out of memory error");
      return;
  }
  jobject2reg_with_patching(klass_reg, obj, patching_info);

  if (UseFastNewObjectArray) {
    assert(oopSize == 4 || oopSize == 8, "change scale factor below");
    __ allocate_array(dst, length->as_rinfo(), scratch1, scratch2, scratch3, scratch4, T_OBJECT, klass_reg, slow_path);
  } else {
    __ branch(LIR_OpBranch::always, slow_path);
    __ branch_destination(slow_path->continuation());
  }
}


static int shift_amount(BasicType t) {
  int elem_size = type2aelembytes[t];
  switch (elem_size) {
    case 1 : return 0;
    case 2 : return 1;
    case 4 : return 2;
    case 8 : return 3;
  }
  ShouldNotReachHere();
  return -1;
}


void LIR_Emitter::indexed_load(RInfo dst, BasicType type, LIR_Opr array, LIR_Opr index, CodeEmitInfo* info) {
  RInfo base = FrameMap::_O7_RInfo;

  int shift = shift_amount(type);
  int offset = arrayOopDesc::base_offset_in_bytes(type);

  if (index->is_constant()) {
    int i = index->as_constant_ptr()->as_jint();
    int elem_size = type2aelembytes[type];
    int array_offset = i * elem_size;
    if (Assembler::is_simm13(array_offset + offset)) {
      base = array->rinfo();;
      offset = array_offset + offset;
    } else if (Assembler::is_simm13(array_offset)) {
      __ add(array->rinfo(), array_offset, base);
    } else {
      __ int2reg(i, base);
      __ shift_left(base, shift, base);
      __ add(LIR_OprFact::rinfo(base), array, LIR_OprFact::rinfo(base));
    }
  } else {
    assert (index->is_register(), "Must be register");
    if (shift > 0) {
      __ shift_left(index->rinfo(), shift, base);
      __ add(LIR_OprFact::rinfo(base), array, LIR_OprFact::rinfo(base));
    } else {
      __ add(index, array, LIR_OprFact::rinfo(base));
    }
  }

  __ load_mem_reg(base, offset, dst, type, info);
}


void LIR_Emitter::indexed_store(BasicType type, LIR_Opr array, LIR_Opr index, LIR_Opr value, RInfo temp_reg, CodeEmitInfo* info) {
  assert(value->is_register() && array->is_register(), "only thing this handles");

  RInfo base = FrameMap::_O7_RInfo;

  int shift = shift_amount(type);
  int offset = arrayOopDesc::base_offset_in_bytes(type);

  if (index->is_constant()) {
    int i = index->as_constant_ptr()->as_jint();
    int elem_size = type2aelembytes[type];
    int array_offset = i * elem_size;
    if (Assembler::is_simm13(array_offset + offset)) {
      base = array->rinfo();;
      offset = array_offset + offset;
    } else if (Assembler::is_simm13(array_offset)) {
      __ add(array->rinfo(), array_offset, base);
    } else {
      __ int2reg(i, base);
      __ shift_left(base, shift, base);
      __ add(LIR_OprFact::rinfo(base), array, LIR_OprFact::rinfo(base));
    }
  } else {
    assert (index->is_register(), "Must be register");
    if (shift > 0) {
      __ shift_left(index->rinfo(), shift, base);
      __ add(LIR_OprFact::rinfo(base), array, LIR_OprFact::rinfo(base));
    } else {
      __ add(index, array, LIR_OprFact::rinfo(base));
    }
  }

  __ store_mem_reg(value->as_rinfo(), base, offset, type, info);

  if (type == T_ARRAY || type == T_OBJECT) {
    __ add(base, offset, FrameMap::_O7_RInfo);
    write_barrier(LIR_OprFact::rinfo(FrameMap::_O7_RInfo, T_ADDRESS), LIR_OprFact::rinfo(FrameMap::_G1_RInfo, T_ADDRESS));
  }
}


// We must use a byte register when storing a byte
void LIR_Emitter::field_store_byte(LIR_Opr object, int offset_in_bytes, LIR_Opr value, RInfo tmp, bool needs_patching, CodeEmitInfo* info) {
  __ store_mem_reg(value->as_rinfo(), object->as_rinfo(), offset_in_bytes, T_BYTE, info, needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none);
}


void LIR_Emitter::math_intrinsic(ciMethod::IntrinsicId id,  RInfo dst, LIR_Opr value, RInfo thread_reg) {
  switch (id) {
    case methodOopDesc::_dsin:
    case methodOopDesc::_dcos: {
      assert(thread_reg.is_valid(), "preserve the thread object for performance reasons");
      //assert(value->is_register() && value->as_rinfo().is_same(FrameMap::_O0_O1_RInfo), "the C call expects its argument in O0/O1");
      assert(dst.is_same(FrameMap::_F0_double_RInfo), "the result will be in f0/f1");
      address runtime_entry = NULL;
      if (id == methodOopDesc::_dsin) {
        runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dsin);
      } else {
        runtime_entry = CAST_FROM_FN_PTR(address, SharedRuntime::dcos);
      }
      __ call_runtime_leaf(runtime_entry, thread_reg, 2, native_arg_no_longs);
      break;
    }
    case methodOopDesc::_dsqrt: {
      assert(!thread_reg.is_valid(), "there is no need for a thread_reg for dsqrt");
      __ sqrt(value->as_rinfo(), dst, thread_reg);
      break;
    }
    default: {
      ShouldNotReachHere();
      break;
    }
  }
}


void LIR_Emitter::put_raw_unsafe(LIR_Opr address, LIR_Opr index, int scale, LIR_Opr value, BasicType value_type) {
  assert(value_type != T_ARRAY && value_type != T_OBJECT, "no put raw of object");
  RInfo addr_reg = long2address(address);  // Address into Lo register
  LIR_Opr base_op = LIR_OprFact::rinfo(addr_reg); // Base
  LIR_Opr index_op = index;

  if (scale != 0) {
    __ shift_left(index_op, scale, index_op);
  }

  if (value_type == T_LONG) {
    // the index may be invalid which means that the address is fully computed in base_op
    // otherwise compute the address into a temporary since we have to add the word offsets
    // to generate the addresses for lo and hi words
    LIR_Opr addr;
    if (index_op->is_illegal()) {
      addr = base_op;
    } else {
      addr = LIR_OprFact::rinfo(FrameMap::_O7_RInfo);
      __ add(base_op, index_op, addr);
    }

    LIR_Address* addr_hi = new LIR_Address(addr, LIR_OprFact::illegalOpr, hi_word_offset_in_bytes());
    LIR_Address* addr_lo = new LIR_Address(addr, LIR_OprFact::illegalOpr, lo_word_offset_in_bytes());
    __ store_mem_reg(value->as_rinfo_lo(), addr_lo, T_INT, NULL, LIR_Op1::patch_none);
    __ store_mem_reg(value->as_rinfo_hi(), addr_hi, T_INT, NULL, LIR_Op1::patch_none);
  } else {
    LIR_Address* addr = new LIR_Address(base_op, index_op, 0);
    if (value_type == T_BOOLEAN || value_type == T_BYTE) {
      __ store_mem_reg(value->as_rinfo(), addr, T_BYTE, NULL, LIR_Op1::patch_none);
    } else {
      __ store_mem_reg(value->as_rinfo(), addr, value_type, NULL, LIR_Op1::patch_none);
    }
  }
}


void LIR_Emitter::get_raw_unsafe(RInfo dst, LIR_Opr address, LIR_Opr index, int scale, BasicType dst_type) {
  assert(dst_type != T_ARRAY && dst_type != T_OBJECT, "no get raw of object");
  RInfo addr_reg = long2address(address);  // Address into Lo register
  LIR_Opr base_op = LIR_OprFact::rinfo(addr_reg); // Base
  LIR_Opr index_op = index;

  if (scale != 0) {
    __ shift_left(index_op, scale, index_op);
  }

  if (dst_type == T_LONG) {
    assert(dst.as_register_lo() != dst.as_register_hi(), "error in regalloc");

    // the index may be invalid which means that the address is fully computed in base_op
    // otherwise compute the address into a temporary since we have to add the word offsets
    // to generate the addresses for lo and hi words
    LIR_Opr addr;
    if (index_op->is_illegal()) {
      addr = base_op;
    } else {
      addr = LIR_OprFact::rinfo(FrameMap::_O7_RInfo);
      __ add(base_op, index_op, addr);
    }

    LIR_Address* addr_hi = new LIR_Address(addr, LIR_OprFact::illegalOpr, hi_word_offset_in_bytes());
    LIR_Address* addr_lo = new LIR_Address(addr, LIR_OprFact::illegalOpr, lo_word_offset_in_bytes());

    if (addr_reg.as_register() == dst.as_register_lo()) {
      // destroy hi register first
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
    } else {
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
    }
  } else {
    LIR_Address* addr = new LIR_Address(base_op, index_op, 0);
    __ load_mem_reg(addr, dst, dst_type, NULL, LIR_Op1::patch_none);
  }
}


void LIR_Emitter::put_Object_unsafe(LIR_Opr src, LIR_Opr offset, LIR_Opr data, BasicType type, bool is_volatile) {
  LIR_Opr base_op = src;
  LIR_Opr index_op = offset;

  if (is_volatile && type == T_LONG) {
    __ volatile_store_unsafe_reg(data->as_rinfo(), src->as_rinfo(), offset->as_rinfo(), type, NULL, LIR_Op1::patch_none);
  }
  else if (type == T_LONG) {
    __ store_mem_reg(data->as_rinfo_lo(), new LIR_Address(base_op, index_op, lo_word_offset_in_bytes()), T_INT, NULL, LIR_Op1::patch_none);
    __ store_mem_reg(data->as_rinfo_hi(), new LIR_Address(base_op, index_op, hi_word_offset_in_bytes()), T_INT, NULL, LIR_Op1::patch_none);
  } else if (type == T_BOOLEAN || type == T_BYTE) {
    __ store_mem_reg(data->as_rinfo(), new LIR_Address(base_op, index_op, 0), T_BYTE, NULL, LIR_Op1::patch_none);
  } else {
    __ store_mem_reg(data->as_rinfo(), new LIR_Address(base_op, index_op, 0), type, NULL, LIR_Op1::patch_none);
    if (type == T_ARRAY || type == T_OBJECT) {
      assert(base_op->is_register() && index_op->is_register(), "both registers");
      __ add(base_op, index_op, LIR_OprFact::rinfo(FrameMap::_O7_RInfo));
      write_barrier(LIR_OprFact::rinfo(FrameMap::_O7_RInfo), LIR_OprFact::rinfo(FrameMap::_G3_RInfo));
    }
  }
}


void LIR_Emitter::get_Object_unsafe(RInfo dst, LIR_Opr src, LIR_Opr offset, BasicType type, bool is_volatile) {
  LIR_Opr base_op = src;
  LIR_Opr index_op = offset;

  if (is_volatile && type == T_LONG) {
    __ volatile_load_unsafe_reg(src->as_rinfo(), offset->as_rinfo(), dst, type, NULL, LIR_Op1::patch_none);
  }
  else if (type == T_LONG) {
    assert(dst.as_register_lo() != dst.as_register_hi(), "error in regalloc");
    LIR_Opr addr = LIR_OprFact::rinfo(FrameMap::_O7_RInfo);
    LIR_Address* addr_hi = new LIR_Address(addr, LIR_OprFact::illegalOpr, hi_word_offset_in_bytes());
    LIR_Address* addr_lo = new LIR_Address(addr, LIR_OprFact::illegalOpr, lo_word_offset_in_bytes());

    __ add(base_op, index_op, addr);
    if (src->as_register() == dst.as_register_lo()) {
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
    } else {
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
    }
  } else {
    LIR_Address* addr = new LIR_Address(base_op, index_op, 0);
    __ load_mem_reg(addr, dst, type, NULL, LIR_Op1::patch_none);
  }
}


void LIR_Emitter::trace_block_entry(BlockBegin* block, address func) {
  __ int2reg(block->block_id(), FrameMap::_O0_RInfo);
  __ call_runtime_leaf(func, FrameMap::_L7_RInfo, 2, native_arg_no_longs);
}


void LIR_Emitter::cmp_mem_int(LIR_OpBranch::LIR_Condition condition, RInfo base, int disp, int c, CodeEmitInfo* info) {
  __ load_mem_reg(base, disp, FrameMap::_O7_RInfo, T_INT, info);
  __ cmp(condition, LIR_OprFact::rinfo(FrameMap::_O7_RInfo), c);
}


void LIR_Emitter::cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, RInfo base, int disp, BasicType type, CodeEmitInfo* info) {
  __ load_mem_reg(base, disp, FrameMap::_O7_RInfo, type, info);
  __ cmp(condition, LIR_OprFact::rinfo(reg), LIR_OprFact::rinfo(FrameMap::_O7_RInfo));
}


void LIR_Emitter::cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, RInfo base, RInfo disp, BasicType type, CodeEmitInfo* info) {
  __ load_mem_reg(new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::rinfo(disp), 0), FrameMap::_O7_RInfo, type, info);
  __ cmp(condition, LIR_OprFact::rinfo(reg), LIR_OprFact::rinfo(FrameMap::_O7_RInfo));
}


bool LIR_Emitter::strength_reduce_multiply(LIR_Opr left, int c, LIR_Opr result, LIR_Opr tmp) {
  assert(left != result, "should be different registers");
  if (is_power_of_2(c + 1)) {
    __ shift_left(left, log2_intptr(c + 1), result);
    __ sub(result, left, result);
    return true;
  } else if (is_power_of_2(c - 1)) {
    __ shift_left(left, log2_intptr(c - 1), result);
    __ add(result, left, result);
    return true;
  }
  return false;
}


void LIR_Emitter::store_stack_parameter (LIR_Opr item, int offset_from_sp_in_words) {
  BasicType t = item->type();
  int offset_from_sp_in_bytes = offset_from_sp_in_words * BytesPerWord;
  RInfo reg = item->as_rinfo();
  if (t == T_DOUBLE) {
    __ store_mem_reg(reg.as_rinfo_lo(), FrameMap::_SP_RInfo, offset_from_sp_in_bytes + lo_word_offset_in_bytes(), T_FLOAT, NULL);
    __ store_mem_reg(reg.as_rinfo_hi(), FrameMap::_SP_RInfo, offset_from_sp_in_bytes + hi_word_offset_in_bytes(), T_FLOAT, NULL);
  } else {
    __ store_mem_reg(reg, FrameMap::_SP_RInfo, offset_from_sp_in_bytes, t, NULL);
  }
}


void LIR_Emitter::write_barrier(LIR_Opr value, LIR_Opr tmp) {
#ifdef _LP64
  Unimplemented();
#else
  RInfo obj = value->rinfo();
  BarrierSet* bs = Universe::heap()->barrier_set();
  assert(bs->kind() == BarrierSet::CardTableModRef, "Wrong barrier set kind");
  CardTableModRefBS* ct = (CardTableModRefBS*)bs;
  assert(sizeof(*ct->byte_map_base) == sizeof(jbyte), "adjust this code");

  __ unsigned_shift_right(obj, CardTableModRefBS::card_shift, obj);
  __ int2reg((int)ct->byte_map_base, tmp->rinfo());
  __ store(LIR_OprFact::intConst(0), new LIR_Address(value, tmp), T_BYTE, NULL);
#endif
}


void LIR_Emitter::call_convert_op(Bytecodes::Code code, RInfo tmp) {
  address entry = NULL;
  int in_size = -1;
  int args = native_arg_no_longs;
  switch (code) {
    case Bytecodes::_l2f:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::l2f);
      in_size = 2;
      args = native_arg0_is_long;
      break;
    case Bytecodes::_l2d:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::l2d);
      in_size = 2;
      args = native_arg0_is_long;
      break;
    case Bytecodes::_f2l:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::f2l);
      in_size = 1;
      args = native_return_is_long;
      break;
    case Bytecodes::_d2l:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::d2l);
      in_size = 2;
      args = native_return_is_long;
      break;
    case Bytecodes::_d2i:
      entry = CAST_FROM_FN_PTR(address, SharedRuntime::d2i);
      in_size = 2;
      break;
    default:
      ShouldNotReachHere();
  }
  __ call_runtime_leaf(entry, tmp, in_size, args);
}


void LIR_Emitter::call_slow_subtype_check(RInfo sub, RInfo super, RInfo result) {
  assert(sub.is_same(FrameMap::_G3_RInfo) && super.is_same(FrameMap::_G1_RInfo), "incorrect call setup");
  __ call_runtime_leaf(CAST_FROM_FN_PTR(address, Runtime1::entry_for(Runtime1::slow_subtype_check_id)), norinfo, 2, 0);
  __ reg2reg(FrameMap::_G3_RInfo, result, T_OBJECT);
}
