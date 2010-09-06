#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_LIREmitter_i486.cpp	1.58 03/12/23 16:36:07 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_LIREmitter_i486.cpp.incl"


#define __ _lir->


int LIR_Emitter::frame_size() { return no_frame_size; }

void LIR_Emitter::monitorenter_at_entry(RInfo receiver, CodeEmitInfo* info) {
  const RInfo obj_info = receiver; // object to lock
  if (method()->is_static()) {
    __ oop2reg(method()->holder()->java_mirror()->encoding(), obj_info);
  } else {
    // receiver is already loaded
  }
  RInfo lock_info = FrameMap::_esiRInfo;
  RInfo sync_header = FrameMap::_eaxRInfo;
  CodeEmitInfo* info_for_exception = NULL;  // synchronized method entry cannot cause a NullPointerException
  monitor_enter(obj_info, lock_info, sync_header, norinfo, 0, info_for_exception, info);
}


void LIR_Emitter::return_op_prolog (int monitor_no) {
  assert(method()->is_synchronized(), "must be synchronized");
  // do not use eax/edx
  RInfo obj_info  = FrameMap::_ecxRInfo;
  RInfo lock_info = FrameMap::_esiRInfo;
  assert(monitor_no >= 0, "monitor slot must exist");
  RInfo SYNC_header = FrameMap::_eaxRInfo;   // synchronization header
  monitor_exit(obj_info, lock_info, SYNC_header, monitor_no);
}


int LIR_Emitter::hi_word_offset_in_bytes() const {
  return BytesPerWord;
}


int LIR_Emitter::lo_word_offset_in_bytes() const {
  return 0; // little endian
}

void LIR_Emitter::new_type_array  (RInfo dst, BasicType elem_type, LIR_Opr lengthItem, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo scratch4, RInfo klass_reg, CodeEmitInfo* info) {
  RInfo lengthR = scratch1;
  CodeStub* slow_path = new NewTypeArrayStub(klass_reg, lengthR, dst, info);
  if (!lengthItem->as_rinfo().is_same(lengthR)) {
    __ reg2reg(lengthItem->as_rinfo(), lengthR, lengthItem->type());
  }
  ciTypeArrayKlass* klass = ciTypeArrayKlass::make(elem_type);
  assert (klass != NULL, "type array klasses exist alread");
  if (UseFastNewTypeArray && !UseSlowPath) {
    __ oop2reg(klass->encoding(), klass_reg);
    __ allocate_array(dst, lengthR, scratch2, scratch3, norinfo, norinfo, elem_type, klass_reg, slow_path);
  } else {
    __ branch(LIR_OpBranch::always, slow_path);
    __ branch_destination(slow_path->continuation());
  }
}


void LIR_Emitter::new_object_array(RInfo dst, ciKlass* elem_klass, LIR_Opr lengthItem, RInfo scratch1, RInfo scratch2, RInfo scratch3, RInfo scratch4, RInfo klass_reg, CodeEmitInfo* info, CodeEmitInfo* patching_info) {
  RInfo lengthR = scratch1;
  CodeStub* slow_path = new NewObjectArrayStub(klass_reg, lengthR, dst, info);
  if (!lengthItem->as_rinfo().is_same(lengthR)) {
    __ reg2reg(lengthItem->as_rinfo(), lengthR, lengthItem->type());
  }
  ciObject* obj = (ciObject*) ciObjArrayKlass::make(elem_klass);
  if (obj == ciEnv::unloaded_ciobjarrayklass()) {
      set_bailout("encountered unloaded_ciobjarrayklass due to out of memory error");
      return;
  }
  jobject2reg_with_patching(klass_reg, obj, patching_info);
  if (UseFastNewObjectArray) {
    // allocate space for array
    assert(oopSize == 4, "change scale factor below");
    __ allocate_array(dst, lengthR, scratch2, scratch4, norinfo, norinfo, T_OBJECT, klass_reg, slow_path);
  } else {
    __ branch(LIR_OpBranch::always, slow_path);
    __ branch_destination(slow_path->continuation());
  }
}


LIR_Address* LIR_Emitter::array_address(LIR_Opr array, LIR_Opr index, int offset_in_bytes, BasicType type) {
  offset_in_bytes += arrayOopDesc::base_offset_in_bytes(type);

  if (index->is_constant()) {
    int elem_size = type2aelembytes[type];
    return new LIR_Address(LIR_OprFact::rinfo(array->as_rinfo(), array->type()),
                           LIR_OprFact::illegalOpr,
                           offset_in_bytes + opr2int(index) * elem_size);
  } else {
    return new LIR_Address(LIR_OprFact::rinfo(array->as_rinfo(), array->type()),
                           LIR_OprFact::rinfo(index->as_rinfo(), index->type()),
                           LIR_Address::scale(type),
                           offset_in_bytes);
  }
}





void LIR_Emitter::indexed_load(RInfo dst, BasicType dst_type, LIR_Opr array, LIR_Opr index, CodeEmitInfo* info) {
  if (dst_type == T_LONG) {
    LIR_Address* addrLo = array_address(array, index, lo_word_offset_in_bytes(), dst_type);
    LIR_Address* addrHi = array_address(array, index, hi_word_offset_in_bytes(), dst_type);
    Register lo_reg = dst.as_register_lo();
    Register hi_reg = dst.as_register_hi();
    Register arr_reg = array->as_rinfo().as_register();
    Register index_reg = index->is_constant() ? noreg : index->as_rinfo().as_register();
    if (hi_reg == arr_reg || hi_reg == index_reg) {
      // load first into lo_reg, and then destroy hi_reg
      assert(lo_reg != arr_reg && lo_reg != index_reg, "cannot emit correct code");
      __ move(addrLo, LIR_OprFact::rinfo(dst.as_rinfo_lo(), T_INT), info);
      __ move(addrHi, LIR_OprFact::rinfo(dst.as_rinfo_hi(), T_INT), info);
    } else {
      __ move(addrHi, LIR_OprFact::rinfo(dst.as_rinfo_hi(), T_INT), info);
      __ move(addrLo, LIR_OprFact::rinfo(dst.as_rinfo_lo(), T_INT), info);
    }
  } else {
    __ move(array_address(array, index, 0, dst_type), LIR_OprFact::rinfo(dst, dst_type), info);
  }
}


void LIR_Emitter::indexed_store(BasicType val_type, LIR_Opr array, LIR_Opr index, LIR_Opr value, RInfo temp_reg, CodeEmitInfo* info) {
//   const2array(src->as_constant_ptr(), dest->as_address_ptr(), type, info);
//   reg2array(src->rinfo(), dest->as_address_ptr(), type, info);

  LIR_Address* addr = array_address(array, index, 0, val_type);
  LIR_Address* addrLo = array_address(array, index, lo_word_offset_in_bytes(), val_type);
  LIR_Address* addrHi = array_address(array, index, hi_word_offset_in_bytes(), val_type);
  if (value->is_constant()) {
    if (val_type == T_LONG || val_type == T_DOUBLE) {
      // double word
      // must pass the right type in order to compute the array offset correctly
      __ store_array(opr2intLo(value), addrLo,  val_type, info);
      __ store_array(opr2intHi(value), addrHi,  val_type, info);
    } else if (val_type == T_OBJECT || val_type == T_ARRAY) {
      __ store_array(opr2jobject(value), addr, val_type, info);
      write_barrier(LIR_OprFact::address(addr), array); // array is available as a temporary

    } else {
      __ store_array(opr2int(value), addr, val_type, info);
    }
  } else if (value->is_register()) {
    if (val_type == T_LONG) {
      __ store_array(value->as_rinfo_lo(), addrLo, val_type, info);
      __ store_array(value->as_rinfo_hi(), addrHi, val_type, info);
    } else {
      __ store_array(value->as_rinfo(), addr, val_type, info);
    }
    if (val_type == T_OBJECT || val_type == T_ARRAY) {
      write_barrier(LIR_OprFact::address(addr), array); // array is available as a temporary
    }
  } else {
    Unimplemented();
  }
}


// We must use a byte register when storing a byte
void LIR_Emitter::field_store_byte(LIR_Opr object, int offset_in_bytes, LIR_Opr value, RInfo tmp, bool needs_patching, CodeEmitInfo* info) {
  assert(value->as_rinfo().as_register()->has_byte_register() || VM_Version::is_P6(), "check that it's a byte");
  __ store_mem_reg(value->as_rinfo(), object->as_rinfo(), offset_in_bytes, T_BYTE, info, needs_patching ? LIR_Op1::patch_normal : LIR_Op1::patch_none);
}

void LIR_Emitter::pop_item(LIR_Opr item) {
  if (item->is_register() && item->value_type()->tag() == intTag) {
    __ pop_reg(item->as_rinfo());
  } else if (item->is_float_kind()) {
    assert(item->is_register(), "Only used for gross store elimination bug fix");
    __ pop_reg(item->as_rinfo());
  } else {
    ShouldNotReachHere();
  }
}



void LIR_Emitter::math_intrinsic(ciMethod::IntrinsicId id,  RInfo dst, LIR_Opr value, RInfo thread_cache) {
  assert(value->is_destroyed(), "check");
  assert(value->value_type()->tag() == doubleTag, "wrong type");

  switch(id) {
    case methodOopDesc::_dsqrt: __ sqrt(value->as_rinfo(), dst, thread_cache); break;
    case methodOopDesc::_dsin : __ sin(value->as_rinfo(), dst, thread_cache);  break;
    case methodOopDesc::_dcos : __ cos(value->as_rinfo(), dst, thread_cache);  break;
    default                   : ShouldNotReachHere();
  }
}


void LIR_Emitter::put_raw_unsafe(LIR_Opr address, LIR_Opr index, int scale, LIR_Opr value, BasicType value_type) {
  RInfo addr_reg = long2address(address);  // Address into Lo register
  LIR_Opr base_op = LIR_OprFact::rinfo(addr_reg); // Base
  LIR_Opr index_op = index;

  if (value_type == T_LONG) {
    LIR_Address* addr_hi = new LIR_Address(base_op, index_op, LIR_Address::Scale(scale), hi_word_offset_in_bytes());
    LIR_Address* addr_lo = new LIR_Address(base_op, index_op, LIR_Address::Scale(scale), lo_word_offset_in_bytes());
    __ store_mem_reg(value->as_rinfo_lo(), addr_lo, T_INT, NULL, LIR_Op1::patch_none);
    __ store_mem_reg(value->as_rinfo_hi(), addr_hi, T_INT, NULL, LIR_Op1::patch_none);
  } else {
    LIR_Address* addr = new LIR_Address(base_op, index_op, LIR_Address::Scale(scale), 0);
    if (value_type == T_BOOLEAN || value_type == T_BYTE) {
      __ store_mem_reg(value->as_rinfo(), addr, T_BYTE, NULL, LIR_Op1::patch_none);
    } else {
      __ store_mem_reg(value->as_rinfo(), addr, value_type, NULL, LIR_Op1::patch_none);
    }
  }
}


void LIR_Emitter::get_raw_unsafe(RInfo dst, LIR_Opr address, LIR_Opr index, int scale, BasicType dst_type) {
  RInfo addr_reg = long2address(address);  // Address into Lo register
  LIR_Opr base_op = LIR_OprFact::rinfo(addr_reg); // Base
  LIR_Opr index_op = index;

  if (dst_type == T_LONG) {
    assert(dst.as_register_lo() != dst.as_register_hi(), "error in regalloc");
    LIR_Address* addr_hi = new LIR_Address(base_op, index_op, LIR_Address::Scale(scale), hi_word_offset_in_bytes());
    LIR_Address* addr_lo = new LIR_Address(base_op, index_op, LIR_Address::Scale(scale), lo_word_offset_in_bytes());
    if (addr_reg.as_register() == dst.as_register_lo()) {
      // destroy hi register first
      assert(!index_op->is_register() || !index_op->is_valid() || index_op->as_register() != dst.as_register_hi(), "not enough registers");
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
    } else {
      assert(!index_op->is_register() || !index_op->is_valid() || index_op->as_register() != dst.as_register_lo(), "not enough registers");
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
    }
  } else {
    LIR_Address* addr = new LIR_Address(base_op, index_op, LIR_Address::Scale(scale), 0);
    __ load_mem_reg(addr, dst, dst_type, NULL, LIR_Op1::patch_none);
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
    LIR_Address* addr_hi = new LIR_Address(base_op, index_op, hi_word_offset_in_bytes());
    LIR_Address* addr_lo = new LIR_Address(base_op, index_op, lo_word_offset_in_bytes());

    if (src->as_rinfo().as_register() == dst.as_register_lo()) {
      assert(!index_op->is_register() || !index_op->is_valid() || index_op->as_register() != dst.as_register_hi(), "not enough registers");
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
    } else {
      assert(!index_op->is_register() || !index_op->is_valid() || index_op->as_register() != dst.as_register_lo(), "not enough registers");
      __ load_mem_reg(addr_lo, dst.as_rinfo_lo(), T_INT, NULL, LIR_Op1::patch_none);
      __ load_mem_reg(addr_hi, dst.as_rinfo_hi(), T_INT, NULL, LIR_Op1::patch_none);
    }
  } else {
    LIR_Address* addr = new LIR_Address(base_op, index_op, 0);
    __ load_mem_reg(addr, dst, type, NULL, LIR_Op1::patch_none);
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
      assert(base_op->is_register(), "must be register");
      write_barrier(LIR_OprFact::address(new LIR_Address(base_op, index_op, 0)), base_op);
    }
  }
}


void LIR_Emitter::trace_block_entry(BlockBegin* block, address func) {
  Unimplemented();
}




void LIR_Emitter::cmp_mem_int(LIR_OpBranch::LIR_Condition condition, RInfo base, int disp, int c, CodeEmitInfo* info) {
  __ cmp_mem_int(condition, base, disp, c, info);
}


void LIR_Emitter::cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, RInfo base, int disp, BasicType type, CodeEmitInfo* info) {
  __ cmp_reg_mem(condition, reg, new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::illegalOpr, disp), type, info);
}


void LIR_Emitter::cmp_reg_mem(LIR_OpBranch::LIR_Condition condition, RInfo reg, RInfo base, RInfo disp, BasicType type, CodeEmitInfo* info) {
  __ cmp_reg_mem(condition, reg, new LIR_Address(LIR_OprFact::rinfo(base), LIR_OprFact::rinfo(disp), 0), type, info);
}


bool LIR_Emitter::strength_reduce_multiply(LIR_Opr left, int c, LIR_Opr result, LIR_Opr tmp) {
  if (tmp->is_valid()) {
    if (is_power_of_2(c + 1)) {
      __ reg2reg(left->rinfo(), tmp->rinfo(), T_INT);
      __ shift_left(tmp, log2_intptr(c + 1), tmp);
      __ sub(tmp, left, result);
      return true;
    } else if (is_power_of_2(c - 1)) {
      __ reg2reg(left->rinfo(), tmp->rinfo(), T_INT);
      __ shift_left(tmp, log2_intptr(c - 1), tmp);
      __ add(tmp, left, result);
      return true;
    }
  }
  return false;
}


void LIR_Emitter::store_stack_parameter (LIR_Opr item, int offset_from_sp_in_words) {
  BasicType type = item->type();
  int offset_from_sp_in_bytes = offset_from_sp_in_words * BytesPerWord;
  if (type == T_DOUBLE || type == T_LONG) {
    offset_from_sp_in_bytes -= BytesPerWord;
  }
  __ store(item, new LIR_Address(LIR_OprFact::rinfo(FrameMap::_espRInfo), LIR_OprFact::illegalOpr, offset_from_sp_in_bytes), type, NULL);
}


void LIR_Emitter::call_slow_subtype_check(RInfo sub, RInfo super, RInfo result) {
  __ push_reg(sub);
  __ push_reg(super);
  __ call_runtime_leaf(CAST_FROM_FN_PTR(address, Runtime1::entry_for(Runtime1::slow_subtype_check_id)), norinfo, 1, 0);
  __ pop_reg(result);
}


void LIR_Emitter::write_barrier(LIR_Opr value, LIR_Opr tmp) {
  assert(value->is_address() || value->is_register(), "wrong parameter");
  assert(tmp->is_illegal() || tmp->is_register(), "wrong parameter");

  RInfo obj;
  if (value->is_address()) {
    __ leal(value, tmp);
    obj = tmp->rinfo();
  } else {
    obj = value->rinfo();
  }

  BarrierSet* bs = Universe::heap()->barrier_set();
  assert(bs->kind() == BarrierSet::CardTableModRef, "Wrong barrier set kind");
  CardTableModRefBS* ct = (CardTableModRefBS*)bs;
  assert(sizeof(*ct->byte_map_base) == sizeof(jbyte), "adjust this code");

  __ unsigned_shift_right(obj, CardTableModRefBS::card_shift, obj);
  __ store_mem_int(0, obj, (int)ct->byte_map_base, T_BYTE, NULL);
}

#undef __asm_

