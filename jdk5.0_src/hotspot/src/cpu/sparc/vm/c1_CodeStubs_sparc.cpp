#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CodeStubs_sparc.cpp	1.67 04/04/14 17:27:29 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_c1_CodeStubs_sparc.cpp.incl"


#define __ ce->masm()->

void StaticCallStub::emit_code(LIR_Assembler* ce) {
  __ relocate(static_stub_Relocation::spec(_call_pc));
  __ set_oop(NULL, G5);
  // must be set to -1 at code generation time
  Address a(G3, (address)-1);
  __ jump_to(a, 0);
  __ delayed()->nop();
}


RangeCheckStub::RangeCheckStub(CodeEmitInfo* info, RInfo index_rinfo, int index_value, bool throw_index_out_of_bounds_exception)
: _throw_index_out_of_bounds_exception(throw_index_out_of_bounds_exception)
, _index_rinfo(index_rinfo)
, _index_value(index_value)
{ 
  _info = info == NULL ? NULL : new CodeEmitInfo(info);
  _info->check_is_exception_info();
}


void RangeCheckStub::emit_code(LIR_Assembler* ce) { 
  __ bind(_entry);
  if (_index_rinfo.is_valid()) {
    __ mov(_index_rinfo.as_register(), G4);
  } else {
    __ set(_index_value, G4);
  }
  if (_throw_index_out_of_bounds_exception) {
    __ call(Runtime1::entry_for(Runtime1::throw_index_exception_id), relocInfo::runtime_call_type);
  } else {
    __ call(Runtime1::entry_for(Runtime1::range_check_failed_id), relocInfo::runtime_call_type);
  }
  __ delayed()->nop();  
  ce->add_call_info_here(_info);
}


void DivByZeroStub::emit_code(LIR_Assembler* ce) {
  if (_offset != -1) {
    ce->compilation()->null_check_table()->append(_offset, __ offset());
  }
  __ bind(_entry);
  __ call(Runtime1::entry_for(Runtime1::throw_div0_exception_id), relocInfo::runtime_call_type);
  __ delayed()->nop();
  ce->add_call_info_here(_info);
}


void ImplicitNullCheckStub::emit_code(LIR_Assembler* ce) {
  ce->compilation()->null_check_table()->append(_offset, __ offset());
  __ bind(_entry);
  __ call(Runtime1::entry_for(Runtime1::throw_null_pointer_exception_id),
          relocInfo::runtime_call_type);
  __ delayed()->nop();
  ce->add_call_info_here(_info);
}


// Implementation of SimpleExceptionStub

// Note: %g1 and %g3 are already in use
void SimpleExceptionStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  __ call(Runtime1::entry_for(_stub), relocInfo::runtime_call_type);
  if (_obj.is_valid()) {
    __ delayed()->mov(_obj.as_register(), G4); // _obj contains the optional argument to the stub
  } else {
    __ delayed()->mov(G0, G4);
  }
  ce->add_call_info_here(_info_for_exception);
}


// Implementation of ArrayStoreExceptionStub

ArrayStoreExceptionStub::ArrayStoreExceptionStub(CodeEmitInfo* info_for_exception):
  _info_for_exception(info_for_exception) {
  _info_for_exception->check_is_exception_info();
}


void ArrayStoreExceptionStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  __ call(Runtime1::entry_for(Runtime1::throw_array_store_exception_id), relocInfo::runtime_call_type);
  __ delayed()->nop();
  ce->add_call_info_here(_info_for_exception);
}




// Implementation of NewInstanceStub

NewInstanceStub::NewInstanceStub(RInfo klass_reg, LIR_Opr result, ciInstanceKlass* klass, CodeEmitInfo* info, Runtime1::StubID stub_id) {
  _result = result;
  _klass = klass;
  _klass_reg = klass_reg;
  _info = new CodeEmitInfo(info);
  assert(stub_id == Runtime1::new_instance_id                 ||
         stub_id == Runtime1::fast_new_instance_id            ||
         stub_id == Runtime1::fast_new_instance_init_check_id,
         "need new_instance id");
  _stub_id   = stub_id;
}


void NewInstanceStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  __ call(Runtime1::entry_for(_stub_id), relocInfo::runtime_call_type);
  __ delayed()->mov_or_nop(_klass_reg.as_register(), G5);
  ce->add_call_info_here(_info);
  __ br(Assembler::always, false, Assembler::pt, _continuation);
  __ delayed()->mov_or_nop(O0, _result->as_register());
}


// Implementation of NewTypeArrayStub
NewTypeArrayStub::NewTypeArrayStub(RInfo klass_reg, RInfo length, RInfo result, CodeEmitInfo* info) {
  _klass_reg = klass_reg;
  _length = length;
  _result = result;
  _info = new CodeEmitInfo(info);
}


void NewTypeArrayStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  __ mov(_length.as_register(), G4);
  __ call(Runtime1::entry_for(Runtime1::new_type_array_id), relocInfo::runtime_call_type);
  __ delayed()->mov_or_nop(_klass_reg.as_register(), G5);
  ce->add_call_info_here(_info);
  __ br(Assembler::always, false, Assembler::pt, _continuation);
  __ delayed()->mov_or_nop(O0, _result.as_register());
}


// Implementation of NewObjectArrayStub

NewObjectArrayStub::NewObjectArrayStub(RInfo klass_reg, RInfo length, RInfo result, CodeEmitInfo* info) {
  _klass_reg = klass_reg;
  _length = length;
  _result = result;
  _info = new CodeEmitInfo(info);
}


void NewObjectArrayStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  __ mov(_length.as_register(), G4);
  __ call(Runtime1::entry_for(Runtime1::new_object_array_id), relocInfo::runtime_call_type);
  __ delayed()->mov_or_nop(_klass_reg.as_register(), G5);
  ce->add_call_info_here(_info);
  __ br(Assembler::always, false, Assembler::pt, _continuation);
  __ delayed()->mov_or_nop(O0, _result.as_register());
}


// Implementation of MonitorAccessStubs

MonitorEnterStub::MonitorEnterStub(RInfo obj_reg, RInfo lock_reg, CodeEmitInfo* info)
           : MonitorAccessStub(obj_reg, lock_reg) {
  _info = new CodeEmitInfo(info);
}


void MonitorEnterStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  NEEDS_CLEANUP // use G register instead of O's
  __ mov(_obj_reg.as_register(), O0);
  if ((ce->compilation()->jvmpi_event_method_entry_enabled() ||
       ce->compilation()->jvmpi_event_method_entry2_enabled()) &&
      _info->bci() == SynchronizationEntryBCI) {
    // This stub will perform jvmpi notification if its caller gets deoptimized after the monitorenter.
    __ call(Runtime1::entry_for(Runtime1::monitorenter_with_jvmpi_id), relocInfo::runtime_call_type);
  } else {
    __ call(Runtime1::entry_for(Runtime1::monitorenter_id), relocInfo::runtime_call_type);
  }
  __ delayed()->mov_or_nop(_lock_reg.as_register(), O1);
  ce->add_call_info_here(_info);
  __ br(Assembler::always, true, Assembler::pt, _continuation);
  __ delayed()->nop();
}


void MonitorExitStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  if (_compute_lock) {
    ce->monitor_address(_monitor_ix, _lock_reg);
  }
  __ call(Runtime1::entry_for(Runtime1::monitorexit_id), relocInfo::runtime_call_type);
  __ delayed()->mov_or_nop(_lock_reg.as_register(), G4);
  __ br(Assembler::always, true, Assembler::pt, _continuation);
  __ delayed()->nop();
}

// Implementation of patching: 
// - Copy the code at given offset to an inlined buffer (first the bytes, then the number of bytes)
// - Replace original code with a call to the stub
// At Runtime:
// - call to stub, jump to runtime 
// - in runtime: preserve all registers (especially objects, i.e., source and destination object)
// - in runtime: after initializing class, restore original code, reexecute instruction 

int PatchingStub::_patch_info_offset = -NativeGeneralJump::instruction_size;

void PatchingStub::align_patch_site(MacroAssembler* ) {
  // patch sites on sparc are always properly aligned.
}

void PatchingStub::emit_code(LIR_Assembler* ce) {
  // copy original code here
  assert(NativeCall::instruction_size <= _bytes_to_copy && _bytes_to_copy <= 0xFF,
	 "not enough room for call");
  assert((_bytes_to_copy & 0x3) == 0, "must copy a multiple of four bytes");

  Label call_patch;

  // static field accesses have special semantics while the class
  // initializer is being run so we emit a test which can be used to
  // check that this code is being executed by the initializing
  // thread.
  address being_initialized_entry = __ pc();
  if (_id == access_field_id) {
    assert(_obj != noreg, "must be a valid register");
    __ ld_ptr(_obj, instanceKlass::init_thread_offset_in_bytes() + sizeof(klassOopDesc), G3);
    __ cmp(G2_thread, G3);
    __ br(Assembler::notEqual, false, Assembler::pn, call_patch);
    __ delayed()->nop();
  }

  // make a copy the code which is going to be patched.
  for (int i = 0; i < _bytes_to_copy; i++) {
    address ptr = (address)(_pc_start + i);
    int a_byte = (*ptr) & 0xFF;
    __ a_byte (a_byte);
  }

  int bytes_to_skip = 4;
  if (_id == access_field_id) {
    // access_field patches may execute the patched code before it's
    // copied back into place so we need to jump back into the main
    // code of the nmethod to continue execution.
    bytes_to_skip += 8;
    __ br(Assembler::always, false, Assembler::pt, _patch_site_continuation);
    __ delayed()->nop();
  }
  // emit the offsets needed to find the code to patch
  int being_initialized_entry_offset = __ pc() - being_initialized_entry + 4;
  // We need to emit a full word, so emit an extra empty byte
  __ a_byte(0);
  __ a_byte(being_initialized_entry_offset);
  __ a_byte(bytes_to_skip);
  __ a_byte(_bytes_to_copy);
  address entry = __ pc();
  NativeGeneralJump::insert_unconditional((address)_pc_start, entry);
  address target = NULL;
  switch (_id) {
    case access_field_id:  target = Runtime1::entry_for(Runtime1::init_check_patching_id); break;
    case load_klass_id:    target = Runtime1::entry_for(Runtime1::load_klass_patching_id); break;
    default: ShouldNotReachHere();
  }

  __ bind(call_patch);
  __ call(target, relocInfo::runtime_call_type);
  __ delayed()->nop();
  ce->add_call_info_here(_info);
  __ br(Assembler::always, false, Assembler::pt, _patch_site_entry);
  __ delayed()->nop();
  if (_id == load_klass_id) {
    CodeBuffer* cb = __ code();
    address pc = (address)_pc_start;
    RelocIterator iter(cb, pc, pc + 1);
    relocInfo::change_reloc_info_for_address(&iter, (address) pc, relocInfo::oop_type, relocInfo::none);

    pc = (address)(_pc_start + NativeMovConstReg::add_offset);
    RelocIterator iter2(cb, pc, pc+1);
    relocInfo::change_reloc_info_for_address(&iter2, (address) pc, relocInfo::oop_type, relocInfo::none);
  }

}

ArrayCopyStub::ArrayCopyStub(CodeEmitInfo* info, StaticCallStub* call_stub):
  _info(info)
, _call_stub(call_stub) { }


void ArrayCopyStub::emit_code(LIR_Assembler* ce) {
  //---------------slow case: call to native-----------------
  __ bind(_entry);
  __ mov(src().as_register(),     O0);
  __ mov(src_pos().as_register(), O1);
  __ mov(dst().as_register(),     O2);
  __ mov(dst_pos().as_register(), O3);
  __ mov(length().as_register(),  O4);

  address call_pc = __ pc();
  ce->emit_code_stub(_call_stub);
  __ call(Runtime1::entry_for(Runtime1::resolve_invokestatic_id), relocInfo::static_call_type);
  __ delayed()->nop();
  ce->add_call_info_here(_info);

#ifndef PRODUCT
  __ set((intptr_t)&Runtime1::_arraycopy_slowcase_cnt, O0);
  __ ld(O0, 0, O1);
  __ inc(O1);
  __ st(O1, 0, O0);
#endif

  __ br(Assembler::always, false, Assembler::pt, _continuation);
  __ delayed()->nop();

  _call_stub->set_code_pc(call_pc);
}


#undef __

