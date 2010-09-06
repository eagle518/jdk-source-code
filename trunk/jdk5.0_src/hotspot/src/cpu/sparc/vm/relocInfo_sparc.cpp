#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)relocInfo_sparc.cpp	1.24 03/12/23 16:37:20 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_relocInfo_sparc.cpp.incl"

void Relocation::pd_set_data_value(address x, intptr_t o) {
  NativeInstruction* ip = nativeInstruction_at(addr());
  jint inst = ip->long_at(0);
  assert(inst != NativeInstruction::illegal_instruction(), "no breakpoint");
  switch (Assembler::inv_op(inst)) {

  case Assembler::ldst_op:
    #ifdef ASSERT
      switch (Assembler::inv_op3(inst)) {
        case Assembler::lduw_op3:
        case Assembler::ldub_op3:
        case Assembler::lduh_op3:
        case Assembler::ldd_op3:
        case Assembler::ldsw_op3:
        case Assembler::ldsb_op3:
        case Assembler::ldsh_op3:
        case Assembler::ldx_op3:
        case Assembler::ldf_op3:
        case Assembler::lddf_op3:
        case Assembler::stw_op3:
        case Assembler::stb_op3:
        case Assembler::sth_op3:
        case Assembler::std_op3:
        case Assembler::stx_op3:
        case Assembler::stf_op3:
        case Assembler::stdf_op3:
        case Assembler::casa_op3:
        case Assembler::casxa_op3:
	  break;
        default:
	  ShouldNotReachHere();
      }
      goto do_non_sethi;
    #endif

  case Assembler::arith_op:
    #ifdef ASSERT
      switch (Assembler::inv_op3(inst)) {
        case Assembler::or_op3:
        case Assembler::add_op3:
        case Assembler::jmpl_op3:
	  break;
        default:
	  ShouldNotReachHere();
      }
    do_non_sethi:;
    #endif
    {
    guarantee(Assembler::inv_immed(inst), "must have a simm13 field");
    int simm13 = Assembler::low10((intptr_t)x) + o;
    guarantee(Assembler::is_simm13(simm13), "offset can't overflow simm13");
    inst &= ~Assembler::simm(    -1, 13);
    inst |=  Assembler::simm(simm13, 13);
    ip->set_long_at(0, inst);
    }
    break;

  case Assembler::branch_op:
    {
#ifdef _LP64
    jint inst2;
    guarantee(Assembler::inv_op2(inst)==Assembler::sethi_op2, "must be sethi");
    ip->set_data64_sethi( ip->addr_at(0), (intptr_t)x ); 
#ifdef COMPILER2
    // [RGV] Someone must have missed putting in a reloc entry for the
    // add in compiler2.
    inst2 = ip->long_at( NativeMovConstReg::add_offset );
    guarantee(Assembler::inv_op(inst2)==Assembler::arith_op, "arith op");
    ip->set_long_at(NativeMovConstReg::add_offset,ip->set_data32_simm13( inst2, (intptr_t)x+o));
#endif
#else
    guarantee(Assembler::inv_op2(inst)==Assembler::sethi_op2, "must be sethi");
    inst &= ~Assembler::hi22(     -1);
    inst |=  Assembler::hi22((intptr_t)x);
    // (ignore offset; it doesn't play into the sethi)
    ip->set_long_at(0, inst);
#endif
    }
    break;

  default:
    guarantee(false, "instruction must perform arithmetic or memory access");
  }
}


address Relocation::pd_call_destination() {
  if (NativeCall::is_call_at(addr())) {
    NativeCall* call = nativeCall_at(addr());
    return call->destination();
  }
  if (NativeFarCall::is_call_at(addr())) {
    NativeFarCall* call = nativeFarCall_at(addr());
    return call->destination();
  }
  // Special case:  Patchable branch local to the code cache.
  // This will break badly if the code cache grows larger than a few Mb.
  NativeGeneralJump* br = nativeGeneralJump_at(addr());
  return br->jump_destination();
}


void Relocation::pd_set_call_destination(address x, intptr_t off) {
  x -= off;
  if (NativeCall::is_call_at(addr())) {
    NativeCall* call = nativeCall_at(addr());
    call->set_destination(x);
    return;
  }
  if (NativeFarCall::is_call_at(addr())) {
    NativeFarCall* call = nativeFarCall_at(addr());
    call->set_destination(x);
    return;
  }
  // Special case:  Patchable branch local to the code cache.
  // This will break badly if the code cache grows larger than a few Mb.
  NativeGeneralJump* br = nativeGeneralJump_at(addr());
  br->set_jump_destination(x);
}


address* Relocation::pd_address_in_code() {
  // SPARC never embeds addresses in code, at present.
  //assert(type() == relocInfo::oop_type, "only oops are inlined at present");
  return (address*)addr();
}


address Relocation::pd_get_address_from_code() {
  // SPARC never embeds addresses in code, at present.
  //assert(type() == relocInfo::oop_type, "only oops are inlined at present");
  return *(address*)addr();
}


int Relocation::pd_breakpoint_size() {
  // minimum breakpoint size, in short words
  return NativeIllegalInstruction::instruction_size / sizeof(short);
}

void Relocation::pd_swap_in_breakpoint(address x, short* instrs, int instrlen) {
  Untested("pd_swap_in_breakpoint");
  // %%% probably do not need a general instrlen; just use the trap size
  if (instrs != NULL) {
    assert(instrlen * sizeof(short) == NativeIllegalInstruction::instruction_size, "enough instrlen in reloc. data");
    for (int i = 0; i < instrlen; i++) {
      instrs[i] = ((short*)x)[i];
    }
  }
  NativeIllegalInstruction::insert(x);
}


void Relocation::pd_swap_out_breakpoint(address x, short* instrs, int instrlen) {
  Untested("pd_swap_out_breakpoint");
  assert(instrlen * sizeof(short) == sizeof(int), "enough buf");
  union { int l; short s[1]; } u;
  for (int i = 0; i < instrlen; i++) {
    u.s[i] = instrs[i];
  }
  NativeInstruction* ni = nativeInstruction_at(x);
  ni->set_long_at(0, u.l);
}
