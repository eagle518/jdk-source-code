#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)vtableStubs_amd64.cpp	1.5 03/12/23 16:35:59 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_vtableStubs_amd64.cpp.incl"

// machine-dependent part of VtableStubs: create VtableStub of correct size and
// initialize its code

#define __ masm->

#ifndef PRODUCT
extern "C" void bad_compiled_vtable_index(JavaThread* thread, 
                                          oop receiver, 
                                          int index);
#endif

// used by compiler only; may use only caller saved registers rax,
// rbx, rcx.  rdx holds first int arg, rsi, rdi, rbp are callee-save &
// must be preserved.  Leave reciever in rcx; required behavior when
// +OptoArgsInRegisters is modifed to put first oop in rcx.
//
// NOTE: If this code is used by the C1, the receiver_location is always 0.
VtableStub* VtableStubs::create_vtable_stub(int vtable_index, 
                                            int receiver_location) 
{
  const int amd64_code_length = VtableStub::pd_code_size_limit(true);
  VtableStub* s = new(amd64_code_length) VtableStub(true, 
                                                    vtable_index,
                                                    receiver_location);
  ResourceMark rm;
  MacroAssembler* masm = 
    new MacroAssembler(new CodeBuffer(s->entry_point(), amd64_code_length));

#ifndef PRODUCT
#ifdef COMPILER2
  if (CountCompiledCalls) {
    __ incl(Address((address) OptoRuntime::nof_megamorphic_calls_addr(),
                    relocInfo::none));
  }
#endif
#endif

  // get receiver (need to skip return address on top of stack)
#ifdef COMPILER1
  assert(receiver_location == 0,
         "receiver is always in rcx - no location info needed");
#else
  if (receiver_location < SharedInfo::stack0) {
    assert(receiver_location == RCX_num, "receiver expected in rcx");
  } else {
    __ movq(rcx, 
            Address(rsp, 
                    SharedInfo::reg2stack(OptoReg::Name(receiver_location)) *
                    wordSize + wordSize /*skip return address*/));
  }
#endif

  // get receiver klass
  address npe_addr = __ pc();
  __ movq(rax, Address(rcx, oopDesc::klass_offset_in_bytes()));

  // compute entry offset (in words)
  int entry_offset = 
    instanceKlass::vtable_start_offset() + vtable_index * vtableEntry::size();

#ifndef PRODUCT
  if (DebugVtables) {
    Label L;
    // check offset vs vtable length
    __ cmpl(Address(rax, instanceKlass::vtable_length_offset() * wordSize),
            vtable_index * vtableEntry::size());
    __ jcc(Assembler::greater, L);
    __ movl(rbx, vtable_index);
    __ call_VM(noreg, 
               CAST_FROM_FN_PTR(address, bad_compiled_vtable_index), rcx, rbx);
    __ bind(L);
  }
#endif // PRODUCT

  // load methodOop and target address
#ifdef COMPILER1
  __ movq(rbx, Address(rax, 
                       entry_offset * wordSize + 
                       vtableEntry::method_offset_in_bytes()));
  address ame_addr = __ pc();
  __ movq(rdx, 
          Address(rbx, 
                  methodOopDesc::from_compiled_code_entry_point_offset()));
  if (DebugVtables) {
    Label L;
    __ testq(rdx, rdx);
    __ jcc(Assembler::notZero, L);
    __ stop("Vtable entry is NULL");
    __ bind(L);
  }
  // rax: receiver klass
  // rbx: methodOop
  // rcx: receiver
  // rdx: entry point
  __ jmp(rdx);
#else
  __ movq(rax, Address(rax, entry_offset * wordSize + 
                       vtableEntry::method_offset_in_bytes()));
  address ame_addr = __ pc();
  __ movq(rbx, 
          Address(rax, 
                  methodOopDesc::from_compiled_code_entry_point_offset()));  

  if (DebugVtables) {
    Label L;
    __ testq(rbx, rbx);
    __ jcc(Assembler::notZero, L);
    __ stop("Vtable entry is NULL");
    __ bind(L);
  } 

  // jump to target (either compiled code or c2iadapter)
  // rax: methodOop (in case we call c2iadapter)
  __ jmp(rbx);
#endif // COMPILER1

  masm->flush();
  s->set_exception_points(npe_addr, ame_addr);
  return s;
}


VtableStub* VtableStubs::create_itable_stub(int vtable_index, 
                                            int receiver_location) 
{
  // Note well: pd_code_size_limit is the absolute minimum we can get
  // away with.  If you add code here, bump the code stub size
  // returned by pd_code_size_limit!
  const int amd64_code_length = VtableStub::pd_code_size_limit(false);
  VtableStub* s = new(amd64_code_length) VtableStub(false, 
                                                     vtable_index, 
                                                     receiver_location);
  ResourceMark rm;
  MacroAssembler* masm = 
    new MacroAssembler(new CodeBuffer(s->entry_point(), amd64_code_length));
  
  // Entry arguments:
  //  rax: Interface
  //  rcx: Receiver
  
  // get receiver (need to skip return address on top of stack)
  COMPILER1_ONLY(
    assert(receiver_location == 0, 
           "receiver is always in rcx - no location info needed");
  )
 
#ifdef COMPILER2
  if (receiver_location < SharedInfo::stack0) {
    assert(receiver_location == RCX_num, "receiver expected in rcx");
  } else {
    __ movq(rcx, 
            Address(rsp, 
                    SharedInfo::reg2stack(OptoReg::Name(receiver_location)) *
                    wordSize + wordSize /*skip return address*/));
  }
#endif
  
  // get receiver klass (also an implicit null-check)
  address npe_addr = __ pc();
  __ movq(rbx, Address(rcx, oopDesc::klass_offset_in_bytes()));    

  COMPILER1_ONLY(__ movq(rsi, rbx);) // Save klass in free register    
  COMPILER2_ONLY(__ pushq(rdx);) // Most registers are in use, so save a few

  // compute itable entry offset (in words)  
  const int base = instanceKlass::vtable_start_offset() * wordSize;    
  assert(vtableEntry::size() * wordSize == 8, 
         "adjust the scaling in the code below");
  // Get length of vtable
  __ movl(rdx, 
          Address(rbx, instanceKlass::vtable_length_offset() * wordSize));
  __ leaq(rbx, Address(rbx, rdx, Address::times_8, base));

  if (HeapWordsPerLong > 1) {
    // Round up to align_object_offset boundary
    __ round_to_q(rbx, BytesPerLong);
  }
  Label hit, next, entry;
  
  __ jmp(entry);

  __ bind(next);
  __ addq(rbx, itableOffsetEntry::size() * wordSize);
  
  __ bind(entry);

#ifdef ASSERT
    // Check that the entry is non-null
  if (DebugVtables) { 
    Label L;
    __ pushq(rbx);
    __ movq(rbx, Address(rbx, itableOffsetEntry::interface_offset_in_bytes()));
    __ testq(rbx, rbx);
    __ jcc(Assembler::notZero, L);
    __ stop("null entry point found in itable's offset table");
    __ bind(L);
    __ popq(rbx);    
  }
#endif

  __ cmpq(rax, Address(rbx, itableOffsetEntry::interface_offset_in_bytes()));
  __ jcc(Assembler::notEqual, next);    
  
  // We found a hit, move offset into rdx
  __ movl(rdx, Address(rbx, itableOffsetEntry::offset_offset_in_bytes()));

  // Compute itableMethodEntry.  
  const int method_offset = 
    (itableMethodEntry::size() * wordSize * vtable_index) + 
    itableMethodEntry::method_offset_in_bytes();
  
  // Get methodOop and entrypoint for compiler    
#ifdef COMPILER1
    __ movq(rbx, Address(rsi, rdx, Address::times_1, method_offset));  
    address ame_addr = __ pc();
    __ movq(rdx, 
            Address(rbx, 
                    methodOopDesc::from_compiled_code_entry_point_offset())); 
#ifdef ASSERT
    if (DebugVtables) { 
      Label L2;
      __ testq(rdx, rdx);
      __ jcc(Assembler::notZero, L2);
      __ stop("compiler entrypoint is null");
      __ bind(L2);    
    }
#endif // ASSERT
    // rbx: methodOop
    // rcx: receiver
    // rdx: entry point
    __ jmp(rdx);
#endif // COMPILER1

#ifdef COMPILER2
    // Get klass pointer again
    __ movq(rax, Address(rcx, oopDesc::klass_offset_in_bytes()));    
    __ movq(rax, Address(rax, rdx, Address::times_1, method_offset));  

    // Restore saved register, before possible trap.
    __ popq(rdx);

    address ame_addr = __ pc();
    __ movq(rbx, 
            Address(rax, 
                    methodOopDesc::from_compiled_code_entry_point_offset())); 

#ifdef ASSERT
  if (DebugVtables) {
      Label L1;
      __ testq(rax, rax);
      __ jcc(Assembler::notZero, L1);
      __ stop("methodOop is null");
      __ bind(L1);

      Label L2;
      __ testq(rbx, rbx);
      __ jcc(Assembler::notZero, L2);
      __ stop("compiler entrypoint is null");
      __ bind(L2);
    }
#endif // ASSERT

    // rax: methodOop
    // rcx: receiver
    // rbx: entry point      
    __ jmp(rbx);
#endif // COMPILER2
    
  masm->flush();
  s->set_exception_points(npe_addr, ame_addr);
  return s;
}

// XXX calculate exact sizes
int VtableStub::pd_code_size_limit(bool is_vtable_stub)
{
  if (is_vtable_stub) {
    // Vtable stub size
    COMPILER1_ONLY(
      return (DebugVtables ? 250 : 23) + (CountCompiledCalls ? 6 : 0) - 6;)
    COMPILER2_ONLY(
      return 1000+(DebugVtables ? 250 : 24) + (CountCompiledCalls ? 6 : 0) - 6;)    
  } else {
    // Itable stub size
    COMPILER1_ONLY(
      return (DebugVtables ? 300 : 67) + (CountCompiledCalls ? 6 : 0); )
    COMPILER2_ONLY(
      return 1000+(DebugVtables ? 300 : 62) + (CountCompiledCalls ? 6 : 0);  )
  }
}

int VtableStub::pd_code_alignment()
{
  return wordSize;
}

