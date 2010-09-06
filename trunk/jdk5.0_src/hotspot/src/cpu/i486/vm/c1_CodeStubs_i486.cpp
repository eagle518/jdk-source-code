#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)c1_CodeStubs_i486.cpp	1.85 04/04/14 17:27:31 JVM"
#endif
// 
// Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
// SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
// 

#include "incls/_precompiled.incl"
#include "incls/_c1_CodeStubs_i486.cpp.incl"


#define __ ce->masm()->

void StaticCallStub::emit_code(LIR_Assembler* ce) {
  if (os::is_MP()) {
    // make sure that the displacement word of the call ends up word aligned
    int offset = __ offset() + NativeMovConstReg::instruction_size + NativeCall::displacement_offset;
    while (offset++ % BytesPerWord != 0) {
      __ nop();
    }
  }
  __ relocate(static_stub_Relocation::spec(_call_pc));
  __ movl(ebx, (jobject)NULL);
  // must be set to -1 at code generation time
  assert(!os::is_MP() || ((__ offset() + 1) % BytesPerWord) == 0, "must be aligned on MP");
  __ jmp((address)-1, relocInfo::runtime_call_type);
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
  // Pass the array index in eax since the runtime stub will add register state to the stack
  if (_index_rinfo.is_valid()) {
    Register index_reg = _index_rinfo.as_register();
    if (index_reg != eax) {
      __ movl(eax, index_reg);
    }
  } else {
    __ movl(eax, _index_value);
  }
  if (_throw_index_out_of_bounds_exception) {
    __ call(Runtime1::entry_for(Runtime1::throw_index_exception_id), relocInfo::runtime_call_type);
  } else {
    __ call(Runtime1::entry_for(Runtime1::range_check_failed_id), relocInfo::runtime_call_type);
  }
  ce->add_call_info_here(_info);
}


void DivByZeroStub::emit_code(LIR_Assembler* ce) {
  if (_offset != -1) {
    ce->compilation()->null_check_table()->append(_offset, __ offset());
  }
  __ bind(_entry);
  // the code for long division mucks with esp so in the exception case we need to reset it's value
  __ leal(esp, Address(ebp, -ce->initial_frame_size_in_bytes()));
  __ call(Runtime1::entry_for(Runtime1::throw_div0_exception_id), relocInfo::runtime_call_type);
  ce->add_call_info_here(_info);
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
  assert(__ esp_offset() == 0, "frame size should be fixed");
  __ bind(_entry);
  __ movl(edx, _klass_reg.as_register());
  __ call(Runtime1::entry_for(_stub_id), relocInfo::runtime_call_type);
  ce->add_call_info_here(_info);
  assert(_result->as_register() == eax, "result must in eax");
  __ jmp(_continuation);
}


// Implementation of NewTypeArrayStub

NewTypeArrayStub::NewTypeArrayStub(RInfo klass_reg, RInfo length, RInfo result, CodeEmitInfo* info) {
  _klass_reg = klass_reg;
  _length = length;
  _result = result;
  _info = new CodeEmitInfo(info);
}


void NewTypeArrayStub::emit_code(LIR_Assembler* ce) {
  assert(__ esp_offset() == 0, "frame size should be fixed");
  __ bind(_entry);
  assert(_length.as_register() == ebx, "length must in ebx");
  assert(_klass_reg.as_register() == edx, "klass_reg must in edx");
  __ call(Runtime1::entry_for(Runtime1::new_type_array_id), relocInfo::runtime_call_type);
  ce->add_call_info_here(_info);
  assert(_result.as_register() == eax, "result must in eax");
  __ jmp(_continuation);
}


// Implementation of NewObjectArrayStub

NewObjectArrayStub::NewObjectArrayStub(RInfo klass_reg, RInfo length, RInfo result, CodeEmitInfo* info) {  
  _klass_reg = klass_reg;
  _result = result;
  _length = length;
  _info = new CodeEmitInfo(info);
}


void NewObjectArrayStub::emit_code(LIR_Assembler* ce) {
  assert(__ esp_offset() == 0, "frame size should be fixed");
  __ bind(_entry);
  assert(_length.as_register() == ebx, "length must in ebx");
  assert(_klass_reg.as_register() == edx, "klass_reg must in edx");
  __ call(Runtime1::entry_for(Runtime1::new_object_array_id), relocInfo::runtime_call_type);
  ce->add_call_info_here(_info);
  assert(_result.as_register() == eax, "result must in eax");
  __ jmp(_continuation);
}


// Implementation of MonitorAccessStubs

MonitorEnterStub::MonitorEnterStub(RInfo obj_reg, RInfo lock_reg, CodeEmitInfo* info)
: MonitorAccessStub(obj_reg, lock_reg)
{
  _info = new CodeEmitInfo(info);
}


void MonitorEnterStub::emit_code(LIR_Assembler* ce) {
  assert(__ esp_offset() == 0, "frame size should be fixed");
  __ bind(_entry);
  ce->push_parameter(_obj_reg.as_register(),  1);
  ce->push_parameter(_lock_reg.as_register(), 0);
  if ((ce->compilation()->jvmpi_event_method_entry_enabled() ||
       ce->compilation()->jvmpi_event_method_entry2_enabled()) &&
      _info->bci() == SynchronizationEntryBCI) {
    // This stub will perform jvmpi notification if its caller gets deoptimized after the monitorenter.
    __ call(Runtime1::entry_for(Runtime1::monitorenter_with_jvmpi_id), relocInfo::runtime_call_type);
  } else {
    __ call(Runtime1::entry_for(Runtime1::monitorenter_id), relocInfo::runtime_call_type);
  }
  ce->add_call_info_here(_info);
  // preserve receiver in ecx in case the method is virtual
  if (!ce->method()->is_static()) {
    // cached receiver is in ecx
    ce->load_receiver_reg(ecx);
  }
  __ jmp(_continuation);
}


void MonitorExitStub::emit_code(LIR_Assembler* ce) {
  __ bind(_entry);
  if (_compute_lock) {
    // lock_reg was destroyed by fast unlocking attempt => recompute it
    ce->monitor_address(_monitor_ix, _lock_reg);
  }
  ce->push_parameter(_lock_reg.as_register(), 0);
  // note: non-blocking leaf routine => no call info needed
  __ call(Runtime1::entry_for(Runtime1::monitorexit_id), relocInfo::runtime_call_type);
  __ jmp(_continuation);
}


// Implementation of patching: 
// - Copy the code at given offset to an inlined buffer (first the bytes, then the number of bytes)
// - Replace original code with a call to the stub
// At Runtime:
// - call to stub, jump to runtime 
// - in runtime: preserve all registers (especially objects, i.e., source and destination object)
// - in runtime: after initializing class, restore original code, reexecute instruction 

int PatchingStub::_patch_info_offset = -NativeGeneralJump::instruction_size;

void PatchingStub::align_patch_site(MacroAssembler* masm) {
  // We're patching a 5-7 byte instruction on intel and we need to
  // make sure that we don't see a piece of the instruction.  It
  // appears mostly impossible on Intel to simply invalidate other
  // processors caches and since they may do aggressive prefetch it's
  // very hard to make a guess about what code might be in the icache.
  // Force the instruction to be double word aligned so that it
  // doesn't span a cache line.
  masm->align(round_to(NativeGeneralJump::instruction_size, wordSize));
}

void PatchingStub::emit_code(LIR_Assembler* ce) {
  assert(NativeCall::instruction_size <= _bytes_to_copy && _bytes_to_copy <= 0xFF, "not enough room for call");

  Label call_patch;

  // static field accesses have special semantics while the class
  // initializer is being run so we emit a test which can be used to
  // check that this code is being executed by the initializing
  // thread.
  address being_initialized_entry = __ pc();
  if (_id == access_field_id) {
    assert(_obj != noreg, "must be a valid register");
    Register tmp = eax;
    if (_obj == tmp) tmp = ebx;
    __ pushl(tmp);
    __ get_thread(tmp);
    __ cmpl(tmp, Address(_obj, instanceKlass::init_thread_offset_in_bytes() + sizeof(klassOopDesc)));
    __ popl(tmp);
    __ jcc(Assembler::notEqual, call_patch);
  }

  // make a copy the code which is going to be patched.
  for ( int i = 0; i < _bytes_to_copy; i++) {
    address ptr = (address)(_pc_start + i);
    int a_byte = (*ptr) & 0xFF;
    __ a_byte (a_byte);
  }

  address end_of_patch = __ pc();
  int bytes_to_skip = patch_info_size;
  if (_id == access_field_id) {
    // access_field patches may execute the patched code before it's
    // copied back into place so we need to jump back into the main
    // code of the nmethod to continue execution.
    bytes_to_skip += 5;
    __ jmp(_patch_site_continuation);
  }
  // emit the offsets needed to find the code to patch
  int being_initialized_entry_offset = __ pc() - being_initialized_entry + patch_info_size;
  __ a_byte(being_initialized_entry_offset);
  __ a_byte(bytes_to_skip);
  __ a_byte(_bytes_to_copy);
  address patch_info_pc = __ pc();
  assert(patch_info_pc - end_of_patch == bytes_to_skip, "incorrect patch info");

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
  assert(_patch_info_offset == (patch_info_pc - __ pc()), "must not change");
  ce->add_call_info_here(_info);
  int jmp_off = __ offset();
  __ jmp(_patch_site_entry);
  // Add enough nops so deoptimization can overwrite the jmp above with a call
  // and not destroy the world.
  for (int j = __ offset() ; j < jmp_off + 5 ; j++ ) {
    __ nop();
  }
  if (_id == load_klass_id) {
    CodeBuffer* cb = __ code();
    RelocIterator iter(cb, (address)_pc_start, (address)(_pc_start + 1));
    relocInfo::change_reloc_info_for_address(&iter, (address) _pc_start, relocInfo::oop_type, relocInfo::none);
  }
}


void ImplicitNullCheckStub::emit_code(LIR_Assembler* ce) {
  ce->compilation()->null_check_table()->append(_offset, __ offset());
  __ bind(_entry);
  __ call(Runtime1::entry_for(Runtime1::throw_null_pointer_exception_id), relocInfo::runtime_call_type);
  ce->add_call_info_here(_info);
}


void SimpleExceptionStub::emit_code(LIR_Assembler* ce) {
  assert(__ esp_offset() == 0, "frame size should be fixed");
  
  __ bind(_entry);
  if (_obj.is_valid()) {
    Register obj_reg = _obj.as_register();
    if (obj_reg != eax) {
      __ movl(eax, obj_reg);
    }
  }
  __ call(Runtime1::entry_for(_stub), relocInfo::runtime_call_type);
  ce->add_call_info_here(_info_for_exception);
}


ArrayStoreExceptionStub::ArrayStoreExceptionStub(CodeEmitInfo* info_for_exception):
  _info_for_exception(info_for_exception) {
  _info_for_exception->check_is_exception_info();
}


void ArrayStoreExceptionStub::emit_code(LIR_Assembler* ce) {
  assert(__ esp_offset() == 0, "frame size should be fixed");
  __ bind(_entry);
  __ call(Runtime1::entry_for(Runtime1::throw_array_store_exception_id), relocInfo::runtime_call_type);
  ce->add_call_info_here(_info_for_exception);
}


ArrayCopyStub::ArrayCopyStub(CodeEmitInfo* info, StaticCallStub* call_stub):
 _info(new CodeEmitInfo(info))
, _call_stub(call_stub)
{
}


void ArrayCopyStub::emit_code(LIR_Assembler* ce) {
  //---------------slow case: call to native-----------------
  __ bind(_entry);
  // push parameters
  ce->push_parameter(src().as_register(), 4);
  ce->push_parameter(src_pos().as_register(), 3);
  ce->push_parameter(dst().as_register(), 2);
  ce->push_parameter(dst_pos().as_register(), 1);
  ce->push_parameter(length().as_register(), 0);

  ce->align_call(lir_static_call);

  address call_pc = __ pc();
  ce->emit_code_stub(_call_stub);
  __ call(Runtime1::entry_for(Runtime1::resolve_invokestatic_id), relocInfo::static_call_type);
  ce->add_call_info_here(_info);

#ifndef PRODUCT
  __ incl(Address((intptr_t)&Runtime1::_arraycopy_slowcase_cnt, relocInfo::none));
#endif
  
  __ jmp(_continuation);
  _call_stub->set_code_pc(call_pc);
}


#undef __
