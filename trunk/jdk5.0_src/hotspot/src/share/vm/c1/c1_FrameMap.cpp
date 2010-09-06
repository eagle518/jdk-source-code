#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_FrameMap.cpp	1.26 04/03/31 18:13:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_c1_FrameMap.cpp.incl"



//-----------------------------------------------------

// Helper routine to simplify transition from old
// jvmci interface to new CI interface. Eventually
// we should adjust the code to use ciSignature*
// instead (gri 1/28/2000).

BasicTypeList* FrameMap::signature_type_array_for(const ciMethod* method) {
  ciSignature* sig = method->signature();
  BasicTypeList* sta = new BasicTypeList(method->arg_size());
  // add receiver, if any
  if (!method->is_static()) sta->append(T_OBJECT);
  // add remaining arguments
  for (int i = 0; i < sig->count(); i++) {
    ciType* type = sig->type_at(i);
    /* add basic type once */ sta->append(type->basic_type());
    if (type->is_two_word())  sta->append(type->basic_type());
  }
  // done
  assert(sta->length() == sta->size(), "should have used all slots");
  return sta;
}


//--------------------------------------------------------
//               FrameMap
//--------------------------------------------------------

bool      FrameMap::_init_done = false;
Register  FrameMap::_cpu_regs [FrameMap::nof_cpu_regs];


Register FrameMap::cpu_rnr2reg (int rnr) {
  assert(_init_done, "tables not initialized");
  debug_only(cpu_range_check(rnr);)
  return _cpu_regs[rnr];
}


// This is slow, called rarely
int FrameMap::cpu_reg2rnr (Register reg) {
  assert(_init_done, "tables not initialized");
  for (int i = 0; i < nof_cpu_regs; i++) {
    if (cpu_rnr2reg(i) == reg) return i;
  }
  if (reg == (Register)-1)
    return -1;
  ShouldNotReachHere();
  return -1;
}


LIR_Opr FrameMap::caller_save_cpu_reg_at(int i) {
  if (i >= 0 && i < nof_caller_save_cpu_regs) {
    return _caller_save_cpu_regs[i];
  }
  return LIR_OprFact::illegalOpr;
}


CallingConvention* FrameMap::calling_convention (const ciMethod* method, intArray* reg_args /* = NULL */) {
  const BasicTypeList* signature = signature_type_array_for(method);
  return calling_convention (method->is_static(), *signature, reg_args);
}


// ------------ extra spill slots ---------------

void FrameMap::add_spill_slots(int nof_slots) {
  assert(nof_slots >= 0, "must be positive");
  _size_spills += nof_slots;
  _size_scratch_spills += nof_slots;
}


bool FrameMap::is_spill_pos(LIR_Opr opr) const {
  if (opr->is_single_stack()) {
    return (opr->single_stack_ix() >= num_local_names());
  } else {
    assert(opr->is_double_stack(), "is_spill_pos must receive LIR_Opr on stack");
    return (opr->double_stack_ix() >= num_local_names());
  }
}

// ------------ locals ---------------

#ifdef ASSERT
void FrameMap::set_is_java_method() {
  _is_java_method = true;
}
#endif

void FrameMap::set_local_name_to_offset_map(WordSizeList* list) {
  _local_name_to_offset_map = list;
}

int FrameMap::num_local_names() const {
  assert(_local_name_to_offset_map != NULL || !_is_java_method, "Must set name-to-offset map for Java methods");

  if (_local_name_to_offset_map != NULL)
    return _local_name_to_offset_map->length();

  // Otherwise, this is a library intrinsic or compiled native method;
  // fake the result so argument handling works properly
  return size_locals();
}
