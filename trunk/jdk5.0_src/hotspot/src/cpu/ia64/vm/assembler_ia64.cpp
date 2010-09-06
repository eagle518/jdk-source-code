#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_ia64.cpp	1.73 04/03/08 11:15:09 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_ia64.cpp.incl"

//#define ASMDEBUG 1

//=============================================================================
// IPF_Bundle
//=============================================================================

// Wish we had bit arrays...
const uint8_t IPF_Bundle::bundle_layout[] = {
  // Indexed by catenation of IPF_Bundle::Template and instruction slot index
  Inst::M_unit,    Inst::I_unit,    Inst::I_unit,    Inst::Unit_none, // MII
  Inst::M_unit,    Inst::I_unit,    Inst::I_unit,    Inst::Unit_none, // MIIs
  Inst::M_unit,    Inst::I_unit,    Inst::I_unit,    Inst::Unit_none, // MIsI
  Inst::M_unit,    Inst::I_unit,    Inst::I_unit,    Inst::Unit_none, // MIsIs
  Inst::M_unit,    Inst::L_unit,    Inst::X_unit,    Inst::Unit_none, // MLX
  Inst::M_unit,    Inst::L_unit,    Inst::X_unit,    Inst::Unit_none, // MLXs
  Inst::Unit_none, Inst::Unit_none, Inst::Unit_none, Inst::Unit_none,
  Inst::Unit_none, Inst::Unit_none, Inst::Unit_none, Inst::Unit_none,
  Inst::M_unit,    Inst::M_unit,    Inst::I_unit,    Inst::Unit_none, // MMI
  Inst::M_unit,    Inst::M_unit,    Inst::I_unit,    Inst::Unit_none, // MMsI
  Inst::M_unit,    Inst::M_unit,    Inst::I_unit,    Inst::Unit_none, // MsMI
  Inst::M_unit,    Inst::M_unit,    Inst::I_unit,    Inst::Unit_none, // MsMIs
  Inst::M_unit,    Inst::F_unit,    Inst::I_unit,    Inst::Unit_none, // MFI
  Inst::M_unit,    Inst::F_unit,    Inst::I_unit,    Inst::Unit_none, // MFIs
  Inst::M_unit,    Inst::M_unit,    Inst::F_unit,    Inst::Unit_none, // MMF
  Inst::M_unit,    Inst::M_unit,    Inst::F_unit,    Inst::Unit_none, // MMFs
  Inst::M_unit,    Inst::I_unit,    Inst::B_unit,    Inst::Unit_none, // MIB
  Inst::M_unit,    Inst::I_unit,    Inst::B_unit,    Inst::Unit_none, // MIBs
  Inst::M_unit,    Inst::B_unit,    Inst::B_unit,    Inst::Unit_none, // MBB
  Inst::M_unit,    Inst::B_unit,    Inst::B_unit,    Inst::Unit_none, // MBBs
  Inst::Unit_none, Inst::Unit_none, Inst::Unit_none, Inst::Unit_none,
  Inst::Unit_none, Inst::Unit_none, Inst::Unit_none, Inst::Unit_none,
  Inst::B_unit,    Inst::B_unit,    Inst::B_unit,    Inst::Unit_none, // BBB
  Inst::B_unit,    Inst::B_unit,    Inst::B_unit,    Inst::Unit_none, // BBBs
  Inst::M_unit,    Inst::M_unit,    Inst::B_unit,    Inst::Unit_none, // MMB
  Inst::M_unit,    Inst::M_unit,    Inst::B_unit,    Inst::Unit_none, // MMBs
  Inst::Unit_none, Inst::Unit_none, Inst::Unit_none, Inst::Unit_none,
  Inst::Unit_none, Inst::Unit_none, Inst::Unit_none, Inst::Unit_none,
  Inst::M_unit,    Inst::F_unit,    Inst::B_unit,    Inst::Unit_none, // MFB
  Inst::M_unit,    Inst::F_unit,    Inst::B_unit,    Inst::Unit_none, // MFBs
};

// Wish we had bit arrays...
const uint8_t IPF_Bundle::first_group_length[] = {
  3, // MII
  3, // MIIs
  2, // MIsI
  2, // MIsIs
  3, // MLX
  3, // MLXs
  0,
  0,
  3, // MMI
  3, // MMIs
  1, // MsMI
  1, // MsMIs
  3, // MFI
  3, // MFIs
  3, // MMF
  3, // MMFs
  3, // MIB
  3, // MIBs
  3, // MBB
  3, // MBBs
  0,
  0,
  3, // BBB
  3, // BBBs
  3, // MMB
  3, // MMBs
  0,
  0,
  3, // MFB
  3  // MFBs
};

//=============================================================================
// Displacement
//=============================================================================


//=============================================================================
// Assembler
//=============================================================================


Assembler::Assembler(CodeBuffer *code) : AbstractAssembler(code) {
  FuncDesc* fd = CAST_FROM_FN_PTR(FuncDesc*, &gp_dummy);
  _gp = fd->gp();
}

void Assembler::gp_dummy() {}


// A constant array to invert relation types
const Assembler::Cmp_Relation Assembler::invert_relation[] = {
  Cmp_Relation_Invert
};

// A constant array to reverse relation types
const Assembler::Cmp_Relation Assembler::reverse_relation[] = {
  Cmp_Relation_Reverse
};


void Assembler::cmp_(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                     Cmp_Relation crel, Cmp_Result ctype,
                     uint x2) {
  uint op, ta, c;

  switch (ctype) {
  case None:
  case Unc:
    switch (crel) {
    case notEqual:     swap(p1, p2);
                       // Fallthru
    case equal:        op = 0xE;     break;

    case lessEqual:    swap(p1, p2);
                       // Fallthru
    case greater:      swap(src1, src2);
                       // Fallthru
    case less:         op = 0xC;     break;

    case greaterEqual: swap(p1, p2);
                       op = 0xC;     break;

    case lowerEqual:   swap(p1, p2);
                       // Fallthru
    case higher:       swap(src1, src2);
                       // Fallthru
    case lower:        op = 0xD;     break;

    case higherEqual:  swap(p1, p2);
                       op = 0xD;     break;

    default:           ShouldNotReachHere();
    }

    ta = 0;
    c = ctype; // Note the dependency on None == 0 and Unc == 1
    goto emit_compare;

  // Parallel compare
  case AndCM:   crel = invert_relation[crel];
                // Fallthru
  case And_:    op = 0xC;     break;

  case OrCM:    crel = invert_relation[crel];
                // Fallthru
  case Or_:     op = 0xD;     break;

  case AndOrCM: crel = invert_relation[crel];
                swap(p1, p2);
                // FallThru
  case OrAndCM: op = 0xE;     break;

  default:      ShouldNotReachHere();
  }

  assert(crel == equal || crel == notEqual, "bad crel");

  ta = 1;
  c = crel; // Note the dependency on equal == 0 and notEqual == 1

emit_compare:
  A6 inst(op, x2, 0, ta, c, p1, p2, src1, src2, qp);
  emit(inst);
}


void Assembler::cmp0_(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
                      Cmp_Relation crel, Cmp_Result ctype,
                      uint x2) {
  uint op, ta, c;

  switch (ctype) {

  // Parallel Compare
  case AndCM:   crel = invert_relation[crel];
                // Fallthru
  case And_:    op = 0xC;     break;

  case OrCM:    crel = invert_relation[crel];
                // Fallthru
  case Or_:     op = 0xD;     break;

  case AndOrCM: crel = invert_relation[crel];
                swap(p1, p2);
                // FallThru
  case OrAndCM: op = 0xE;     break;

  default:      ShouldNotReachHere();
  }

  // Operation is 'src crel 0', so we invert the hardware relation.

  switch (crel) {
  case less:         ta = 0; c = 0; break;
  case greaterEqual: ta = 0; c = 1; break;
  case lessEqual:    ta = 1; c = 0; break;
  case greater:      ta = 1; c = 1; break;
  case equal:
  default:           ShouldNotReachHere();
  }

  A7 inst(op, x2, 1, ta, c, p1, p2, src, qp);
  emit(inst);
}


void Assembler::cmp_(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, int imm8, Register src,
                     Cmp_Relation crel, Cmp_Result ctype,
                     uint x2) {
  uint op, ta, c;

  switch (ctype) {
  case None:
  case Unc:
    switch (crel) {
    case notEqual:     swap(p1, p2);
                       // Fallthru
    case equal:        op = 0xE;   break;

    case lessEqual:    imm8 -= 1;
                       // Fallthru
    case less:         op = 0xC;   break;

    case greater:      imm8 -= 1;
                       // Fallthru
    case greaterEqual: swap(p1, p2);
                       op = 0xC;   break;

    case lowerEqual:   imm8 -= 1;
                       // Fallthru
    case lower:        op = 0xD;   break;

    case higher:       imm8 -= 1;
                       // Fallthru
    case higherEqual:  swap(p1, p2);
                       op = 0xD;   break;

    default:           ShouldNotReachHere();
    }

    ta = 0;
    c = ctype; // Note the dependency on None == 0 and Unc == 1
    goto emit_compare;

  // Parallel compare
  case AndCM:   crel = invert_relation[crel];
                // Fallthru
  case And_:    op = 0xC;   break;

  case OrCM:    crel = invert_relation[crel];
                // Fallthru
  case Or_:     op = 0xD;   break;

  case AndOrCM: crel = invert_relation[crel];
                swap(p1, p2);
                // FallThru
  case OrAndCM: op = 0xE;   break;

  default:      ShouldNotReachHere();
  }

  assert(crel == equal || crel == notEqual, "bad crel");

  ta = 1;
  c = crel; // Note the dependency on equal == 0 and notEqual == 1

emit_compare:
  A8 inst(op, x2, ta, c, p1, p2, imm8, src, qp);
  emit(inst);
}


// (qp) fcmp.frel.fctype.sf p1,p2=f2,f3
void Assembler::fcmp(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, FloatRegister src1, FloatRegister src2,
                     Cmp_Relation fcrel, Cmp_Result fctype, FloatStatusField sf) {
  assert(fctype == None || fctype == Unc, "bad fctype");
  uint ra, rb;

  switch (fcrel) {
  case notEqual:     swap(p1, p2);
                     // Fallthru
  case equal:        ra = 0; rb = 0; break;

  case greaterEqual: swap(p1, p2);
                     // Fallthru
  case less:         ra = 0; rb = 1; break;

  case greater:      swap(p1, p2);
                     // Fallthru
  case lessEqual:    ra = 1; rb = 0; break;

  case ordered:      swap(p1, p2);
                     // Fallthru
  case unordered:    ra = 1; rb = 1; break;

  default:           ShouldNotReachHere();
  };

  F4 inst(ra, rb, fctype, sf, p1, p2, src1, src2, qp);
  emit(inst);
}


void Assembler::fclass(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, FloatRegister src, Float_Class fclass,
                       Cmp_Relation fcrel, Cmp_Result fctype) {
  verify_unsigned_range(fclass, 9);
  assert(fctype == None || fctype == Unc, "bad fctype");

  switch (fcrel) {
  case notmember: swap(p1, p2);
  case member:    break;
  default:        ShouldNotReachHere();
  }

  F5 inst(fctype, p1, p2, src, fclass, qp);
  emit(inst);
}


void Assembler::tbit_(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src, uint pos,
		      Cmp_Relation trel, Cmp_Result ctype, uint y) {
  // I-Unit
  assert(trel == equal || trel == notEqual, "bad bit test relation");
  verify_unsigned_range(pos, 6);

  uint c, ta, tb;

  switch (ctype) {
  case None:
  case Unc:     c = ctype; ta = 0; tb = 0;
                if (trel == notEqual) swap(p1, p2);
                break;

  case And_:    c = trel; ta = 0; tb = 1; break;
  case Or_:     c = trel; ta = 1; tb = 0; break;
  case OrAndCM: c = trel; ta = 1; tb = 1; break;

  default:      ShouldNotReachHere();
  }

  I16 inst(ta, tb, y, c, p1, p2, src, pos, qp);
  emit(inst);
}


Inst::ImmedClass Assembler::get_immed_class(IPF_Bundle::Template templt, uint slot, uint41_t inst) {

  // Wish we had bit arrays...
  static uint8_t /* Inst::ImmedClass */ unit_op_to_class [] = {
    /* op                0,         1,   2,   3,         4,         5,   6,         7, */
    /*                   8,         9,   A,   B,         C,         D,   E,         F  */
    /* ------------------------------------------------------------------------------  */
    /* M_unit */ Inst::M22, Inst::I20,   0,   0,         0,         0,   0,         0,
                         0,         0,   0,   0,         0,         0,   0,         0,
    /* F_unit */ Inst::F14,         0,   0,   0,         0,         0,   0,         0,
                         0,         0,   0,   0,         0,         0,   0,         0,
    /* I_unit */ Inst::I20,         0,   0,   0,         0,         0,   0,         0,
                         0,         0,   0,   0,         0,         0,   0,         0,
    /* B_unit */         0,         0,   0,   0, Inst::M22, Inst::M22,   0, Inst::M22,
                         0,         0,   0,   0,         0,         0,   0,         0,
    /* L_unit */         0,         0,   0,   0,         0,         0,   0,         0,
                         0,         0,   0,   0,         0,         0,   0,         0,
    /* X_unit */         0,         0,   0,   0,         0,         0,   Inst::X2,  0,
                         0,         0,   0,   0,  Inst::X3,  Inst::X3,   0,         0
  };

#ifdef ASSERT
  uint op = Inst::inv_u_op(inst);

  switch (IPF_Bundle::get_unit(templt, slot)) {
  case Inst::M_unit: {
    assert(op < 2, "not an M-branch");
    uint x3 = Inst::inv_u_x3(inst);
    if (op == 0) {
      assert(x3 >= 4 && x3 <= 7, "bad M-branch encoding");
    } else {
      assert(x3 == 1 || x3 == 3, "bad M-branch encoding");
    }
    break;
  }
  case Inst::F_unit:
    assert(op == 0, "not an F-branch");
    assert(F_Inst::inv_u_x(inst) == 0 && F_Inst::inv_u_x6a(inst) == 8, "bad F-branch encoding");
    break;
  case Inst::I_unit:
    assert(op == 0, "not an F-branch");
    assert(Inst::inv_u_x3(inst) == 1, "bad I-branch encoding");
    break;
  case Inst::B_unit:
    assert(op == 4 || op == 5 || op == 7, "not a B-branch");
    break;
  case Inst::X_unit:
    assert(op == 6 || op == 0xC || op == 0xD, "not an X-branch or X-mov");
    break;
  case Inst::L_unit:
  default:
    ShouldNotReachHere();
    break;
  }
#endif // ASSERT

  return Inst::ImmedClass(unit_op_to_class[(IPF_Bundle::get_unit(templt, slot) << 4) | Inst::inv_u_op(inst)]);
}


// Construct a version of the branch instruction at bundle offset inst_offset
// described by class/inst whose displacement refers to dest_offset.
//
void Assembler::patched_branch(int64_t dest_offset,
                               Inst::ImmedClass dclass, uint41_t inst,
                               int64_t inst_offset,
                               uint41_t& new_inst,
                               uint41_t& new_L) {

  uint64_t disp = dest_offset - inst_offset;

  switch (dclass) {
  case Inst::I20:
    I20::set_target(disp, inst, new_inst);
    break;
  case Inst::M22:
    M22::set_target(disp, inst, new_inst);
    break;
  case Inst::F14:
    F14::set_target(disp, inst, new_inst);
    break;
  case Inst::X2:
    X2::set_imm(dest_offset, inst, new_inst, new_L);
    break;
  case Inst::X3:
    X3::set_target(disp, inst, new_inst, new_L);
    break;
  default:
    ShouldNotReachHere();
    break;
  }
}


// Return the offset of the destination of the branch instruction located
// in the bundle at offset described by class/inst.
//
int64_t Assembler::branch_destination(Inst::ImmedClass dclass, uint41_t inst,
                                      int64_t offset, uint41_t L) {
  switch (dclass) {
  case Inst::I20:
    return offset + I20::inv_target(inst);
  case Inst::M22:
    return offset + M22::inv_target(inst);
  case Inst::F14:
    return offset + F14::inv_target(inst);
  case Inst::X3:
    return offset + X3::inv_target(inst, L);
  default:
    ShouldNotReachHere();
  }
  return 0;
}

int AbstractAssembler::code_fill_byte() {
  return 0x00;                  // illegal instruction 0x00000000
}

// Given a Label, return a branch displacement that it refers to. If L is
// bound, the displacement is from the current bundle to the bundle to which L
// is bound, stripped of the lower 4 bits (which are always 0).  If L is not
// bound and has been referenced, the displacement is an encoding that refers
// to the bundle that contains the instruction that last referenced L.
// In this case, the address may contain a slot index in its lower two bits.
//
int CodeBuffer::encoded_target(Label *L) {
  // If L->unused(), L->pos() is undefined.

  // If the label is bound, then just return the 25 bit displacement
  if( L->is_bound() )
    return (L->pos() - code_size());

  // If the label is unused, then initialize it as pointing to itself
  int current_pos = code_size() | code_slot();

  if( L->is_unused() ) {
    L->link_to( current_pos );
    return 0;
  }

  // Previously used.  Don't drop the slot index from the displacement!
  int resultDelta = (L->pos() - current_pos) << 4;
  L->link_to( current_pos );
  return resultDelta;
}


//=============================================================================
// MacroAssembler
//=============================================================================


void MacroAssembler::fdivs(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  Unimplemented();
}
void MacroAssembler::fdivs(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fdivs(PR0, dst, src1, src2);
}


void MacroAssembler::fdivd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                           FloatRegister tmp3, FloatRegister tmp4, PredicateRegister failed) {
  // Maximum Throughput
  frcpa(qp, dst, failed, src1, src2, sf0);     // dst = approx(1.0/src2)
  fnma (failed, tmp3, src2, dst,  src1, sf1);  // tmp3 = 1.0 - src2 * dst
  fma  (failed, dst,  tmp3, dst,  dst,  sf1);  // dst  = tmp3 * dst + dst
  fmpy (failed, tmp3, tmp3, tmp3,       sf1);  // tmp3 = tmp3**2
  fma  (failed, dst,  tmp3, dst,  dst,  sf1);  // dst  = tmp3 * dst + dst
  fmpy (failed, tmp3, tmp3, tmp3,       sf1);  // tmp3 = tmp3**2
  fma  (failed, dst,  tmp3, dst,  dst,  sf1);  // dst  = tmp3 * dst + dst
  fmpyd(failed, tmp3, src1, dst,        sf1);  // tmp3 = src1 * dst
  fnmad(failed, tmp4, src2, tmp3, src1, sf1);  // tmp4 = src1 - src1 * src2 * dst
  fmad (failed, dst,  tmp4, dst,  tmp3, sf1);  // dst  = src1 * ((1 - src2 * dst) * dst + dst)
}
void MacroAssembler::fdivd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                           FloatRegister tmp3, FloatRegister tmp4, PredicateRegister failed) {
  fdivd(PR0, dst, src1, src2, tmp3, tmp4, failed);
}


//   Note that only lower 32 bits are good unless src1 and src2 are
//   sign extended to 64 bits
void MacroAssembler::impy(PredicateRegister qp, Register dst, Register src1, Register src2,
                          FloatRegister tmp1, FloatRegister tmp2) {
  setfsig(qp, tmp1, src1);
  setfsig(qp, tmp2, src2);
  xmpy   (qp, tmp1, tmp2, tmp1);
  getfsig(qp, dst,  tmp1);
}
void MacroAssembler::impy(                      Register dst, Register src1, Register src2,
                          FloatRegister tmp1, FloatRegister tmp2) {
  impy(PR0, dst, src1, src2, tmp1, tmp2);
}


void MacroAssembler::stop(const char* msg) {
}


void MacroAssembler::call_VM_leaf_base(address entry_point) {
  mov(GR_Lsave_GP, GP);

  // Get target address
  address target = ((FuncDesc*)entry_point)->entry();

  mova(GR_RET, target);

  mov(BR6_SCRATCH, GR_RET);
  call(BR6_SCRATCH);

  mov(GP, GR_Lsave_GP);
}


void MacroAssembler::call_VM_leaf(address entry_point) {
  call_VM_leaf_base(entry_point);
}

void MacroAssembler::call_VM_leaf(address entry_point, Register arg_1) {
  mov(GR_O0, arg_1);
  call_VM_leaf(entry_point);
}

void MacroAssembler::call_VM_leaf(address entry_point, Register arg_1, Register arg_2) {
  mov(GR_O0, arg_1);
  mov(GR_O1, arg_2); assert(arg_2 != GR_O0, "smashed argument");
  call_VM_leaf(entry_point);
}

void MacroAssembler::call_VM_leaf(address entry_point, Register arg_1, Register arg_2, Register arg_3) {
  mov(GR_O0, arg_1);
  mov(GR_O1, arg_2); assert(arg_2 != GR_O0,                   "smashed argument");
  mov(GR_O2, arg_3); assert(arg_3 != GR_O0 && arg_3 != GR_O1, "smashed argument");
  call_VM_leaf(entry_point);
}


void MacroAssembler::get_vm_result(Register oop_result) {
  const Register vm_result_addr = GR2_SCRATCH;
//verify_thread();
  add(vm_result_addr, thread_(vm_result));
  ld8(oop_result, vm_result_addr);
  st8(vm_result_addr, GR0);
//verify_oop(oop_result);
}

void MacroAssembler::get_vm_result_2(Register oop_result) {
  const Register vm_result_2_addr = GR2_SCRATCH;
//verify_thread();
  add(vm_result_2_addr, thread_(vm_result_2));
  ld8(oop_result, vm_result_2_addr);
  st8(vm_result_2_addr, GR0);
//verify_oop(oop_result);
}

//
// Save all scratch registers to the stack
// Preserves but uses GR2, GR3, PR6
// Allocates additional 16 bytes of ABI Scratch area
// At bottom of stack.

void MacroAssembler::push_scratch_regs(void) {

  // Save everything that is volatile using no registers
  const int  frame_size = wordSize*push_regs_size;
  const PredicateRegister is_notnat = PR6_SCRATCH;

  // Allocate space for the registers
  sub(SP, SP, frame_size-(gr2_off*wordSize)); // Set SP at GR2 save location
  st8spill(SP, GR2);
  add(GR2, SP, 8);
  sub(SP, SP, (gr2_off*wordSize));            // Allocate ABI Scratch area
  st8spill(GR2, GR3, 8);
  st8spill(GR2, GR8, 8);
  st8spill(GR2, GR9, 8);
  st8spill(GR2, GR10, 8);
  st8spill(GR2, GR11, 8);
  st8spill(GR2, GR14, 8);
  st8spill(GR2, GR15, 8);
  st8spill(GR2, GR16, 8);
  st8spill(GR2, GR17, 8);
  st8spill(GR2, GR18, 8);
  st8spill(GR2, GR19, 8);
  st8spill(GR2, GR20, 8);
  st8spill(GR2, GR21, 8);
  st8spill(GR2, GR22, 8);
  st8spill(GR2, GR23, 8);
  st8spill(GR2, GR24, 8);
  st8spill(GR2, GR25, 8);
  st8spill(GR2, GR26, 8);
  st8spill(GR2, GR27, 8);
  st8spill(GR2, GR28, 8);
  st8spill(GR2, GR29, 8);
  st8spill(GR2, GR30, 8);
  st8spill(GR2, GR31, 8);

  // Float registers spills are 16 bytes long

  stfspill(GR2, FR6,  16);
  stfspill(GR2, FR7,  16);
  stfspill(GR2, FR8,  16);
  stfspill(GR2, FR9,  16);
  stfspill(GR2, FR10, 16);
  stfspill(GR2, FR11, 16);
  stfspill(GR2, FR12, 16);
  stfspill(GR2, FR13, 16);
  stfspill(GR2, FR14, 16);
  stfspill(GR2, FR15, 16);

  mov(GR3, BR6);
  st8spill(GR2, GR3, 8);  // BR6
  mov(GR3, BR7);
  st8spill(GR2, GR3, 8);  // BR7
  mov_from_pr(GR3);       // Get All the predicates
  st8spill(GR2, GR3, 16); // PR and skip pad

  // Store volatile floats as doubles for deopt
  fclass(PR0, PR0, is_notnat, FR6, NatVal, member);
  stfd(is_notnat, GR2, FR6, 8);
  fclass(PR0, PR0, is_notnat, FR7, NatVal, member);
  stfd(is_notnat, GR2, FR7, 8);
  fclass(PR0, PR0, is_notnat, FR8, NatVal, member);
  stfd(is_notnat, GR2, FR8, 8);
  fclass(PR0, PR0, is_notnat, FR9, NatVal, member);
  stfd(is_notnat, GR2, FR9, 8);
  fclass(PR0, PR0, is_notnat, FR10, NatVal, member);
  stfd(is_notnat, GR2, FR10, 8);
  fclass(PR0, PR0, is_notnat, FR11, NatVal, member);
  stfd(is_notnat, GR2, FR11, 8);
  fclass(PR0, PR0, is_notnat, FR12, NatVal, member);
  stfd(is_notnat, GR2, FR12, 8);
  fclass(PR0, PR0, is_notnat, FR13, NatVal, member);
  stfd(is_notnat, GR2, FR13, 8);
  fclass(PR0, PR0, is_notnat, FR14, NatVal, member);
  stfd(is_notnat, GR2, FR14, 8);
  fclass(PR0, PR0, is_notnat, FR15, NatVal, member);
  stfd(is_notnat, GR2, FR15);

  // Java doesn't use FR32-FR127 (yet) so no need to save
}

void MacroAssembler::pop_scratch_regs() {

  const int  frame_size = wordSize*push_regs_size;

  // Restore the volatiles

  add(GR2, SP, (gr8_off*wordSize)); 
  ld8fill(GR8,  GR2, 8);
  ld8fill(GR9,  GR2, 8);
  ld8fill(GR10, GR2, 8);
  ld8fill(GR11, GR2, 8);
  ld8fill(GR14, GR2, 8);
  ld8fill(GR15, GR2, 8);
  ld8fill(GR16, GR2, 8);
  ld8fill(GR17, GR2, 8);
  ld8fill(GR18, GR2, 8);
  ld8fill(GR19, GR2, 8);
  ld8fill(GR20, GR2, 8);
  ld8fill(GR21, GR2, 8);
  ld8fill(GR22, GR2, 8);
  ld8fill(GR23, GR2, 8);
  ld8fill(GR24, GR2, 8);
  ld8fill(GR25, GR2, 8);
  ld8fill(GR26, GR2, 8);
  ld8fill(GR27, GR2, 8);
  ld8fill(GR28, GR2, 8);
  ld8fill(GR29, GR2, 8);
  ld8fill(GR30, GR2, 8);
  ld8fill(GR31, GR2, 8);

  ldffill(FR6,  GR2, 16);
  ldffill(FR7,  GR2, 16);
  ldffill(FR8,  GR2, 16);
  ldffill(FR9,  GR2, 16);
  ldffill(FR10, GR2, 16);
  ldffill(FR11, GR2, 16);
  ldffill(FR12, GR2, 16);
  ldffill(FR13, GR2, 16);
  ldffill(FR14, GR2, 16);
  ldffill(FR15, GR2, 16);

  // Restore the Branch and Predicate registers

  ld8fill(GR3, GR2, 8);  // BR6
  mov(BR6, GR3);
  ld8fill(GR3, GR2, 8);  // BR7
  mov(BR7, GR3);
  ld8fill(GR3, GR2);     // PR
  mov_to_pr(GR3);        

  // Now restore GR2,GR3

  add(GR3, SP, (gr2_off*wordSize)); 
  ld8fill(GR2,  GR3, 8);
  ld8fill(GR3,  GR3);
  
  // Remove stack frame
  add(SP, SP, frame_size); 
}


void MacroAssembler::os_breakpoint(void) {

  // Call the breakpoint routine
  // Need a window to prevent os::breakpoint from changing our output registers
  push_dummy_full_frame(GR0);  
  push_scratch_regs();

  // Get target address
  address target = CAST_FROM_FN_PTR(address, os::breakpoint);
  address target2 = ((FuncDesc*)target)->entry();

  mova(GR_RET, target2);

  mov(BR6_SCRATCH, GR_RET);
  call(BR6_SCRATCH);

  // Restore the volatiles
  pop_scratch_regs();
  pop_dummy_full_frame();  
}


// Setup everything we need in the thread to identify the last Java frame.
// Note (1) the pc we use is not any actual return address. None of the
// current uses require anything other than "close" (i.e. identify the code
// buffer).  Should a more exact location be need this should be specialized.
// Note (2) We don't have to clear the flushed_windows state here because
// it starts clear (false) and whenever we return (via reset_last_Java_frame)
// it is reset.
//
int MacroAssembler::set_last_Java_frame(const Register last_java_sp) {
  int last_Java_pc_offset;

  const Register last_Java_fp_addr     = GR2_SCRATCH;
  const Register last_Java_fp          = GR3_SCRATCH;

  const Register last_Java_pc_addr     = GR2_SCRATCH;
  const Register last_Java_pc          = GR3_SCRATCH;

  const Register flags_addr            = GR2_SCRATCH;
  const Register flags                 = GR2_SCRATCH;

  const Register last_Java_sp_addr     = GR3_SCRATCH;

  add(last_Java_fp_addr, GR4_thread, in_bytes(JavaThread::last_Java_fp_offset()));
  movm(last_Java_fp, AR_BSP);

  st8(last_Java_fp_addr, last_Java_fp);
  add(last_Java_pc_addr, GR4_thread, in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()));

  flush_bundle();
  last_Java_pc_offset = offset();
  mov_from_ip(last_Java_pc);
  flush_bundle();

  st8(last_Java_pc_addr, last_Java_pc);
#ifdef ASSERT
  {
    // Verify that the flag word is not corrupt with leftovers
    Label skip;
    add(flags_addr, GR4_thread, in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()));
    ld4(flags, flags_addr);
    cmp(PR6, PR0, GR0, flags, Assembler::equal);
    br(PR6, skip);
    stop("flag word invalid");
    bind(skip);
  }
#endif /* ASSERT */

  add(last_Java_sp_addr, GR4_thread, in_bytes(JavaThread::last_Java_sp_offset()));
  st8(last_Java_sp_addr, last_java_sp, Assembler::ordered_release);

  return last_Java_pc_offset;
}

// Setup everything we need in the thread to identify the last Java frame.
// Caller specifies all the important data.
//
void MacroAssembler::set_last_Java_frame(const Register last_java_sp, 
                                       const Register last_java_fp,
                                       const Register last_java_pc) {

  const Register last_addr_1   = GR2_SCRATCH;
  const Register last_addr_2   = GR3_SCRATCH;

  add(last_addr_1, GR4_thread,   in_bytes(JavaThread::last_Java_fp_offset()));
  add(last_addr_2, GR4_thread,   in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()));

  st8(last_addr_1, last_java_fp, in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()) -
                                 in_bytes(JavaThread::last_Java_fp_offset()));
  st8(last_addr_2, last_java_pc, in_bytes(JavaThread::last_Java_sp_offset()) -
                                 in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()));

  // Windows are unflushed
  st4(last_addr_1, GR0, Assembler::ordered_release);
  // We now have a last java frame
  st8(last_addr_2, last_java_sp, Assembler::ordered_release);


}

void MacroAssembler::reset_last_Java_frame() {
  const Register last_Java_fp_addr     = GR2_SCRATCH;
  const Register last_Java_sp_addr     = GR3_SCRATCH;
  const Register last_Java_pc_addr     = GR2_SCRATCH;
  const Register flags_addr            = GR3_SCRATCH;

  add(last_Java_sp_addr, GR4_thread, in_bytes(JavaThread::last_Java_sp_offset()));
  add(last_Java_fp_addr, GR4_thread, in_bytes(JavaThread::last_Java_fp_offset()));

  st8(last_Java_sp_addr, GR0, Assembler::ordered_release);
  mf();
  st8(last_Java_fp_addr, GR0);

  add(last_Java_pc_addr, GR4_thread, in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::last_Java_pc_offset()));
  add(flags_addr, GR4_thread, in_bytes(JavaThread::frame_anchor_offset() + JavaFrameAnchor::flags_offset()));

  st8(last_Java_pc_addr, GR0);
  st4(flags_addr, GR0, Assembler::ordered_release);		/* windows not flushed */
}


int MacroAssembler::call_VM_base(
  Register        oop_result,
  Register        java_thread_cache,
  Register        last_java_sp,
  address         entry_point,
  bool            check_exception)
{
  int last_Java_pc_offset;

  if (!last_java_sp->is_valid()) {
    // default to current SP
    last_java_sp = SP;
  }

  last_Java_pc_offset = set_last_Java_frame(last_java_sp);

  // Pass thread as first argument
  mov(GR_O0, GR4_thread);

  call_VM_leaf_base(entry_point);

  reset_last_Java_frame();

  // Check for pending exceptions
  if (check_exception) {
//  ShouldNotReachHere();
  }

  // Get oop result if there is one and reset the value in the thread
  if (oop_result->is_valid()) {
    get_vm_result(oop_result);
  }

  return last_Java_pc_offset;
}


void MacroAssembler::call_VM(Register oop_result, address entry_point, bool check_exceptions) {
  (void)call_VM_base(oop_result, noreg, noreg, entry_point, check_exceptions);
}

void MacroAssembler::call_VM(Register oop_result, address entry_point, Register arg_1, bool check_exceptions) {
  // GR_O0 is reserved for the thread
  mov(GR_O1, arg_1);
  call_VM(oop_result, entry_point, check_exceptions);
}

void MacroAssembler::call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, bool check_exceptions) {
  // GR_O0 is reserved for the thread
  mov(GR_O1, arg_1);
  mov(GR_O2, arg_2); assert(arg_2 != GR_O1, "smashed argument");
  call_VM(oop_result, entry_point, check_exceptions);
}

void MacroAssembler::call_VM(Register oop_result, address entry_point, Register arg_1, Register arg_2, Register arg_3, bool check_exceptions) {
  // GR_O0 is reserved for the thread
  mov(GR_O1, arg_1);
  mov(GR_O2, arg_2); assert(arg_2 != GR_O1,                   "smashed argument");
  mov(GR_O3, arg_3); assert(arg_3 != GR_O1 && arg_3 != GR_O2, "smashed argument");
  call_VM(oop_result, entry_point, check_exceptions);
}


int MacroAssembler::call_VM_for_oopmap(Register oop_result, address entry_point, bool check_exceptions) {
  return call_VM_base(oop_result, noreg, noreg, entry_point, check_exceptions);
}


void MacroAssembler::return_from_dummy_frame() {
  const Register       return_addr = GR2_SCRATCH;
  const BranchRegister return_br   = BR6_SCRATCH;

  flush_bundle();
  mov_from_ip(return_addr);
  flush_bundle();

  adds(return_addr, sizeof(IPF_Bundle)*4, return_addr);
  flush_bundle();

  movret(return_br, return_addr);
  flush_bundle();

  // Return to next bundle
  ret(return_br);
}


void MacroAssembler::push_thin_frame() {
  alloc(GR_Lsave_PFS, 8, 16, 8, 0);
  mov(GR_Lsave_caller_BSP, GR6_caller_BSP); 
  mov(GR6_caller_BSP, AR_BSP); 
  mov(GR_Lsave_SP, SP);
  mov(GR_Lsave_RP, RP);
}

void MacroAssembler::pop_thin_frame() {
  mov(GR6_caller_BSP, GR_Lsave_caller_BSP); 
  movret(RP, GR_Lsave_RP);
  mov(SP, GR_Lsave_SP);
  mov(AR_PFS, GR_Lsave_PFS);
}

void MacroAssembler::pop_dummy_thin_frame() {
  pop_thin_frame();
  return_from_dummy_frame();
}


void MacroAssembler::push_full_frame() {
  push_thin_frame();
  mov(GR_Lsave_LC, AR_LC);
  mov(GR_Lsave_UNAT, AR_UNAT);
}

void MacroAssembler::save_full_frame(const uint frame_size_in_bytes) {
  push_full_frame();
  assert((frame_size_in_bytes & 0xF) == 0, "stack must be aligned");
  sub(SP, SP, frame_size_in_bytes);
}

void MacroAssembler::pop_full_to_thin_frame() {
  mov(AR_UNAT, GR_Lsave_UNAT);
  mov(AR_LC, GR_Lsave_LC);
}

void MacroAssembler::pop_full_frame() {
  pop_full_to_thin_frame();
  pop_thin_frame();
}


void MacroAssembler::push_dummy_thin_frame(const Register fake_RP) {
  Label next;

  call(next);
  bind(next);
  mov(RP, fake_RP);
  push_thin_frame();
}

void MacroAssembler::push_dummy_full_frame(const Register fake_RP) {
  Label next;

  call(next);
  bind(next);
  mov(RP, fake_RP);
  push_full_frame();
}

void MacroAssembler::pop_dummy_full_frame() {
  pop_full_frame();
  return_from_dummy_frame();
}



//=============================================================================
// IPF Bundler
//=============================================================================

// The following file is generated, and contains
// information on all the legal bundle configurations
#include "bundle_itanium.hpp"

// Array of NOP encodings for the bundles
static const uint41_t unit_nop_encoding[Inst::Unit_count] = {
  CONST64(0x00008000000), // M_unit
  CONST64(0x00008000000), // F_unit
  CONST64(0x00008000000), // I_unit
  CONST64(0x04000000000), // B_unit
  CONST64(0x00000000000), // L_unit
  CONST64(0x00008000000)  // X_unit
};

#ifndef PRODUCT
static const char* kind_string[Inst::Kind_count] = {
    "M", "F", "I", "B", "L", "X", "A" };
#endif

volatile int gottlieb = 0;

// Find the index for a set of bundles that can encode the set
// of instructions; return 0 if no such bundle set exists
uint CodeBuffer::FindBundleSet(Inst &inst) {

#if !defined(PRODUCT) && defined(ASMDEBUG)
  {
    if (gottlieb == 0) {
      printf("Waiting on `gottlieb` variable spin-lock\n");
      while (gottlieb == 0);
    }
  }

  printf("-> FindBundleSet: \"%s\"\n", kind_string[inst.kind()]);
#endif

  Inst::Kind k = inst.kind();

  // If this is the first instruction, then it is ok
  if (_inst_count > 0) {

    // If all the instruction slots are taken, then no
    if (_inst_count >= MaxInsts)
      return 0;

    // Do this only if checking is enabled
    if( !_explicit_bundling ) {

      // If any of the registers written by this instruction are read in
      // the bundle, then force a new bundle.
      if (_current_read.overlaps(inst.writeState(), false, false))
        return 0;

      // If any of the registers read by this instruction are written in
      // the bundle, then force a new bundle.
      //
      // Note: branch instructions that read predicate registers
      //   are NOT reflected in this, because we permit branch
      //   instructions to read predicate registers in the
      //   same cycle that they are written
      if (_current_write.overlaps(inst.readState(),
            inst.pr_fast_read() && is_pr_fast_write_only(),
            inst.br_fast_read() && is_br_fast_write_only()))
        return 0;

      // If any of the registers written by this instruction are written in
      // the bundle, then force a new bundle.
      if (_current_write.overlaps(inst.writeState(), false, false))
        return 0;
    }
  }

  // Increment (temporarily) for the proposed instruction
  uint limit = ++_accum_kind_count[k];

#if !defined(PRODUCT) && defined(ASMDEBUG)
printf("  LX: %d, B: %d, F: %d, I: %d, M: %d, A: %d\n",
  _accum_kind_count[Inst::X_inst],
  _accum_kind_count[Inst::B_inst],
  _accum_kind_count[Inst::F_inst],
  _accum_kind_count[Inst::I_inst],
  _accum_kind_count[Inst::M_inst],
  _accum_kind_count[Inst::A_inst]);
#endif

  // Look up based on all the instructions accumulated, and see if
  // a set of bundles exists that can encode this set.
  uint ndx = 0;

  if( limit <= limit_table[k] )
    ndx = emit_table[_accum_kind_count[Inst::X_inst]]
                    [_accum_kind_count[Inst::B_inst]]
                    [_accum_kind_count[Inst::F_inst]]
                    [_accum_kind_count[Inst::I_inst]]
                    [_accum_kind_count[Inst::M_inst]]
                    [_accum_kind_count[Inst::A_inst]][0];

#if !defined(PRODUCT) && defined(ASMDEBUG)
printf("<- FindBundleSet: %d\n", ndx);
#endif

  if (!ndx)
    // No solution found: back out the kind for this instruction
    --_accum_kind_count[k];

  // Zero means no such bundle set exists
  return ndx;
}


void CodeBuffer::emit(Inst& inst) {
  // Insert this double instruction into an instruction group, flushing the
  // current instruction group if necessary
  uint x = insert(inst);

  BundleInst &element = _element[x];

  element.set(inst.bits(), inst.kind(), inst.is_ordered(), _inst_count++);
}

void CodeBuffer::emit(Inst& inst, Label *target) {
  // Insert this instruction into an instruction group, flushing the
  // current instruction group if necessary
  uint x = insert(inst);

  BundleInst &element = _element[x];

  element.set(inst.bits(), inst.kind(), inst.is_ordered(), _inst_count++);
  element.set_label(target);
}

void CodeBuffer::emit(Inst& inst, address target, relocInfo::relocType rtype) {
  // Insert this instruction into an instruction group, flushing the
  // current instruction group if necessary
  uint x = insert(inst);

  BundleInst &element = _element[x];

  element.set(inst.bits(), inst.kind(), inst.is_ordered(), _inst_count++);
  element.set_addr(target);
  element.set_reloc(rtype);
}

void CodeBuffer::emit(X_Inst& inst) {
  // Insert this double instruction into an instruction group, flushing the
  // current instruction group if necessary
  uint x = insert(inst);

  BundleInst &element1 = _element[x];
  BundleInst &element2 = _element[x+1];

  element1.set(inst.L(),    Inst::L_inst, inst.is_ordered(), _inst_count++);
  element2.set(inst.bits(), Inst::X_inst, inst.is_ordered(), _inst_count++);
}

void CodeBuffer::emit(X_Inst& inst, Label* target) {
  // Long branches not yet implemented
  ShouldNotReachHere();
}

void CodeBuffer::emit(X_Inst& inst, address target, relocInfo::relocType rtype) {
  // Insert this double instruction into an instruction group, flushing the
  // current instruction group if necessary
  uint x = insert(inst);

  BundleInst &element1 = _element[x];
  BundleInst &element2 = _element[x+1];

  element1.set(inst.L(),    Inst::L_inst, inst.is_ordered(), _inst_count++);
  element2.set(inst.bits(), Inst::X_inst, inst.is_ordered(), _inst_count++);

  element1.set_addr(target);
  element1.set_reloc(rtype);
}

// Return the index in the arrays of the new instruction
uint CodeBuffer::insert(Inst& inst) {

  uint x, y;

  // check to see if there are any ordered instructions
  if( is_ordered() ) {

    // If this instruction is not ordered, then the preceeding
    // ordered instruction(s) must occur first (oversimplification)
    if( !inst.is_ordered() )
      flush_bundle(false);

    // If this is not a B-type branch, then flush everything else first
    // because B-types are always at the end (too hard for anything else)
    else if ( inst.kind() != Inst::B_inst )
      flush_bundle(false);
  }

  // Set if there is a bundle set for this instruction
  uint bundle_index = FindBundleSet(inst);

  // If a bundle set is not found, then update the current bundle state
  // to start a new bundle
  if (!bundle_index) {

    // Flush the accumulated instructions
    flush_bundle(false);

    // try again, with only the new instruction
    bundle_index = FindBundleSet(inst);

    assert(bundle_index != 0, "no encoding for instruction found");
  }

  // Add the instruction to the current state; insert the instruction
  // based on the sorted order of the instruction kinds
  Inst::Kind kind = inst.kind();

  // Locate the correct position in the list to insert the instruction
  for ( x = 0; x < _inst_count; x++ ) {
    Inst::Kind current = _element[x].kind();
    if( current > kind )
      break;
    else if( current == Inst::L_inst )
      x++;
  }

  // Move all the elements after this one forward one element
  for ( y = _inst_count; y > x; y-- )
    _element[y] = _element[y-1];

  // If this is ordered then propagate to the instruction group
  if( inst.is_ordered() )
    set_is_ordered();

  // Update the register state
  _current_read. or_mask(inst.readState());
  _current_write.or_mask(inst.writeState());

  // Note if this instruction permits a write and read of the
  // same pr in the same instruction.
  if( inst.writeState().is_set_pr() && !inst.pr_fast_write() )
    clear_is_pr_fast_write_only();

  return x;
}

// Return the index in the arrays of the new instruction
uint CodeBuffer::insert(X_Inst& inst) {
  // Set if there is a bundle set for this instruction
  uint bundle_index = FindBundleSet(inst);

  // If a bundle set is not found, then update the current bundle state
  // to start a new bundle
  if (!bundle_index) {

    // Flush the accumulated instructions
    flush_bundle(false);

    // try again, with only the new instruction
    bundle_index = FindBundleSet(inst);

    assert(bundle_index != 0, "no encoding for instruction found");
  }

  // Add the instruction to the current state; insert the instruction
  // based on the sorted order of the instruction kinds
  Inst::Kind kind = inst.kind();

  uint x, y;

  // Locate the correct position in the list to insert the instruction
  for ( x = 0; x < _inst_count; x++ ) {
    Inst::Kind current = _element[x].kind();
    if( current > kind )
      break;
    else if( current == Inst::L_inst )
      x++;
  }

  // Move all the elements after this one forward two elements
  for ( y = _inst_count; y > x; y-- )
    _element[y+1] = _element[y-1];

  // If this is ordered then propagate to the instruction group
  if( inst.is_ordered() )
    set_is_ordered();

  // Update the register state
  _current_read. or_mask(inst.readState());
  _current_write.or_mask(inst.writeState());

  return x;
}

// Flush the present instruction group
void CodeBuffer::flush_bundle(bool start_new_bundle) {
  uint ndx0, ndx1;
  uint8_t* possibilities = NULL;
  uint inst_slots = 0;
  Inst::Unit inst_unit[MaxInsts];
  uint first_instruction = 0;
  uint first_bundle      = 0;

  // If there is nothing in the bundle, then just quit
  if( _inst_count == 0 ) {
    // Force the start of a new bundle if required
    if( start_new_bundle ) 
      _previous_bundle_format = IPF_Bundle::none;
    return;
  }

  // Point to the bundles
  IPF_Bundle* bundles = (IPF_Bundle*)code_end();

  // See if the previous bundle can be split. If so, then give
  // it a try.
  if( _previous_bundle_format == IPF_Bundle::MsMIs )
    possibilities = (uint8_t*)
      &emit_MsMI_table[_accum_kind_count[Inst::X_inst]]
                      [_accum_kind_count[Inst::B_inst]]
                      [_accum_kind_count[Inst::F_inst]]
                      [_accum_kind_count[Inst::I_inst]]
                      [_accum_kind_count[Inst::M_inst]]
                      [_accum_kind_count[Inst::A_inst]][0];

  else if( _previous_bundle_format == IPF_Bundle::MIsIs )
    possibilities = (uint8_t*)
      &emit_MIsI_table[_accum_kind_count[Inst::X_inst]]
                      [_accum_kind_count[Inst::B_inst]]
                      [_accum_kind_count[Inst::F_inst]]
                      [_accum_kind_count[Inst::I_inst]]
                      [_accum_kind_count[Inst::M_inst]]
                      [_accum_kind_count[Inst::A_inst]][0];

  // If the previous bundle could possibly be split, then investigate further
  if( possibilities ) {

    // Check to see if there is a solution that will fit by splitting
    // the previous bundle
    ndx0 = possibilities[0];

    // Split the previous bundle!
    if( ndx0 ) {
      EmitSpecifier solution = solutions[ndx0];
      first_instruction = IPF_Bundle::firstGroupLength(_previous_bundle_format);
      first_bundle      = 1;
      bundles           = &bundles[-1];

      // set the previous bundle to split
      bundles[0].set_template(solution.kind(0));

      for (uint j = 0; j < first_instruction; j++ )
        inst_unit[inst_slots++] = Inst::Unit_none;
    }

    // No possibility found
    else
      possibilities = NULL;
  }

  // If no split possibility was found, then use the general solution
  if( !possibilities ) {
    possibilities = (uint8_t*)
      &emit_table[_accum_kind_count[Inst::X_inst]]
                 [_accum_kind_count[Inst::B_inst]]
                 [_accum_kind_count[Inst::F_inst]]
                 [_accum_kind_count[Inst::I_inst]]
                 [_accum_kind_count[Inst::M_inst]]
                 [_accum_kind_count[Inst::A_inst]][0];
  }  

  // Look up based on all the instructions accumulated, and see if
  // a set of bundles exists that can encode this set.
  ndx0 = possibilities[0];
  ndx1 = possibilities[1];

#if !defined(PRODUCT) && defined(ASMDEBUG)
  printf("CodeBuffer::flush: %d\n", ndx0);
#endif

  // Verify that the instruction group exists
  assert(ndx0 != 0, "no valid instruction group exists");

  // Get the bundling specifier
  EmitSpecifier solution = solutions[ndx0];
  
  // Fill in the instruction kind from the bundle layout
  for (uint i = 0; i < solution.length(); i++) {
    for (uint j = first_instruction; j < 3; j++)
      inst_unit[inst_slots++] = IPF_Bundle::get_unit(solution.kind(i), j);
    first_instruction = 0;
  }

  // Fill in the bundles with NOPs, based on the bundle template format
  for (uint i = first_bundle; i < solution.length(); i++) {
    IPF_Bundle::Template kind = IPF_Bundle::Template(solution.kind(i));

    bundles[i].set(kind, unit_nop_encoding[IPF_Bundle::get_unit(kind, 0)],
                         unit_nop_encoding[IPF_Bundle::get_unit(kind, 1)],
                         unit_nop_encoding[IPF_Bundle::get_unit(kind, 2)]);
  }

  // Insert each instruction in the the bundle(s)
  //
  // Note: this relies on the A instructions occurring after all M and I instructions
  // Note: the unit and kind are required to be the same, save that the A instruction
  //   kind has no unit, being either an I or M unit
  for ( uint x = 0; x < _inst_count; x++ ) {
    BundleInst &element = _element[x];
    Inst::Kind kind = element.kind();
    uint y;

    // We always want to fill in the last B slot, and we know that the B slots are
    // contiguous (because any B slot in a bundle forces a stall after that bundle)
    // so back up the number of B instructions and start searching.
    if (kind == Inst::B_inst) {
      for ( y = inst_slots - _accum_kind_count[Inst::B_inst]; y < inst_slots; y++ )
        if (inst_unit[y] == Inst::B_unit)
          break;
      assert( y >= 0, "No slot for B instruction found" );
    }

    else if (kind == Inst::A_inst) {
      for ( y = 0; y < inst_slots; y++ )
        if (inst_unit[y] == Inst::M_unit ||
            inst_unit[y] == Inst::I_unit)
          break;
      assert( y < inst_slots, "No slot for A instruction found" );
    }

    else {
      for ( y = 0; y < inst_slots; y++ )
        if (inst_unit[y] == kind)
          break;
      assert( y < inst_slots, "No slot for instruction found" );
    }

    // Now fill in the instruction slots
    uint slot, bundle;

    for ( slot = y, bundle = 0; slot >= 3; slot -= 3, bundle++ );

    // Set the slot and address
    set_code_slot(slot);
    set_code_end( (address)(&bundles[bundle]) );

    // Get the instruction bits
    uint41_t inst = element.inst();

    // incorporate label information
    if( element.has_label() ) {
      Assembler::patched_branch(
        encoded_target(element.label()),
        IPF_Bundle::Template(solution.kind(bundle)),
        slot,
        inst,
        0,
        inst);
    }

    // Incorporate address and relocation info
    if( element.has_address() ) {
      if( kind == Inst::L_inst ) {
        // Verify this is always followed by an X_inst
        assert(_element[x+1].kind() == Inst::X_inst, "must be L/X pair");

	Assembler::patched_branch(
          (intptr_t)element.addr(),
          IPF_Bundle::Template(solution.kind(bundle)),
          slot+1,
          _element[x+1].inst(),
          (intptr_t)code_end(),
          _element[x+1].inst());
      } else {
	Assembler::patched_branch(
          (intptr_t)element.addr(),
          IPF_Bundle::Template(solution.kind(bundle)),
          slot,
          inst,
          (intptr_t)code_end(),
          inst);
      }
    }

    // Emit relocation information
    if( element.has_reloc() ) {
      if( element.reloc() == relocInfo::oop_type ) {
        assert( element.has_address(), "oops require an address" );
        jobject obj = (jobject)element.addr();
        int oop_index = oop_recorder()->allocate_index(obj);
        relocate(code_end(), oop_Relocation::spec(oop_index), code_slot());
      } else {
         relocate(code_end(), element.reloc(), code_slot());
      }
    }

    // Found a slot, now place the bits for the instruction in the instruction
    bundles[bundle].set_slot(inst, slot);

    // Now clear the slot, so that it cannot be filled again
    inst_unit[y] = Inst::Unit_none;
  }

  // Set the end of the code to point past the emitted instruction(s)
  set_code_end( (address)(&bundles[solution.length()]) );

#if !defined(PRODUCT) && defined(ASMDEBUG)
  Disassembler::decode((u_char*)bundles, (u_char*)(bundles + solution.length()));
#endif

  // Reset the bundle buffer state
  reset_bundle_state();

  // See if this can later be a split bundle
  if( !start_new_bundle && ndx1 ) {
    EmitSpecifier solution = solutions[ndx1];
    _previous_bundle_format = solution.kind(solution.length()-1);
  }
}

#ifndef PRODUCT
void CodeBuffer::print_state() {
  print();

  tty->print("\n  inst:%d   (L:%d   B:%d   F:%d   I:%d   M:%d   A:%d)\n\n", 
    _inst_count,
    _accum_kind_count[Inst::L_inst],
    _accum_kind_count[Inst::B_inst],
    _accum_kind_count[Inst::F_inst],
    _accum_kind_count[Inst::I_inst],
    _accum_kind_count[Inst::M_inst],
    _accum_kind_count[Inst::A_inst]);

  for ( uint x = 0; x < _inst_count; x++ ) {
    BundleInst &element = _element[x];
    tty->print("  %d) %s : " PTR64_FORMAT "", x, kind_string[element.kind()], element.inst());
    if( element.has_label() )
      tty->print(" <label>");
    if( element.has_address() )
      tty->print(" <address>");
    tty->print("\n");
  }
}
#endif
