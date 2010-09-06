#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)relocInfo_amd64.cpp	1.5 03/12/23 16:35:54 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_relocInfo_amd64.cpp.incl"


void Relocation::pd_set_data_value(address x, intptr_t o)
{
  x += o;
  typedef Assembler::WhichOperand WhichOperand;
  WhichOperand which = (WhichOperand) format(); // that is, disp32 or imm64
  assert(which == Assembler::disp32_operand ||
         which == Assembler::imm64_operand, "format unpacks ok");
  if (which == Assembler::imm64_operand) {
    *pd_address_in_code() = x;
  } else {
    address disp = Assembler::locate_operand(addr(), which);
    *(int32_t*) disp = x - disp - 4;
  }
}


address Relocation::pd_call_destination()
{
  NativeInstruction* ni = nativeInstruction_at(addr());
  if (ni->is_call()) {
    return nativeCall_at(addr())->destination();
  } else if (ni->is_jump()) {
    return nativeJump_at(addr())->jump_destination();
  } else if (ni->is_cond_jump()) {
    return nativeGeneralJump_at(addr())->jump_destination();
  } else {
    ShouldNotReachHere();
    return NULL;
  }
}


void Relocation::pd_set_call_destination(address x, intptr_t off)
{
  x -= off;
  NativeInstruction* ni = nativeInstruction_at(addr());
  if (ni->is_call()) {
    nativeCall_at(addr())->set_destination(x);
  } else if (ni->is_jump()) {
    NativeJump* nj = nativeJump_at(addr());
    if (nj->jump_destination() == (address) -1) {
      x = (address) -1; // retain jump to self
    }
    nj->set_jump_destination(x);
  } else if (ni->is_cond_jump()) {
    // %%%% kludge this, for now, until we get a jump_destination method
    address old_dest = nativeGeneralJump_at(addr())->jump_destination();
    address disp = Assembler::locate_operand(addr(),
                                             Assembler::call32_operand);
    *(jint*) disp += (x - old_dest);
  } else {
    ShouldNotReachHere();
  }
}


address Relocation::pd_get_address_from_code()
{
  // All embedded Intel addresses are stored in 32-bit words.
  // Since the addr points at the start of the instruction,
  // we must parse the instruction a bit to find the embedded word.
  assert(is_data(), "must be a DataRelocation");
  typedef Assembler::WhichOperand WhichOperand;
  WhichOperand which = (WhichOperand) format(); // that is, disp32 or imm64
  assert(which == Assembler::disp32_operand ||
         which == Assembler::imm64_operand, "format unpacks ok");
  if (which == Assembler::imm64_operand) {
    return *(address*) Assembler::locate_operand(addr(), which);
  } else {
    address disp = Assembler::locate_operand(addr(), which);
    address a = disp + *(int32_t*) disp + 4;
    return a;
  }
}

address* Relocation::pd_address_in_code()
{
  // All embedded Intel addresses are stored in 32-bit words.
  // Since the addr points at the start of the instruction,
  // we must parse the instruction a bit to find the embedded word.
  assert(is_data(), "must be a DataRelocation");
  typedef Assembler::WhichOperand WhichOperand;
  WhichOperand which = (WhichOperand) format(); // that is, disp32 or imm64
  assert(which == Assembler::disp32_operand ||
         which == Assembler::imm64_operand, "format unpacks ok");
  if (which == Assembler::imm64_operand) {
    return (address*) Assembler::locate_operand(addr(), which);
  } else {
    ShouldNotReachHere();
    //    address disp = Assembler::locate_operand(addr(), which);
    //    address a = disp + *(int32_t*) disp + 4;
    //    return &a;
    return NULL;
  }
}


int Relocation::pd_breakpoint_size()
{
  // minimum breakpoint size, in short words
  return NativeIllegalInstruction::instruction_size / sizeof(short);
}

void Relocation::pd_swap_in_breakpoint(address x, short* instrs,
                                       int instrlen)
{
  Untested("pd_swap_in_breakpoint");
  if (instrs != NULL) {
    assert(instrlen * sizeof(short) ==
           NativeIllegalInstruction::instruction_size,
           "enough instrlen in reloc. data");
    for (int i = 0; i < instrlen; i++) {
      instrs[i] = ((short*)x)[i];
    }
  }
  NativeIllegalInstruction::insert(x);
}


void Relocation::pd_swap_out_breakpoint(address x, short* instrs, int instrlen)
{
  Untested("pd_swap_out_breakpoint");
  assert(NativeIllegalInstruction::instruction_size == sizeof(short),
         "right address unit for update");
  NativeInstruction* ni = nativeInstruction_at(x);
  *(short*) ni->addr_at(0) = instrs[0];
}
