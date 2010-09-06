#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)nativeInst_sparc.cpp	1.83 03/12/23 16:37:18 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_nativeInst_sparc.cpp.incl"


void NativeInstruction::set_data64_sethi(address instaddr, intptr_t x) {
  ResourceMark rm;
  CodeBuffer* buf = new CodeBuffer(instaddr, 10 * BytesPerInstWord );
  MacroAssembler* _masm = new MacroAssembler(buf);
  Register destreg; 
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe(instaddr);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */

  destreg = inv_rd(*(unsigned int *)instaddr); 
  // Generate a the new sequence
  _masm->sethi( Address( destreg, (address)x ), true );
  ICache::invalidate_range(instaddr, 7 * BytesPerInstWord);
}

void NativeInstruction::verify() {
  // make sure code pattern is actually an instruction address
  address addr = addr_at(0);
  if (addr == 0 || ((intptr_t)addr & 3) != 0) {
    fatal("not an instruction address");
  }
}

void NativeInstruction::print() {
  tty->print_cr(INTPTR_FORMAT ": 0x%x", addr_at(0), long_at(0));
}

void NativeInstruction::set_long_at(int offset, int i) {
  address addr = addr_at(offset);
  *(int*)addr = i;
  ICache::invalidate_word(addr);
}

void NativeInstruction::set_jlong_at(int offset, jlong i) {
  address addr = addr_at(offset);
  *(jlong*)addr = i;
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe(addr);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
  // Don't need to invalidate 2 words here, because
  // the flush instruction operates on doublewords.
  ICache::invalidate_word(addr);
}

void NativeInstruction::set_addr_at(int offset, address x) {
  address addr = addr_at(offset);
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe(addr);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
  assert( ((intptr_t)addr & (wordSize-1)) == 0, "set_addr_at bad address alignment");
  *(uintptr_t*)addr = (uintptr_t)x;
  // Don't need to invalidate 2 words here in the 64-bit case,
  // because the flush instruction operates on doublewords.
  ICache::invalidate_word(addr);
  // The Intel code has this assertion for NativeCall::set_destination,
  // NativeMovConstReg::set_data, NativeMovRegMem::set_offset,
  // NativeJump::set_jump_destination, and NativePushImm32::set_data
  //assert (Patching_lock->owned_by_self(), "must hold lock to patch instruction")
}

bool NativeInstruction::is_zero_test(Register &reg) {
  int x = long_at(0);
  Assembler::op3s temp = (Assembler::op3s) (Assembler::sub_op3 | Assembler::cc_bit_op3);
  if (is_op3(x, temp, Assembler::arith_op) &&
      inv_immed(x) && inv_rd(x) == G0) {
      if (inv_rs1(x) == G0) {
        reg = inv_rs2(x);
        return true;  
      } else if (inv_rs2(x) == G0) {
        reg = inv_rs1(x);
        return true;  
      }
  }
  return false;
}

bool NativeInstruction::is_load_store_with_small_offset(Register reg) {
  int x = long_at(0);
  if (is_op(x, Assembler::ldst_op) &&
      inv_rs1(x) == reg && inv_immed(x)) {
    return true;
  }
  return false;
}

void NativeCall::verify() {
  NativeInstruction::verify();
  // make sure code pattern is actually a call instruction
  if (!is_op(long_at(0), Assembler::call_op)) {
    fatal("not a call");
  }
}

void NativeCall::print() {
  tty->print_cr(INTPTR_FORMAT ": call " INTPTR_FORMAT, instruction_address(), destination());
}


// MT-safe patching of a call instruction (and following word).
// First patches the second word, and then atomicly replaces
// the first word with the first new instruction word.
// Other processors might briefly see the old first word
// followed by the new second word.  This is OK if the old
// second word is harmless, and the new second word may be
// harmlessly executed in the delay slot of the call.
void NativeCall::replace_mt_safe(address instr_addr, address code_buffer) {
  assert(Patching_lock->is_locked() ||
         SafepointSynchronize::is_at_safepoint(), "concurrent code patching"); 
   assert (instr_addr != NULL, "illegal address for code patching");
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe(instr_addr);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
   NativeCall* n_call =  nativeCall_at (instr_addr); // checking that it is a call
   assert(NativeCall::instruction_size == 8, "wrong instruction size; must be 8");
   int i0 = ((int*)code_buffer)[0];
   int i1 = ((int*)code_buffer)[1];
   int* contention_addr = (int*) n_call->addr_at(1*BytesPerInstWord);
   assert(inv_op(*contention_addr) == Assembler::arith_op ||
          *contention_addr == nop_instruction() || !VM_Version::v9_instructions_work(), 
          "must not interfere with original call");
   // The set_long_at calls do the ICacheInvalidate so we just need to do them in reverse order
   n_call->set_long_at(1*BytesPerInstWord, i1);
   n_call->set_long_at(0*BytesPerInstWord, i0);
   // NOTE:  It is possible that another thread T will execute
   // only the second patched word.
   // In other words, since the original instruction is this
   //    call patching_stub; nop                   (NativeCall)
   // and the new sequence from the buffer is this:
   //    sethi %hi(K), %r; add %r, %lo(K), %r      (NativeMovConstReg)
   // what T will execute is this:
   //    call patching_stub; add %r, %lo(K), %r
   // thereby putting garbage into %r before calling the patching stub.
   // This is OK, because the patching stub ignores the value of %r.

   // Make sure the first-patched instruction, which may co-exist
   // briefly with the call, will do something harmless.
   assert(inv_op(*contention_addr) == Assembler::arith_op ||
          *contention_addr == nop_instruction() || !VM_Version::v9_instructions_work(), 
          "must not interfere with original call");
}

// Similar to replace_mt_safe, but just changes the destination.  The
// important thing is that free-running threads are able to execute this
// call instruction at all times.  Thus, the displacement field must be
// instruction-word-aligned.  This is always true on SPARC.
//
// Used in the runtime linkage of calls; see class CompiledIC.
void NativeCall::set_destination_mt_safe(address dest) {
  assert(Patching_lock->is_locked() ||
         SafepointSynchronize::is_at_safepoint(), "concurrent code patching"); 
  // set_destination uses set_long_at which does the ICache::invalidate
  set_destination(dest);
}

// Code for unit testing implementation of NativeCall class
void NativeCall::test() {
#ifdef ASSERT
  ResourceMark rm;
  CodeBuffer* cb = new CodeBuffer(100, 100, 0, 0, 0, false);
  MacroAssembler* a = new MacroAssembler(cb);
  NativeCall  *nc;
  int idx;
  int offsets[] = {
    0x0,
    0xfffffff0,
    0x7ffffff0,
    0x80000000,
    0x20,
    0x4000,
  };

  VM_Version::allow_all();

  a->call( a->pc(), relocInfo::none );
  a->delayed()->nop();
  nc = nativeCall_at( cb->code_begin() );
  nc->print();

  nc = nativeCall_overwriting_at( nc->next_instruction_address() );
  for (idx=0; idx < (sizeof(offsets) / sizeof(int)); idx++)
  {
    nc->set_destination( cb->code_begin() + offsets[idx] );
    assert(nc->destination() == (cb->code_begin() + offsets[idx]), "check unit test");
    nc->print();
  }

  nc = nativeCall_before( cb->code_begin() + 8 );
  nc->print();

  VM_Version::revert();
#endif
}
// End code for unit testing implementation of NativeCall class

//-------------------------------------------------------------------

#ifdef _LP64

void NativeFarCall::set_destination(address dest) { 
  // Address materialized in the instruction stream, so nothing to do.
  return;
#if 0 // What we'd do if we really did want to change the destination
  if (destination() == dest) {
    return;
  }
  ResourceMark rm;
  CodeBuffer* buf = new CodeBuffer(addr_at(0), instruction_size + 1);
  MacroAssembler* _masm = new MacroAssembler(buf);
  // Generate the new sequence
  Address(O7, dest);
  _masm->jumpl_to(dest, O7);
  ICache::invalidate_range(addr_at(0), instruction_size );
#endif
} 

void NativeFarCall::verify() {
  // make sure code pattern is actually a jumpl_to instruction
  assert(instruction_size == NativeJump::instruction_size, "same as jump_to");
  assert(jmpl_offset == NativeMovConstReg::add_offset, "sethi size ok");
  nativeJump_at(addr_at(0))->verify();
}

bool NativeFarCall::is_call_at(address instr) {
  return nativeInstruction_at(instr)->is_sethi();
}

void NativeFarCall::print() {
  tty->print_cr(INTPTR_FORMAT ": call " INTPTR_FORMAT, instruction_address(), destination());
}

bool NativeFarCall::destination_is_compiled_verified_entry_point() {
#ifndef CORE
  nmethod* callee = CodeCache::find_nmethod(destination());
  if (callee == NULL) {
    return false;
  } else {
    return destination() == callee->verified_entry_point();
  }
#else
  return false;
#endif
}

// MT-safe patching of a far call.
void NativeFarCall::replace_mt_safe(address instr_addr, address code_buffer) {
  Unimplemented();
}

// Code for unit testing implementation of NativeFarCall class
void NativeFarCall::test() {
  Unimplemented();
}
// End code for unit testing implementation of NativeFarCall class

#endif // _LP64

//-------------------------------------------------------------------


void NativeMovConstReg::verify() {
  NativeInstruction::verify();
  // make sure code pattern is actually a "set_oop" synthetic instruction
  // see MacroAssembler::set_oop()
  int i0 = long_at(sethi_offset);
  int i1 = long_at(add_offset);

  // verify the pattern "sethi %hi22(imm), reg ;  add reg, %lo10(imm), reg"
  Register rd = inv_rd(i0);
#ifndef _LP64
  if (!(is_op2(i0, Assembler::sethi_op2) && rd != G0 &&
	is_op3(i1, Assembler::add_op3, Assembler::arith_op) &&
	inv_immed(i1) && (unsigned)get_simm13(i1) < (1 << 10) &&
	rd == inv_rs1(i1) && rd == inv_rd(i1))) {
    fatal("not a set_oop");
  }
#else
  if (!is_op2(i0, Assembler::sethi_op2) && rd != G0 ) {
    fatal("not a set_oop");
  }
#endif
}


void NativeMovConstReg::print() {
  tty->print_cr(INTPTR_FORMAT ": mov reg, " INTPTR_FORMAT, instruction_address(), data());
}


#ifdef _LP64
intptr_t NativeMovConstReg::data() const {
  return data64(addr_at(sethi_offset), long_at(add_offset));
}
#else
intptr_t NativeMovConstReg::data() const {
  return data32(long_at(sethi_offset), long_at(add_offset));
}
#endif


void NativeMovConstReg::set_data(intptr_t x) {
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe((address) this);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
#ifdef _LP64
  set_data64_sethi(addr_at(sethi_offset), x);
#else
  set_long_at(sethi_offset, set_data32_sethi(  long_at(sethi_offset), x));
#endif
  set_long_at(add_offset,   set_data32_simm13( long_at(add_offset),   x));

#ifndef CORE
  // also store the value into an oop_Relocation cell, if any
  CodeBlob* nm = CodeCache::find_blob(instruction_address());
  if (nm != NULL) {
    RelocIterator iter(nm, instruction_address(), next_instruction_address());
    oop* oop_addr = NULL;
    while (iter.next()) {
      if (iter.type() == relocInfo::oop_type) {
	oop_Relocation *r = iter.oop_reloc();
	if (oop_addr == NULL) {
	  oop_addr = r->oop_addr();
	  *oop_addr = (oop)x;
	} else {
	  assert(oop_addr == r->oop_addr(), "must be only one set-oop here");
	}
      }
    }
  }
#endif
}


// Code for unit testing implementation of NativeMovConstReg class
void NativeMovConstReg::test() {
#ifdef ASSERT
  ResourceMark rm;
  CodeBuffer* cb = new CodeBuffer(100, 100, 0, 0, 0, false);
  MacroAssembler* a = new MacroAssembler(cb);
  NativeMovConstReg* nm;
  int idx;
  int offsets[] = {
    0x0,
    0x7fffffff,
    0x80000000,
    0xffffffff,
    0x20,
    4096,
    4097,
  };

  VM_Version::allow_all();

  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none);
  a->add(I3, low10(0xaaaabbbb), I3);
  a->sethi(0xccccdddd, O2, true, RelocationHolder::none);
  a->add(O2, low10(0xccccdddd), O2);

  nm = nativeMovConstReg_at( cb->code_begin() );
  nm->print();

  nm = nativeMovConstReg_at( nm->next_instruction_address() );
  for (idx=0; idx < (sizeof(offsets) / sizeof(int)); idx++)
  {
    nm->set_data( offsets[idx] );
    assert(nm->data() == offsets[idx], "check unit test");
  }
  nm->print();

  VM_Version::revert();
#endif
}
// End code for unit testing implementation of NativeMovConstReg class

//-------------------------------------------------------------------

void NativeMovConstRegPatching::verify() {
  NativeInstruction::verify();
  // Make sure code pattern is sethi/nop/add.
  int i0 = long_at(sethi_offset);
  int i1 = long_at(nop_offset);
  int i2 = long_at(add_offset);
  assert(nop_offset == NativeMovConstReg::add_offset, "sethi size ok");


  // Verify the pattern "sethi %hi22(imm), reg; nop; add reg, %lo10(imm), reg"
  // The casual reader should note that on Sparc a nop is a special case if sethi
  // in which the destination register is %g0.
  Register rd0 = inv_rd(i0);
  Register rd1 = inv_rd(i1);
  if (!(is_op2(i0, Assembler::sethi_op2) && rd0 != G0 &&
	is_op2(i1, Assembler::sethi_op2) && rd1 == G0 &&	// nop is a special case of sethi
	is_op3(i2, Assembler::add_op3, Assembler::arith_op) &&
	inv_immed(i2) && (unsigned)get_simm13(i2) < (1 << 10) &&
	rd0 == inv_rs1(i2) && rd0 == inv_rd(i2))) {
    fatal("not a set_oop");
  }
}


void NativeMovConstRegPatching::print() {
  tty->print_cr(INTPTR_FORMAT ": mov reg, " INTPTR_FORMAT, instruction_address(), data());
}


int NativeMovConstRegPatching::data() const {
#ifdef _LP64
  return data64(addr_at(sethi_offset), long_at(add_offset));
#else
  return data32(long_at(sethi_offset), long_at(add_offset));
#endif
}


void NativeMovConstRegPatching::set_data(int x) {
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe((address)this);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
#ifdef _LP64
  set_data64_sethi(addr_at(sethi_offset), x);
#else
  set_long_at(sethi_offset, set_data32_sethi(long_at(sethi_offset), x));
#endif
  set_long_at(add_offset, set_data32_simm13(long_at(add_offset), x));

#ifndef CORE
  // also store the value into an oop_Relocation cell, if any
  CodeBlob* nm = CodeCache::find_blob(instruction_address());
  if (nm != NULL) {
    RelocIterator iter(nm, instruction_address(), next_instruction_address());
    oop* oop_addr = NULL;
    while (iter.next()) {
      if (iter.type() == relocInfo::oop_type) {
	oop_Relocation *r = iter.oop_reloc();
	if (oop_addr == NULL) {
	  oop_addr = r->oop_addr();
	  *oop_addr = (oop)x;
	} else {
	  assert(oop_addr == r->oop_addr(), "must be only one set-oop here");
	}
      }
    }
  }
#endif // !CORE
}


// Code for unit testing implementation of NativeMovConstRegPatching class
void NativeMovConstRegPatching::test() {
#ifdef ASSERT
  ResourceMark rm;
  CodeBuffer* cb = new CodeBuffer(100, 100, 0, 0, 0, false);
  MacroAssembler* a = new MacroAssembler(cb);
  NativeMovConstRegPatching* nm;
  int idx;
  int offsets[] = {
    0x0,
    0x7fffffff,
    0x80000000,
    0xffffffff,
    0x20,
    4096,
    4097,
  };

  VM_Version::allow_all();

  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none);
  a->nop();
  a->add(I3, low10(0xaaaabbbb), I3);
  a->sethi(0xccccdddd, O2, true, RelocationHolder::none);
  a->nop();
  a->add(O2, low10(0xccccdddd), O2);

  nm = nativeMovConstRegPatching_at( cb->code_begin() );
  nm->print();

  nm = nativeMovConstRegPatching_at( nm->next_instruction_address() );
  for (idx=0; idx < (sizeof(offsets) / sizeof(int)); idx++)
  {
    nm->set_data( offsets[idx] );
    assert(nm->data() == offsets[idx], "check unit test");
  }
  nm->print();

  VM_Version::revert();
#endif // ASSERT
}
// End code for unit testing implementation of NativeMovConstRegPatching class


//-------------------------------------------------------------------


void NativeMovRegMem::copy_instruction_to(address new_instruction_address) {
  Untested("copy_instruction_to");
  int instruction_size = next_instruction_address() - instruction_address();
  for (int i = 0; i < instruction_size; i += BytesPerInstWord) {
    *(int*)(new_instruction_address + i) = *(int*)(address(this) + i);
  }
}


void NativeMovRegMem::verify() {
  NativeInstruction::verify();
  // make sure code pattern is actually a "ld" or "st" of some sort.
  int i0 = long_at(0);
  int op3 = inv_op3(i0);

  assert(add_offset == NativeMovConstReg::add_offset, "sethi size ok");

  if (!(is_op(i0, Assembler::ldst_op) &&
	inv_immed(i0) &&
	0 != (op3 < op3_ldst_int_limit
	 ? (1 <<  op3                      ) & (op3_mask_ld  | op3_mask_st)
	 : (1 << (op3 - op3_ldst_int_limit)) & (op3_mask_ldf | op3_mask_stf))))
  {
    int i1 = long_at(ldst_offset);
    Register rd = inv_rd(i0);

    op3 = inv_op3(i1);
    if (!is_op(i1, Assembler::ldst_op) && rd == inv_rs2(i1) && 
	 0 != (op3 < op3_ldst_int_limit
	      ? (1 <<  op3                      ) & (op3_mask_ld  | op3_mask_st)
	       : (1 << (op3 - op3_ldst_int_limit)) & (op3_mask_ldf | op3_mask_stf))) {
      fatal("not a ld* or st* op");
    }
  }
}


void NativeMovRegMem::print() {
  if (is_immediate()) {
    tty->print_cr(INTPTR_FORMAT ": mov reg, [reg + %x]", instruction_address(), offset());
  } else {
    tty->print_cr(INTPTR_FORMAT ": mov reg, [reg + reg]", instruction_address());
  }
}


// Code for unit testing implementation of NativeMovRegMem class
void NativeMovRegMem::test() {
#ifdef ASSERT
  ResourceMark rm;
  CodeBuffer* cb = new CodeBuffer(1000, 1000, 0, 0, 0, false);
  MacroAssembler* a = new MacroAssembler(cb);
  NativeMovRegMem* nm;
  int idx = 0;
  int idx1;
  int offsets[] = {
    0x0,
    0xffffffff,
    0x7fffffff,
    0x80000000,
    4096,
    4097,
    0x20,
    0x4000,
  };

  VM_Version::allow_all();

  a->ldsw( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldsw( G5, I3, G4 ); idx++;
  a->ldsb( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldsb( G5, I3, G4 ); idx++;
  a->ldsh( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldsh( G5, I3, G4 ); idx++;
  a->lduw( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->lduw( G5, I3, G4 ); idx++;
  a->ldub( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldub( G5, I3, G4 ); idx++;
  a->lduh( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->lduh( G5, I3, G4 ); idx++;
  a->ldx( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldx( G5, I3, G4 ); idx++;
  a->ldd( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldd( G5, I3, G4 ); idx++;
  a->ldf( FloatRegisterImpl::D, O2, -1, F14 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldf( FloatRegisterImpl::S, O0, I3, F15 ); idx++;

  a->stw( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->stw( G5, G4, I3 ); idx++;
  a->stb( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->stb( G5, G4, I3 ); idx++;
  a->sth( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->sth( G5, G4, I3 ); idx++;
  a->stx( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->stx( G5, G4, I3 ); idx++;
  a->std( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->std( G5, G4, I3 ); idx++;
  a->stf( FloatRegisterImpl::S, F18, O2, -1 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->add(I3, low10(0xaaaabbbb), I3);
  a->stf( FloatRegisterImpl::S, F15, O0, I3 ); idx++;

  nm = nativeMovRegMem_at( cb->code_begin() );
  nm->print();
  nm->set_offset( low10(0) );
  nm->print();
  nm->add_offset_in_bytes( low10(0xbb) * wordSize );
  nm->print();

  while (--idx) {
    nm = nativeMovRegMem_at( nm->next_instruction_address() );
    nm->print();
    for (idx1 = 0; idx1 < (sizeof(offsets) / sizeof(int)); idx1++)
    {
      nm->set_offset( nm->is_immediate() ? low10(offsets[idx1]) : offsets[idx1] );
      assert(nm->offset() == (nm->is_immediate() ? low10(offsets[idx1]) : offsets[idx1]),
             "check unit test");
      nm->print();
    }
    nm->add_offset_in_bytes( low10(0xbb) * wordSize );
    nm->print();
  }

  VM_Version::revert();
#endif // ASSERT
}

// End code for unit testing implementation of NativeMovRegMem class

//--------------------------------------------------------------------------------


void NativeMovRegMemPatching::copy_instruction_to(address new_instruction_address) {
  Untested("copy_instruction_to");
  int instruction_size = next_instruction_address() - instruction_address();
  for (int i = 0; i < instruction_size; i += wordSize) {
    *(long*)(new_instruction_address + i) = *(long*)(address(this) + i);
  }
}


void NativeMovRegMemPatching::verify() {
  NativeInstruction::verify();
  // make sure code pattern is actually a "ld" or "st" of some sort.
  int i0 = long_at(0);
  int op3 = inv_op3(i0);

  assert(nop_offset == NativeMovConstReg::add_offset, "sethi size ok");

  if (!(is_op(i0, Assembler::ldst_op) &&
	inv_immed(i0) &&
	0 != (op3 < op3_ldst_int_limit
	 ? (1 <<  op3                      ) & (op3_mask_ld  | op3_mask_st)
	 : (1 << (op3 - op3_ldst_int_limit)) & (op3_mask_ldf | op3_mask_stf))))
  {
    int i1 = long_at(ldst_offset);
    Register rd = inv_rd(i0);

    op3 = inv_op3(i1);
    if (!is_op(i1, Assembler::ldst_op) && rd == inv_rs2(i1) && 
	 0 != (op3 < op3_ldst_int_limit
	      ? (1 <<  op3                      ) & (op3_mask_ld  | op3_mask_st)
	       : (1 << (op3 - op3_ldst_int_limit)) & (op3_mask_ldf | op3_mask_stf))) {
      fatal("not a ld* or st* op");
    }
  }
}


void NativeMovRegMemPatching::print() {
  if (is_immediate()) {
    tty->print_cr(INTPTR_FORMAT ": mov reg, [reg + %x]", instruction_address(), offset());
  } else {
    tty->print_cr(INTPTR_FORMAT ": mov reg, [reg + reg]", instruction_address());
  }
}


// Code for unit testing implementation of NativeMovRegMemPatching class
void NativeMovRegMemPatching::test() {
#ifdef ASSERT
  ResourceMark rm;
  CodeBuffer* cb = new CodeBuffer(1000, 1000, 0, 0, 0, false);
  MacroAssembler* a = new MacroAssembler(cb);
  NativeMovRegMemPatching* nm;
  int idx = 0;
  int idx1;
  int offsets[] = {
    0x0,
    0xffffffff,
    0x7fffffff,
    0x80000000,
    4096,
    4097,
    0x20,
    0x4000,
  };

  VM_Version::allow_all();

  a->ldsw( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldsw( G5, I3, G4 ); idx++;
  a->ldsb( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldsb( G5, I3, G4 ); idx++;
  a->ldsh( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldsh( G5, I3, G4 ); idx++;
  a->lduw( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->lduw( G5, I3, G4 ); idx++;
  a->ldub( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldub( G5, I3, G4 ); idx++;
  a->lduh( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->lduh( G5, I3, G4 ); idx++;
  a->ldx( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldx( G5, I3, G4 ); idx++;
  a->ldd( G5, low10(0xffffffff), G4 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldd( G5, I3, G4 ); idx++;
  a->ldf( FloatRegisterImpl::D, O2, -1, F14 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->ldf( FloatRegisterImpl::S, O0, I3, F15 ); idx++;

  a->stw( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->stw( G5, G4, I3 ); idx++;
  a->stb( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->stb( G5, G4, I3 ); idx++;
  a->sth( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->sth( G5, G4, I3 ); idx++;
  a->stx( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->stx( G5, G4, I3 ); idx++;
  a->std( G5, G4, low10(0xffffffff) ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->std( G5, G4, I3 ); idx++;
  a->stf( FloatRegisterImpl::S, F18, O2, -1 ); idx++;
  a->sethi(0xaaaabbbb, I3, true, RelocationHolder::none); a->nop(); a->add(I3, low10(0xaaaabbbb), I3);
  a->stf( FloatRegisterImpl::S, F15, O0, I3 ); idx++;

  nm = nativeMovRegMemPatching_at( cb->code_begin() );
  nm->print();
  nm->set_offset( low10(0) );
  nm->print();
  nm->add_offset_in_bytes( low10(0xbb) * wordSize );
  nm->print();

  while (--idx) {
    nm = nativeMovRegMemPatching_at( nm->next_instruction_address() );
    nm->print();
    for (idx1 = 0; idx1 < (sizeof(offsets) / sizeof(int)); idx1++)
    {
      nm->set_offset( nm->is_immediate() ? low10(offsets[idx1]) : offsets[idx1] );
      assert(nm->offset() == (nm->is_immediate() ? low10(offsets[idx1]) : offsets[idx1]),
             "check unit test");
      nm->print();
    }
    nm->add_offset_in_bytes( low10(0xbb) * wordSize );
    nm->print();
  }

  VM_Version::revert();
#endif // ASSERT
}
// End code for unit testing implementation of NativeMovRegMemPatching class


//--------------------------------------------------------------------------------


void NativeJump::verify() {
  NativeInstruction::verify();
  int i0 = long_at(sethi_offset);
  int i1 = long_at(jmpl_offset);
  assert(jmpl_offset == NativeMovConstReg::add_offset, "sethi size ok");
  // verify the pattern "sethi %hi22(imm), treg ;  jmpl treg, %lo10(imm), lreg"
  Register rd = inv_rd(i0);
#ifndef _LP64
  if (!(is_op2(i0, Assembler::sethi_op2) && rd != G0 &&
	(is_op3(i1, Assembler::jmpl_op3, Assembler::arith_op) ||
	(TraceJumps && is_op3(i1, Assembler::add_op3, Assembler::arith_op))) &&
	inv_immed(i1) && (unsigned)get_simm13(i1) < (1 << 10) &&
	rd == inv_rs1(i1))) {
    fatal("not a jump_to instruction");  
  }
#else
  // In LP64, the jump instruction location varies for non relocatable
  // jumps, for example is could be sethi, xor, jmp instead of the
  // 7 instructions for sethi.  So let's check sethi only.
  if (!is_op2(i0, Assembler::sethi_op2) && rd != G0 ) {
    fatal("not a jump_to instruction");  
  }
#endif
}


void NativeJump::print() {
  tty->print_cr(INTPTR_FORMAT ": jmpl reg, " INTPTR_FORMAT, instruction_address(), jump_destination());
}


// Code for unit testing implementation of NativeJump class
void NativeJump::test() {
#ifdef ASSERT
  ResourceMark rm;
  CodeBuffer* cb = new CodeBuffer(100, 100, 0, 0, 0, false);
  MacroAssembler* a = new MacroAssembler(cb);
  NativeJump* nj;
  int idx;
  int offsets[] = {
    0x0,
    0xffffffff,
    0x7fffffff,
    0x80000000,
    4096,
    4097,
    0x20,
    0x4000,
  };

  VM_Version::allow_all();

  a->sethi(0x7fffbbbb, I3, true, RelocationHolder::none);
  a->jmpl(I3, low10(0x7fffbbbb), G0, RelocationHolder::none);
  a->delayed()->nop();
  a->sethi(0x7fffbbbb, I3, true, RelocationHolder::none);
  a->jmpl(I3, low10(0x7fffbbbb), L3, RelocationHolder::none);
  a->delayed()->nop();

  nj = nativeJump_at( cb->code_begin() );
  nj->print();

  nj = nativeJump_at( nj->next_instruction_address() );
  for (idx=0; idx < (sizeof(offsets) / sizeof(int)); idx++)
  {
    nj->set_jump_destination( nj->instruction_address() + offsets[idx] );
    assert(nj->jump_destination() == (nj->instruction_address() + offsets[idx]), "check unit test");
    nj->print();
  }

  VM_Version::revert();
#endif // ASSERT
}
// End code for unit testing implementation of NativeJump class


void NativeJump::insert(address code_pos, address entry) {
  Unimplemented();
}

// MT safe inserting of a jump over an unknown instruction sequence (used by nmethod::makeZombie)
// The problem: jump_to <dest> is a 3-word instruction (including its delay slot).
// Atomic write can be only with 1 word.
void NativeJump::patch_verified_entry(address entry, address verified_entry, address dest) {
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe(verified_entry);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
  // Here's one way to do it:  Pre-allocate a three-word jump sequence somewhere
  // in the header of the nmethod, within a short branch's span of the patch point.
  // Set up the jump sequence using NativeJump::insert, and then use an annulled
  // unconditional branch at the target site (an atomic 1-word update).
  // Limitations:  You can only patch nmethods, with any given nmethod patched at
  // most once, and the patch must be in the nmethod's header.
  // It's messy, but you can ask the CodeCache for the nmethod containing the
  // target address.

  // %%%%% For now, do something MT-stupid:
  ResourceMark rm;
  int code_size = 1 * BytesPerInstWord;
  CodeBuffer* cb = new CodeBuffer(verified_entry, code_size + 1);
  MacroAssembler* a = new MacroAssembler(cb);
#ifdef COMPILER2
  assert(dest == OptoRuntime::handle_wrong_method_stub(), "The segfault always takes us to OptoRuntime::handle_wrong_method_stub");
#endif
  if (VM_Version::v9_instructions_work()) {
    a->ldsw(G0, 0, O7);	// "ld" must agree with code in the signal handler
  } else {
    a->lduw(G0, 0, O7);	// "ld" must agree with code in the signal handler
  }
  ICache::invalidate_range(verified_entry, code_size);
}


void NativeIllegalInstruction::insert(address code_pos) {
  NativeIllegalInstruction* nii = (NativeIllegalInstruction*) nativeInstruction_at(code_pos);
  nii->set_long_at(0, illegal_instruction());
}

static int illegal_instruction_bits = 0;

int NativeInstruction::illegal_instruction() {
  if (illegal_instruction_bits == 0) {
    ResourceMark rm;
    char buf[40];
    CodeBuffer cbuf((address)&buf[0], 20);
    MacroAssembler* a = new MacroAssembler(&cbuf);
    address ia = a->pc();
    a->trap(ST_RESERVED_FOR_USER_0 + 1);
    int bits = *(int*)ia;
    assert(is_op3(bits, Assembler::trap_op3, Assembler::arith_op), "bad instruction");
    illegal_instruction_bits = bits;
    assert(illegal_instruction_bits != 0, "oops");
  }
  return illegal_instruction_bits;
}

static int ic_miss_trap_bits = 0;

int NativeInstruction::is_ic_miss_trap() {
  if (ic_miss_trap_bits == 0) {
    ResourceMark rm;
    char buf[40];
    CodeBuffer cbuf((address)&buf[0], 20);
    MacroAssembler* a = new MacroAssembler(&cbuf);
    address ia = a->pc();
    a->trapx(Assembler::notEqual,G0,ST_RESERVED_FOR_USER_0 + 2);
    int bits = *(int*)ia;
    assert(is_op3(bits, Assembler::trap_op3, Assembler::arith_op), "bad instruction");
    ic_miss_trap_bits = bits;
    assert(ic_miss_trap_bits != 0, "oops");
  }
  return long_at(0) == ic_miss_trap_bits;
}


bool NativeInstruction::is_illegal() {
  if (illegal_instruction_bits == 0) {
    return false;
  }
  return long_at(0) == illegal_instruction_bits;
}


void NativeGeneralJump::verify() {
  assert(((NativeInstruction *)this)->is_jump() ||
	 ((NativeInstruction *)this)->is_cond_jump(), "not a general jump instruction");
}


void NativeGeneralJump::insert_unconditional(address code_pos, address entry) {
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe(code_pos);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
  Assembler::Condition condition = Assembler::always;
  int x = Assembler::op2(Assembler::br_op2) | Assembler::annul(false) |
    Assembler::cond(condition) | Assembler::wdisp((intptr_t)entry, (intptr_t)code_pos, 22);
  NativeGeneralJump* ni = (NativeGeneralJump*) nativeInstruction_at(code_pos);
  ni->set_long_at(0, x);
}


// MT-safe patching of a jmp instruction (and following word).
// First patches the second word, and then atomicly replaces
// the first word with the first new instruction word.
// Other processors might briefly see the old first word
// followed by the new second word.  This is OK if the old
// second word is harmless, and the new second word may be
// harmlessly executed in the delay slot of the call.
void NativeGeneralJump::replace_mt_safe(address instr_addr, address code_buffer) {
#ifndef CORE
#ifdef ASSERT
  {
    CodeBlob* nm = CodeCache::find_blob_unsafe(instr_addr);
    if (nm != NULL && nm->is_nmethod()) {
      assert(!((nmethod*)nm)->is_patched_for_deopt(), "Can't patch a deoptimized nmethod");
    }
  }
#endif /* ASSERT */
#endif /* CORE */
   assert(Patching_lock->is_locked() ||
         SafepointSynchronize::is_at_safepoint(), "concurrent code patching"); 
   assert (instr_addr != NULL, "illegal address for code patching");
   NativeGeneralJump* h_jump =  nativeGeneralJump_at (instr_addr); // checking that it is a call
   assert(NativeGeneralJump::instruction_size == 8, "wrong instruction size; must be 8");
   int i0 = ((int*)code_buffer)[0];
   int i1 = ((int*)code_buffer)[1];
   int* contention_addr = (int*) h_jump->addr_at(1*BytesPerInstWord);
   assert(inv_op(*contention_addr) == Assembler::arith_op ||
          *contention_addr == nop_instruction() || !VM_Version::v9_instructions_work(), 
          "must not interfere with original call");
   // The set_long_at calls do the ICacheInvalidate so we just need to do them in reverse order
   h_jump->set_long_at(1*BytesPerInstWord, i1);
   h_jump->set_long_at(0*BytesPerInstWord, i0);
   // NOTE:  It is possible that another thread T will execute
   // only the second patched word.
   // In other words, since the original instruction is this
   //    jmp patching_stub; nop                    (NativeGeneralJump)
   // and the new sequence from the buffer is this:
   //    sethi %hi(K), %r; add %r, %lo(K), %r      (NativeMovConstReg)
   // what T will execute is this:
   //    jmp patching_stub; add %r, %lo(K), %r
   // thereby putting garbage into %r before calling the patching stub.
   // This is OK, because the patching stub ignores the value of %r.

   // Make sure the first-patched instruction, which may co-exist
   // briefly with the call, will do something harmless.
   assert(inv_op(*contention_addr) == Assembler::arith_op ||
          *contention_addr == nop_instruction() || !VM_Version::v9_instructions_work(), 
          "must not interfere with original call");
}

