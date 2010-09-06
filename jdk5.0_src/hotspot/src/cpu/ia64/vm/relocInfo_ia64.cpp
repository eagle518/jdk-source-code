#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)relocInfo_ia64.cpp	1.8 03/12/23 16:36:48 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_relocInfo_ia64.cpp.incl"


void Relocation::pd_set_data_value(address x, intptr_t o) {
  NativeInstruction* ni = nativeInstruction_at(addr());
  if( ni->is_movl() ) {
    nativeMovConstReg_at(addr())->set_data(((intptr_t)x) + o);
  }
  else
    ShouldNotReachHere();
}


address Relocation::pd_call_destination() {
  NativeInstruction* ni = nativeInstruction_at(addr());
  if (ni->is_call())
    return nativeCall_at(addr())->destination();
#if 0
  else if (ni->is_jump())
    return nativeJump_at(addr())->jump_destination();
  else if (ni->is_cond_jump())
    return nativeGeneralJump_at(addr())->jump_destination();
  else
#endif  /* excise for now */
    { ShouldNotReachHere(); return NULL; }
}


void Relocation::pd_set_call_destination(address x, intptr_t off) {
  NativeInstruction* ni = nativeInstruction_at(addr());
  if (ni->is_call())
    nativeCall_at(addr())->set_destination(x);
#if 0 /* excise for now */
  else if (ni->is_jump())
    nativeJump_at(addr())->set_jump_destination(x);
  else if (ni->is_cond_jump()) {
    // %%%% kludge this, for now, until we get a jump_destination method
    address old_dest = nativeGeneralJump_at(addr())->jump_destination();
    address disp = Assembler::locate_operand(addr(), Assembler::call32_operand);
    *(jint*)disp += (x - old_dest);
  }
#endif  /* excise for now */
  else
    { ShouldNotReachHere(); }
}


address* Relocation::pd_address_in_code() {
  ShouldNotReachHere();
  return NULL;
}


address Relocation::pd_get_address_from_code() {
  // Only support movl's for now
  NativeInstruction* ni = nativeInstruction_at(addr());
  if( ni->is_movl() ) {
    return (address)(nativeMovConstReg_at(addr())->data());
  }
  ShouldNotReachHere();
  return NULL;
}


int Relocation::pd_breakpoint_size() {
#if 0 /* excise for now */
  // minimum breakpoint size, in short words
  return NativeIllegalInstruction::instruction_size / sizeof(short);
#endif  /* excise for now */
  ShouldNotReachHere();
  return 0;
}

void Relocation::pd_swap_in_breakpoint(address x, short* instrs, int instrlen) {
  Untested("pd_swap_in_breakpoint");
#if 0 /* excise for now */
  if (instrs != NULL) {
    assert(instrlen * sizeof(short) == NativeIllegalInstruction::instruction_size, "enough instrlen in reloc. data");
    for (int i = 0; i < instrlen; i++) {
      instrs[i] = ((short*)x)[i];
    }
  }
  NativeIllegalInstruction::insert(x);
#endif  /* excise for now */
}


void Relocation::pd_swap_out_breakpoint(address x, short* instrs, int instrlen) {
  Untested("pd_swap_out_breakpoint");
#if 0 /* excise for now */
  assert(NativeIllegalInstruction::instruction_size == sizeof(short), "right address unit for update");
  NativeInstruction* ni = nativeInstruction_at(x);
  *(short*)ni->addr_at(0) = instrs[0];
#endif  /* excise for now */
}


