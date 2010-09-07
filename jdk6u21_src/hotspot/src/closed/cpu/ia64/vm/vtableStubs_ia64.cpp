/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */


#include "incls/_precompiled.incl"
#include "incls/_vtableStubs_ia64.cpp.incl"

// machine-dependent part of VtableStubs: create vtableStub of correct size and
// initialize its code

#define __ masm->


#ifndef PRODUCT
extern "C" void bad_compiled_vtable_index(JavaThread* thread, oop receiver, int index);
#endif


// used by compiler only; may use only caller saved, non-argument registers
// NOTE:  %%%% if any change is made to this stub make sure that the function
//             pd_code_size_limit is changed to ensure the correct size for VtableStub
VtableStub* VtableStubs::create_vtable_stub(int vtable_index) {
  int ia64_code_length = VtableStub::pd_code_size_limit(true);
  VtableStub* s = new(ia64_code_length) VtableStub(true, vtable_index);
  ResourceMark rm;
  CodeBuffer cb(s->entry_point(), ia64_code_length);
  MacroAssembler* masm = new MacroAssembler(&cb);


//   __ os_breakpoint();

#ifndef PRODUCT
  if (CountCompiledCalls) {
    const Register GR14_counter_addr = GR14;
    const Register GR15_counter = GR15;
    __ mova(GR14_counter_addr, SharedRuntime::nof_megamorphic_calls_addr());
    __ ld4(GR15_counter, GR14_counter_addr);
    __ add(GR15_counter, GR15_counter, 1);
    __ st4(GR14_counter_addr, GR15_counter);
  }
#endif

  // We are in a new frame at this point (just inputs, no locals, no outs)

  // This assert is wrong in the sense that it thinks sparc like as far as frames
  // not ia64 like where the frame comes into existance at the call not the alloc (save)
  assert(VtableStub::receiver_location() == GR_O0->as_VMReg(), "receiver expected in GR_O0");

  // get receiver klass
  const Register GR14_rcvr_klass = GR14;  // addr then value

  __ add(GR14_rcvr_klass, GR_I0, oopDesc::klass_offset_in_bytes());

  // We might implicit NULL fault here
  __ flush_bundle();

  address npe_addr = __ pc();

  __ ld8(GR14_rcvr_klass, GR14_rcvr_klass);
  __ flush_bundle();


  // set methodOop (in case of interpreted method), and destination address
  int entry_offset = instanceKlass::vtable_start_offset() + vtable_index*vtableEntry::size();
#ifndef PRODUCT
  if (DebugVtables) {
    Label L;
    // check offset vs vtable length
    const Register GR15_vtable_len = GR15;  // Address then value
    __ add(GR15_vtable_len, GR14_rcvr_klass, instanceKlass::vtable_length_offset()*wordSize);
    __ ld4(GR15_vtable_len, GR15_vtable_len);
    __ cmp4(PR0, PR6, vtable_index*vtableEntry::size(), GR15_vtable_len, Assembler::higher);
    __ br(PR6, L, Assembler::dpnt);
    __ mov(GR16, vtable_index);
    __ call_VM(noreg, CAST_FROM_FN_PTR(address, bad_compiled_vtable_index), GR_I0, GR16);
    __ bind(L);
  }
#endif
  const Register GR16_vtable_addr = GR16;
  const Register GR17_entry_addr = GR17;
  int v_off = entry_offset*wordSize + vtableEntry::method_offset_in_bytes();

  __ add(GR16_vtable_addr, v_off, GR14_rcvr_klass);
  __ ld8(GR27_method, GR16_vtable_addr);

  #ifndef PRODUCT
  if (DebugVtables) {
    Label L;
    __ cmp(PR0, PR6, GR0, GR27_method, Assembler::equal);
    __ br(PR0, L);
    __ stop("Vtable entry is ZERO");
    __ bind(L);
  }
  #endif

  const Register GR17_target = GR17;  // Addr then value
  __ add(GR17_target, GR27_method, in_bytes(methodOopDesc::from_compiled_offset()));
  __ flush_bundle();

  address ame_addr = __ pc();  // if the vtable entry is null, the method is abstract

  __ ld8(GR17_target, GR17_target);
  __ flush_bundle();

  // jump to target (either compiled code or c2i massager)
  __ mov(BR7_SCRATCH, GR17_target);
  __ br(BR7_SCRATCH);

  __ flush_bundle();

  masm->flush();
  s->set_exception_points(npe_addr, ame_addr);
  return s;
}


// NOTE:  %%%% if any change is made to this stub make sure that the function
//             pd_code_size_limit is changed to ensure the correct size for VtableStub
VtableStub* VtableStubs::create_itable_stub(int vtable_index) {
  const int ia64_code_length = VtableStub::pd_code_size_limit(false);
  VtableStub* s = new(ia64_code_length) VtableStub(false, vtable_index);
  ResourceMark rm;
  CodeBuffer cb(s->entry_point(), ia64_code_length);
  MacroAssembler* masm = new MacroAssembler(&cb);

#ifndef PRODUCT
  if (CountCompiledCalls) {
    const Register GR14_counter_addr = GR14;
    const Register GR15_counter = GR15;
    __ mova(GR14_counter_addr, SharedRuntime::nof_megamorphic_calls_addr());
    __ ld4(GR15_counter, GR14_counter_addr);
    __ add(GR15_counter, GR15_counter, 1);
    __ st4(GR14_counter_addr, GR15_counter);
  }
#endif /* PRODUCT */

  // We are in a new frame at this point (just inputs, no locals, no outs)

  // This assert is wrong in the sense that it thinks sparc like as far as frames
  // not ia64 like where the frame comes into existance at the call not the alloc (save)
  assert(VtableStub::receiver_location() == GR_O0->as_VMReg(), "receiver expected in GR_O0");

  // Entry arguments:
  //  GR27_interface: Interface
  //  I0:             Receiver
  //
  const Register GR27_interface = GR27;

  // get receiver klass
  const Register rcvr_klass_addr = GR14;  // addr then value
  const Register rcvr_klass = rcvr_klass_addr;

  __ add(rcvr_klass_addr, GR_I0, oopDesc::klass_offset_in_bytes());

  // We might implicit NULL fault here
  __ flush_bundle();

  address npe_addr = __ pc();

  __ ld8(rcvr_klass, rcvr_klass_addr);
  __ flush_bundle();

  // load start of itable entries into GR15_itable_entry
  const Register vtable_len_addr = GR15;
  const Register vtable_len = vtable_len_addr;
  const Register itable_entry_addr = GR16;

  const int vtable_base_offset = instanceKlass::vtable_start_offset() * wordSize;

  __ add(vtable_len_addr, rcvr_klass, instanceKlass::vtable_length_offset() * wordSize);
  __ ld4(vtable_len, vtable_len_addr);

  // %%% Could store the aligned, prescaled offset in the klassoop.
  __ shladd(itable_entry_addr, vtable_len, exact_log2(vtableEntry::size() * wordSize), rcvr_klass);

  // Loop over all itable entries until desired interfaceOop(Rinterface) found
  Label search;
  __ bind(search);

  const Register itable_interface_addr = GR17;
  const Register itable_interface = itable_interface_addr;

  __ add(itable_interface_addr, itable_entry_addr,
                                vtable_base_offset + itableOffsetEntry::interface_offset_in_bytes());
  __ ld8(itable_interface, itable_interface_addr);

  // If the entry is NULL then we've reached the end of the table
  // without finding the expected interface, so throw an exception
  Label throw_icce;
  __ cmp(PR6, PR0, itable_interface, 0, Assembler::equal);
  __ br(PR6, throw_icce, Assembler::dpnt);

  __ cmp(PR6, PR0, GR27_interface, itable_interface, Assembler::notEqual);
  __ add(PR6, itable_entry_addr, itable_entry_addr, itableOffsetEntry::size() * wordSize);
  __ br(PR6, search, Assembler::dpnt);

  // entry found and itable_entry_addr points to it, get offset of vtable for interface
  const Register vtable_offset_addr = GR18;
  const Register vtable_offset = vtable_offset_addr;

  __ add(vtable_offset_addr, itable_entry_addr,
                             vtable_base_offset + itableOffsetEntry::offset_offset_in_bytes());
  __ ld4(vtable_offset, vtable_offset_addr);

  // Compute itableMethodEntry and get methodOop and entry point for compiler
  const int method_offset = (itableMethodEntry::size() * wordSize * vtable_index) +
                            itableMethodEntry::method_offset_in_bytes();

  const Register itable_method_addr = GR19;
  const Register itable_method = itable_method_addr;

  __ add(itable_method_addr, rcvr_klass, vtable_offset);
  __ add(itable_method_addr, itable_method_addr, method_offset);
  __ ld8(GR27_method, itable_method_addr);


#ifndef PRODUCT
  if (DebugVtables) {
    Label L01;
    __ cmp(PR6, PR0, GR27_method, 0, Assembler::notEqual);
    __ br(PR6, L01, Assembler::dptk);
    __ stop("methodOop is null");
    __ bind(L01);
    //    __ verify_oop(GR27_method);
  }
#endif

  const Register target_addr = GR20;
  const Register target = target_addr;

  __ add(target_addr, GR27_method, in_bytes(methodOopDesc::from_compiled_offset()));
  __ flush_bundle();

  address ame_addr = __ pc();  // if the vtable entry is null, the method is abstract
  __ ld8(target, target_addr);
  __ flush_bundle();

  // GR27_method:  methodOop
  // I0:           Receiver
  // target:       entry point
  __ mov(BR7, target);
  __ br(BR7);

  __ flush_bundle();

  __ bind(throw_icce);
  __ mov(BR7, StubRoutines::throw_IncompatibleClassChangeError_entry());
  __ br(BR7);

  __ flush_bundle();

  masm->flush();

  guarantee(__ pc() <= s->code_end(), "overflowed buffer");

  s->set_exception_points(npe_addr, ame_addr);
  return s;
}


int VtableStub::pd_code_size_limit(bool is_vtable_stub) {
  if (DebugVtables || CountCompiledCalls || VerifyOops) return 1024;
  else {
    if (is_vtable_stub) {
      return 1024;
    } else {
      return 1024;
    }
  }
}


int VtableStub::pd_code_alignment() {
  // Itanium cache line size is 1 bundle
  const unsigned int icache_line_size = 16;
  return icache_line_size;
}
