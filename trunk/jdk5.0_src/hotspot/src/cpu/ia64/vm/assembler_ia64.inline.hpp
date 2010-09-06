#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)assembler_ia64.inline.hpp	1.37 03/12/23 16:36:33 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

//=============================================================================
// Displacement
//=============================================================================

// Create an uncompressed Displacement
//
inline Displacement::Displacement(int pos, const AbstractAssembler* masm) {
  IPF_Bundle* bundle = (IPF_Bundle*)masm->addr_at(pos & ~3);
  uint        slot   = pos & 3;
  uint41_t    inst   = bundle->get_slot(slot);

  _inst   = inst;
  _slot   = slot;
  _dclass = Assembler::get_immed_class(bundle->get_template(), slot, inst);
}

inline void Displacement::bind(Label &L, int pos, AbstractAssembler* masm) {
  int label_pos = L.pos();
  next(L);

  int inst_offset = label_pos & ~3;

  uint41_t new_inst;
  Assembler::patched_branch(pos, dclass(), inst(), inst_offset, new_inst);

  IPF_Bundle* bundle = (IPF_Bundle*)masm->addr_at(inst_offset);
  bundle->set_slot(new_inst, label_pos & 3);
}

inline void Displacement::next(Label& L) const {
  // L.pos() points to bundle/slot of next branch to be patched.
  // Terminate when the decoded displacement of the latter points to itself.
  //
  int pos = Assembler::branch_destination(dclass(), inst(), 0) >> 4;

  // If the encoded branch displacement is 0, then this terminates the list
  if (pos == 0) {
    L.unuse();
  } else {
    // The decoded branch displacement field is the bundle byte offset (which is
    // 16-byte aligned) or'ed with the branch instruction slot index.
    //
    L.link_to(pos + L.pos());
  }
}


//=============================================================================
// Assembler
//=============================================================================


inline void Assembler::set_pc_from_code() {
  _code_pos = code()->code_end();
}


// The following methods are used to generate IPF instructions for the
// bundler to incorporate into the code stream. 

inline void Assembler::emit(Inst& inst) {
  code()->emit(inst);
  set_pc_from_code();
}

inline void Assembler::emit(Inst& inst, Label* target) {
  code()->emit(inst, target);
  set_pc_from_code();
}

inline void Assembler::emit(Inst& inst, address addr, relocInfo::relocType rtype) {
  code()->emit(inst, addr, rtype);
  set_pc_from_code();
}

inline void Assembler::emit(X_Inst& inst) {
  code()->emit(inst);
  set_pc_from_code();
}

inline void Assembler::emit(X_Inst& inst, Label* target) {
  code()->emit(inst, target);
  set_pc_from_code();
}

inline void Assembler::emit(X_Inst& inst, address addr, relocInfo::relocType rtype) {
  code()->emit(inst, addr, rtype);
  set_pc_from_code();
}


inline void Assembler::flush_bundle() {
  code()->flush_bundle(true);
  set_pc_from_code();
}

inline void Assembler::eog() {
  code()->flush_bundle(false);
  set_pc_from_code();
}

inline void Assembler::explicit_bundling() { code()->explicit_bundling(); }

inline void Assembler::check_bundling() { code()->check_bundling(); }


inline void Assembler::align(int modulus) {
  // Assume at least 4-byte alignment in the first place.
  while (mask_address_bits(pc(), modulus-1) != 0) {
    emit_long(0);
  }
}


inline address Assembler::emit_addr(address addr) {
  address start = pc();

  *(address*)_code_pos = addr;
  _code_pos += sizeof(address);
  code()->set_code_end(_code_pos);

  return start;
}


// Emit a function descriptor and point it at some entry point, or
// just past the descriptor if an entry point is not specified.
//
inline address Assembler::emit_fd(address entry) {
  // Clear out the bundle cache.
  flush_bundle();

  FuncDesc* fd = (FuncDesc*)pc();

  assert(sizeof(FuncDesc) == 2*sizeof(address), "function descriptor size");

  (void)emit_addr();
  (void)emit_addr();

  fd->set_entry(entry == NULL ? pc() : entry);
  fd->set_gp(_gp);

  return (address)fd;
}


// Given a Label, return the address it refers to.  Labels and displacements
// deal in offsets, but target() must return an address.  If L is bound,
// the address is that of the bundle to which L is bound.  If L is not bound
// and has been referenced, the address points to the bundle that contains the
// instruction that last referenced L.  In this case, the address may contain
// a slot index in its lower two bits.
//
inline address Assembler::target(Label& L) {
  int resultOffset = L.pos();

  if (!L.is_bound()) {
    // Link L to the current code position.
    //
    if (L.is_unused())
      // Circular link terminates fixup chain.
      //
      resultOffset = offset();

    // Branch slot number will be inserted by emit().
    //
    L.link_to(offset());
  }

  return addr_at(resultOffset);
}


// Construct a version of the branch instruction at bundle offset inst_offset
// described by template/slot/inst whose displacement refers to dest_offset.
//
inline void Assembler::patched_branch(int64_t dest_offset,
                                      IPF_Bundle::Template templt, uint slot, uint41_t inst,
                                      int64_t inst_offset,
                                      uint41_t& new_inst, uint41_t& new_L) {
   patched_branch(dest_offset, get_immed_class(templt, slot, inst), inst, inst_offset, new_inst, new_L);
}

inline void Assembler::patched_branch(int64_t dest_offset,
                                      IPF_Bundle::Template templt, uint slot, uint41_t inst,
                                      int64_t inst_offset,
                                      uint41_t& new_inst) {
   uint41_t dummy;
   patched_branch(dest_offset, templt, slot, inst, inst_offset, new_inst, dummy);
}

inline void Assembler::patched_branch(int64_t dest_offset,
                                      Inst::ImmedClass dclass, uint41_t inst,
                                      int64_t inst_offset, uint41_t& new_inst) {
   uint41_t dummy;
   patched_branch(dest_offset, dclass, inst, inst_offset, new_inst, dummy);
}


// Return the offset of the destination of the branch instruction located
// in the bundle at offset described by template/slot/inst.
//
inline int64_t Assembler::branch_destination(IPF_Bundle::Template templt, uint slot, uint41_t inst,
                                             int64_t offset, uint41_t L) {
  return branch_destination(get_immed_class(templt, slot, inst), inst, offset, L);
}


inline bool Assembler::is_movl(IPF_Bundle::Template templt, uint41_t X) {
  return X2::is_movl(templt, X, IPF_Bundle::m_MLX | IPF_Bundle::m_MLXs);
}


inline bool Assembler::is_call_indirect(IPF_Bundle::Template templt, uint41_t X) {
  return B5::is_call_indirect(templt, X,
    IPF_Bundle::m_MIB | IPF_Bundle::m_MIBs |
    IPF_Bundle::m_MBB | IPF_Bundle::m_MBBs |
    IPF_Bundle::m_BBB | IPF_Bundle::m_BBBs |
    IPF_Bundle::m_MMB | IPF_Bundle::m_MMBs |
    IPF_Bundle::m_MFB | IPF_Bundle::m_MFBs);
}

inline void Assembler::adda(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(0, 0, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::adda(                      Register dst, Register src1, Register src2) {
  adda(PR0, dst, src1, src2);
}

inline void Assembler::add1(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(0, 1, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::add1(                      Register dst, Register src1, Register src2) {
  add1(PR0, dst, src1, src2);
}

inline void Assembler::adds(PredicateRegister qp, Register dst, int imm14, Register src) {
  verify_signed_range(imm14, 14);
  A4 inst(2, dst, imm14, src, qp);
  emit(inst);
}
inline void Assembler::adds(                      Register dst, int imm14, Register src) {
  adds(PR0, dst, imm14, src);
}


inline void Assembler::addl(PredicateRegister qp, Register dst, int imm22, Register src) {
  verify_signed_range(imm22, 22);
  verify_0to3(src);
  A5 inst(dst, imm22, src, qp);
  emit(inst);
}
inline void Assembler::addl(                      Register dst, int imm22, Register src) {
  addl(PR0, dst, imm22, src);
}


inline void Assembler::addp4(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(2, 0, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::addp4(                      Register dst, Register src1, Register src2) {
  addp4(PR0, dst, src1, src2);
}

inline void Assembler::addp4(PredicateRegister qp, Register dst, int imm14, Register src) {
  verify_signed_range(imm14, 14);
  A4 inst(3, dst, imm14, src, qp);
  emit(inst);
}
inline void Assembler::addp4(                      Register dst, int imm14, Register src) {
  addp4(PR0, dst, imm14, src);
}


inline void Assembler::shladd(PredicateRegister qp, Register dst, Register src, uint count, Register addend) {
  verify_shladd_count(count);
  A2 inst(4, dst, src, count, addend, qp);
  emit(inst);
}
inline void Assembler::shladd(                      Register dst, Register src, uint count, Register addend) {
  shladd(PR0, dst, src, count, addend);
}

inline void Assembler::shladdp4(PredicateRegister qp, Register dst, Register src, uint count, Register addend) {
  verify_shladd_count(count);
  A2 inst(6, dst, src, count, addend, qp);
  emit(inst);
}
inline void Assembler::shladdp4(                      Register dst, Register src, uint count, Register addend) {
  shladdp4(PR0, dst, src, count, addend);
}


inline void Assembler::sub(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(1, 1, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::sub(                      Register dst, Register src1, Register src2) {
  sub(PR0, dst, src1, src2);
}

inline void Assembler::sub1(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(1, 0, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::sub1(                      Register dst, Register src1, Register src2) {
  sub1(PR0, dst, src1, src2);
}

inline void Assembler::sub(PredicateRegister qp, Register dst, int imm8, Register src) {
  verify_signed_range(imm8, 8);
  A3 inst(0x9, 1, dst, imm8, src, qp);
  emit(inst);
}
inline void Assembler::sub(                      Register dst, int imm8, Register src) {
  sub(PR0, dst, imm8, src);
}


inline void Assembler::and3(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(3, 0, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::and3(                      Register dst, Register src1, Register src2) {
  and3(PR0, dst, src1, src2);
}

inline void Assembler::and3(PredicateRegister qp, Register dst, int imm8, Register src) {
  verify_signed_range(imm8, 8);
  A3 inst(0xB, 0, dst, imm8, src, qp);
  emit(inst);
}
inline void Assembler::and3(                      Register dst, int imm8, Register src) {
  and3(PR0, dst, imm8, src);
}


inline void Assembler::andcm(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(3, 1, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::andcm(                      Register dst, Register src1, Register src2) {
  andcm(PR0, dst, src1, src2);
}

inline void Assembler::andcm(PredicateRegister qp, Register dst, int imm8, Register src) {
  verify_signed_range(imm8, 8);
  A3 inst(0xB, 1, dst, imm8, src, qp);
  emit(inst);
}
inline void Assembler::andcm(                      Register dst, int imm8, Register src2) {
  andcm(PR0, dst, imm8, src2);
}


inline void Assembler::or3(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(3, 2, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::or3(                      Register dst, Register src1, Register src2) {
  or3(PR0, dst, src1, src2);
}

inline void Assembler::or3(PredicateRegister qp, Register dst, int imm8, Register src) {
  verify_signed_range(imm8, 8);
  A3 inst(0xB, 2, dst, imm8, src, qp);
  emit(inst);
}
inline void Assembler::or3(                      Register dst, int imm8, Register src) {
  or3(PR0, dst, imm8, src);
}


inline void Assembler::xor3(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A1 inst(3, 3, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::xor3(                      Register dst, Register src1, Register src2) {
  xor3(PR0, dst, src1, src2);
}

inline void Assembler::xor3(PredicateRegister qp, Register dst, int imm8, Register src) {
  verify_signed_range(imm8, 8);
  A3 inst(0xB, 3, dst, imm8, src, qp);
  emit(inst);
}
inline void Assembler::xor3(                      Register dst, int imm8, Register src) {
  xor3(PR0, dst, imm8, src);
}


inline void Assembler::cmp(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                           Cmp_Relation crel, Cmp_Result ctype) {
  cmp_(qp, p1, p2, src1, src2, crel, ctype, 0);
}
inline void Assembler::cmp(                      PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                           Cmp_Relation crel, Cmp_Result ctype) {
  cmp(PR0, p1, p2, src1, src2, crel, ctype);
}

inline void Assembler::cmp0(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
                            Cmp_Relation crel, Cmp_Result ctype) {
  if (crel == equal || crel == notEqual) {
    cmp_(qp, p1, p2, src, GR0, crel, ctype, 0);
  } else {
    cmp0_(qp, p1, p2, src, crel, ctype, 0);
  }
}
inline void Assembler::cmp0(                      PredicateRegister p1, PredicateRegister p2, Register src,
                            Cmp_Relation crel, Cmp_Result ctype) {
  cmp0(PR0, p1, p2, src, crel, ctype);
}

inline void Assembler::cmp(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                           Cmp_Relation crel, Cmp_Result ctype) {
  cmp_(qp, p1, p2, imm, src, crel, ctype, 2);
}
inline void Assembler::cmp(                      PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                           Cmp_Relation crel, Cmp_Result ctype) {
  cmp(PR0, p1, p2, imm, src, crel, ctype);
}


inline void Assembler::cmp4(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                            Cmp_Relation crel, Cmp_Result ctype) {
  cmp_(qp, p1, p2, src1, src2, crel, ctype, 1);
}
inline void Assembler::cmp4(                      PredicateRegister p1, PredicateRegister p2, Register src1, Register src2,
                            Cmp_Relation crel, Cmp_Result ctype) {
  cmp4(PR0, p1, p2, src1, src2, crel, ctype);
}

inline void Assembler::cmp40(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
                             Cmp_Relation crel, Cmp_Result ctype) {
  if (crel == equal || crel == notEqual) {
    cmp_(qp, p1, p2, src, GR0, crel, ctype, 1);
  } else {
    cmp0_(qp, p1, p2, src, crel, ctype, 1);
  }
}
inline void Assembler::cmp40(                      PredicateRegister p1, PredicateRegister p2, Register src,
                             Cmp_Relation crel, Cmp_Result ctype) {
  cmp40(PR0, p1, p2, src, crel, ctype);
}

inline void Assembler::cmp4(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                            Cmp_Relation crel, Cmp_Result ctype) {
  cmp_(qp, p1, p2, imm, src, crel, ctype, 3);
}
inline void Assembler::cmp4(                      PredicateRegister p1, PredicateRegister p2, int imm, Register src,
                            Cmp_Relation crel, Cmp_Result ctype) {
  cmp4(PR0, p1, p2, imm, src, crel, ctype);
}


inline void Assembler::padd1(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s) {
  A9 inst(0, 0, 0, s, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::padd1(                      Register dst, Register src1, Register src2, Saturation s) {
  padd1(PR0, dst, src1, src2, s);
}

inline void Assembler::padd2(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s) {
  A9 inst(0, 1, 0, s, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::padd2(                      Register dst, Register src1, Register src2, Saturation s) {
  padd2(PR0, dst, src1, src2, s);
}

inline void Assembler::padd4(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s) {
  assert(s == modulo, "bad saturation");
  A9 inst(1, 0, 0, s, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::padd4(                      Register dst, Register src1, Register src2, Saturation s) {
  padd4(PR0, dst, src1, src2, s);
}


inline void Assembler::psub1(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s) {
  A9 inst(0, 0, 1, s, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::psub1(                      Register dst, Register src1, Register src2, Saturation s) {
  psub1(PR0, dst, src1, src2, s);
}

inline void Assembler::psub2(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s) {
  A9 inst(0, 1, 1, s, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::psub2(                      Register dst, Register src1, Register src2, Saturation s) {
  psub2(PR0, dst, src1, src2, s);
}

inline void Assembler::psub4(PredicateRegister qp, Register dst, Register src1, Register src2, Saturation s) {
  assert(s == modulo, "bad saturation");
  A9 inst(1, 0, 1, s, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::psub4(                      Register dst, Register src1, Register src2, Saturation s) {
  psub4(PR0, dst, src1, src2, s);
}


inline void Assembler::pavg1(PredicateRegister qp, Register dst, Register src1, Register src2, pavg_Round r) {
  A9 inst(0, 0, 2, r, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::pavg1(                      Register dst, Register src1, Register src2, pavg_Round r) {
  pavg1(PR0, dst, src1, src2, r);
}

inline void Assembler::pavg2(PredicateRegister qp, Register dst, Register src1, Register src2, pavg_Round r) {
  A9 inst(0, 1, 2, r, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::pavg2(                      Register dst, Register src1, Register src2, pavg_Round r) {
  pavg2(PR0, dst, src1, src2, r);
}


inline void Assembler::pavgsub1(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A9 inst(0, 0, 3, 2, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::pavgsub1(                      Register dst, Register src1, Register src2) {
  pavgsub1(PR0, dst, src1, src2);
}

inline void Assembler::pavgsub2(PredicateRegister qp, Register dst, Register src1, Register src2) {
  A9 inst(0, 1, 3, 2, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::pavgsub2(                      Register dst, Register src1, Register src2) {
  pavgsub2(PR0, dst, src1, src2);
}


inline void Assembler::pcmp1(PredicateRegister qp, Register dst, Register src1, Register src2, Cmp_Relation prel) {
  verify_prel(prel);
  A9 inst(0, 0, 9, prel == equal ? 0 : 1, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::pcmp1(                      Register dst, Register src1, Register src2, Cmp_Relation prel) {
  pcmp1(PR0, dst, src1, src2, prel);
}

inline void Assembler::pcmp2(PredicateRegister qp, Register dst, Register src1, Register src2, Cmp_Relation prel) {
  verify_prel(prel);
  A9 inst(0, 1, 9, prel == equal ? 0 : 1, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::pcmp2(                      Register dst, Register src1, Register src2, Cmp_Relation prel) {
  pcmp2(PR0, dst, src1, src2, prel);
}

inline void Assembler::pcmp4(PredicateRegister qp, Register dst, Register src1, Register src2, Cmp_Relation prel) {
  verify_prel(prel);
  A9 inst(1, 0, 9, prel == equal ? 0 : 1, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::pcmp4(                      Register dst, Register src1, Register src2, Cmp_Relation prel) {
  pcmp4(PR0, dst, src1, src2, prel);
}


inline void Assembler::pshladd2(PredicateRegister qp, Register dst, Register src1, uint count, Register src2) {
  verify_pshift_count(count);
  A10 inst(4, dst, src1, count, src2, qp);
  emit(inst);  
}
inline void Assembler::pshladd2(                      Register dst, Register src1, uint count, Register src2) {
  pshladd2(PR0, dst, src1, count, src2);
}

inline void Assembler::pshradd2(PredicateRegister qp, Register dst, Register src1, uint count, Register src2) {
  verify_pshift_count(count);
  A10 inst(6, dst, src1, count, src2, qp);
  emit(inst);  
}
inline void Assembler::pshradd2(                      Register dst, Register src1, uint count, Register src2) {
  pshradd2(PR0, dst, src1, count, src2);
}


inline void Assembler::shr(PredicateRegister qp, Register dst, Register src, Register count) {
  I5 inst(1, 1, 2, dst, src, count, qp);
  emit(inst);
}
inline void Assembler::shr(                      Register dst, Register src, Register count) {
  shr(PR0, dst, src, count);
}

inline void Assembler::shru(PredicateRegister qp, Register dst, Register src, Register count) {
  I5 inst(1, 1, 0, dst, src, count, qp);
  emit(inst);
}
inline void Assembler::shru(                      Register dst, Register src, Register count) {
  shru(PR0, dst, src, count);
}

inline void Assembler::shl(PredicateRegister qp, Register dst, Register src, Register count) {
  I7 inst(1, 1, dst, src, count, qp);
  emit(inst);
}
inline void Assembler::shl(                      Register dst, Register src, Register count) {
  shl(PR0, dst, src, count);
}

inline void Assembler::popc(PredicateRegister qp, Register dst, Register src) {
  I9 inst(dst, src, qp);
  emit(inst);
}
inline void Assembler::popc(                      Register dst, Register src) {
  popc(PR0, dst, src);
}


inline void Assembler::shrp(PredicateRegister qp, Register dst, Register src1, Register src2, uint count) {
  verify_unsigned_range(count, 6);
  I10 inst(dst, src1, src2, count, qp);
  emit(inst);
}
inline void Assembler::shrp(                      Register dst, Register src1, Register src2, uint count) {
  shrp(PR0, dst, src1, src2, count);
}


inline void Assembler::extru(PredicateRegister qp, Register dst, Register src, uint pos, uint len) {
  verify_field_len(len, 64);
  verify_unsigned_range(pos, 6);
  I11 inst(0, dst, src, pos, len, qp);
  emit(inst);
}
inline void Assembler::extru(                      Register dst, Register src, uint pos, uint len) {
  extru(PR0, dst, src, pos, len);
}

inline void Assembler::extr(PredicateRegister qp, Register dst, Register src, uint pos, uint len) {
  verify_field_len(len, 64);
  verify_unsigned_range(pos, 6);
  I11 inst(1, dst, src, pos, len, qp);
  emit(inst);
}
inline void Assembler::extr(                      Register dst, Register src, uint pos, uint len) {
  extr(PR0, dst, src, pos, len);
}


inline void Assembler::depz(PredicateRegister qp, Register dst, Register src, uint pos, uint len) {
  verify_field_len(len, 64);
  verify_unsigned_range(pos, 6);
  I12 inst(dst, src, pos, len, qp);
  emit(inst);
}
inline void Assembler::depz(                      Register dst, Register src, uint pos, uint len) {
  depz(PR0, dst, src, pos, len);
}

inline void Assembler::depz(PredicateRegister qp, Register dst, int imm8, uint pos, uint len) {
  verify_field_len(len, 64);
  verify_unsigned_range(pos, 6);
  verify_signed_range(imm8, 8);
  I13 inst(dst, imm8, pos, len, qp);
  emit(inst);
}
inline void Assembler::depz(                      Register dst, int imm8, uint pos, uint len) {
  depz(PR0, dst, imm8, pos, len);
}

inline void Assembler::dep(PredicateRegister qp, Register dst, int imm1, Register src, uint pos, uint len) {
  verify_field_len(len, 64);
  verify_unsigned_range(pos, 6);
  verify_unsigned_range(imm1, 1);
  I14 inst(dst, imm1, src, pos, len, qp);
  emit(inst);
}
inline void Assembler::dep(                      Register dst, int imm1, Register src, uint pos, uint len) {
  dep(PR0, dst, imm1, src, pos, len);
}


inline void Assembler::tbit(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src, uint pos,
                            Cmp_Relation trel, Cmp_Result ctype) {
  tbit_(qp, p1, p2, src, pos, trel, ctype, 0);
}
inline void Assembler::tbit(                      PredicateRegister p1, PredicateRegister p2, Register src, uint pos,
                            Cmp_Relation trel, Cmp_Result ctype) {
  tbit_(PR0, p1, p2, src, pos, trel, ctype, 0);
}

inline void Assembler::tnat(PredicateRegister qp, PredicateRegister p1, PredicateRegister p2, Register src,
                            Cmp_Relation trel, Cmp_Result ctype) {
  tbit_(qp, p1, p2, src, 0, trel, ctype, 1);
}
inline void Assembler::tnat(                      PredicateRegister p1, PredicateRegister p2, Register src,
                            Cmp_Relation trel, Cmp_Result ctype) {
  tbit_(PR0, p1, p2, src, 0, trel, ctype, 1);
}


inline void Assembler::breaki(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  I19 inst(0, imm21, qp);
  emit(inst);
}
inline void Assembler::breaki(                      uint imm21) {
  breaki(PR0, imm21);
}

inline void Assembler::nopi(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  I19 inst(1, imm21, qp);
  emit(inst);
}
inline void Assembler::nopi(                      uint imm21) {
  nopi(PR0, imm21);
}


inline void Assembler::chksi(PredicateRegister qp, Register src, Label& target) {
  I20 inst(src, 0, qp);
  emit(inst, &target);
}
inline void Assembler::chksi(                      Register src, Label& target) {
  chksi(PR0, src, target);
}


inline void Assembler::movi(PredicateRegister qp, BranchRegister dst, Register src, uint tag13,
                            Branch_Hint mwh, Importance_Hint ih) {
  I21 inst(0, ih, mwh, dst, src, tag13, qp);
  emit(inst);
}
inline void Assembler::movi(                      BranchRegister dst, Register src, uint tag13,
                            Branch_Hint mwh, Importance_Hint ih) {
  movi(PR0, dst, src, tag13, mwh, ih);
}

inline void Assembler::movret(PredicateRegister qp, BranchRegister dst, Register src, uint tag13,
                              Branch_Hint mwh, Importance_Hint ih) {
  I21 inst(1, ih, mwh, dst, src, tag13, qp);
  emit(inst);
}
inline void Assembler::movret(                      BranchRegister dst, Register src, uint tag13,
                              Branch_Hint mwh, Importance_Hint ih) {
  movret(PR0, dst, src ,tag13, mwh, ih);
}

inline void Assembler::movi(PredicateRegister qp, BranchRegister dst, Register src, Label& tag,
                            Branch_Hint mwh, Importance_Hint ih) {
  I21 inst(1, ih, mwh, dst, src, 0, qp);
  emit(inst, &tag);
}
inline void Assembler::movi(                      BranchRegister dst, Register src, Label& tag,
                            Branch_Hint mwh, Importance_Hint ih) {
  movi(PR0, dst, src, tag, mwh, ih);
}

inline void Assembler::movret(PredicateRegister qp, BranchRegister dst, Register src, Label& tag,
                              Branch_Hint mwh, Importance_Hint ih) {
  I21 inst(1, ih, mwh, dst, src, 0, qp);
  emit(inst, &tag);
}
inline void Assembler::movret(                      BranchRegister dst, Register src, Label& tag,
                              Branch_Hint mwh, Importance_Hint ih) {
  movret(PR0, dst, src, tag, mwh, ih);
}


inline void Assembler::movi(PredicateRegister qp, Register dst, BranchRegister src) {
  I22 inst(dst, src, qp);
  emit(inst);
}
inline void Assembler::movi(                      Register dst, BranchRegister src) {
  movi(PR0, dst, src);
}


inline void Assembler::mov_to_pr(PredicateRegister qp, Register src, int mask17) {
  I23 inst(src, mask17, qp);
  emit(inst);
}
inline void Assembler::mov_to_pr(                      Register src, int mask17) {
  mov_to_pr(PR0, src, mask17);
}

inline void Assembler::mov_to_prrot(PredicateRegister qp, int64_t imm44) {
  I24 inst(imm44, qp);
  emit(inst);
}
inline void Assembler::mov_to_prrot(                      int64_t imm44) {
  mov_to_prrot(PR0, imm44);
}


inline void Assembler::mov_from_ip(PredicateRegister qp, Register dst) {
  I25 inst(0x30, dst, qp);
  emit(inst);
}
inline void Assembler::mov_from_ip(                      Register dst) {
  mov_from_ip(PR0, dst);
}

inline void Assembler::mov_from_pr(PredicateRegister qp, Register dst) {
  I25 inst(0x33, dst, qp);
  emit(inst);
}
inline void Assembler::mov_from_pr(                      Register dst) {
  mov_from_pr(PR0, dst);
}


inline void Assembler::movi(PredicateRegister qp, ApplicationRegister dst, Register src) {
  I26 inst(dst, src, qp);
  emit(inst);
}
inline void Assembler::movi(                      ApplicationRegister dst, Register src) {
  movi(PR0, dst, src);
}

inline void Assembler::movi(PredicateRegister qp, ApplicationRegister dst, int imm8) {
  verify_signed_range(imm8, 8);
  I27 inst(dst, imm8, qp);
  emit(inst);
}
inline void Assembler::movi(                      ApplicationRegister dst, int imm8) {
  movi(PR0, dst, imm8);
}

inline void Assembler::movi(PredicateRegister qp, Register dst, ApplicationRegister src) {
  I28 inst(dst, src, qp);
  emit(inst);
}
inline void Assembler::movi(                      Register dst, ApplicationRegister src) {
  movi(PR0, dst, src);
}


inline void Assembler::zxt1(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x0, dst, src, qp);
  emit(inst);
}
inline void Assembler::zxt1(                      Register dst, Register src) {
  zxt1(PR0, dst, src);
}

inline void Assembler::zxt2(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x1, dst, src, qp);
  emit(inst);
}
inline void Assembler::zxt2(                      Register dst, Register src) {
  zxt2(PR0, dst, src);
}

inline void Assembler::zxt4(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x2, dst, src, qp);
  emit(inst);
}
inline void Assembler::zxt4(                      Register dst, Register src) {
  zxt4(PR0, dst, src);
}


inline void Assembler::sxt1(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x4, dst, src, qp);
  emit(inst);
}
inline void Assembler::sxt1(                      Register dst, Register src) {
  sxt1(PR0, dst, src);
}

inline void Assembler::sxt2(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x5, dst, src, qp);
  emit(inst);
}
inline void Assembler::sxt2(                      Register dst, Register src) {
  sxt2(PR0, dst, src);
}

inline void Assembler::sxt4(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x6, dst, src, qp);
  emit(inst);
}
inline void Assembler::sxt4(                      Register dst, Register src) {
  sxt4(PR0, dst, src);
}


inline void Assembler::czx1l(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x8, dst, src, qp);
  emit(inst);
}
inline void Assembler::czx1l(                      Register dst, Register src) {
  czx1l(PR0, dst, src);
}

inline void Assembler::czx1r(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0x9, dst, src, qp);
  emit(inst);
}
inline void Assembler::czx1r(                      Register dst, Register src) {
  czx1r(PR0, dst, src);
}

inline void Assembler::czx2l(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0xC, dst, src, qp);
  emit(inst);
}
inline void Assembler::czx2l(                      Register dst, Register src) {
  czx2l(PR0, dst, src);
}

inline void Assembler::czx2r(PredicateRegister qp, Register dst, Register src) {
  I29 inst(0xD, dst, src, qp);
  emit(inst);
}
inline void Assembler::czx2r(                      Register dst, Register src) {
  czx2r(PR0, dst, src);
}


inline void Assembler::ld1(PredicateRegister qp, Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M1 inst(ldtype | 0, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ld1(                      Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld1(PR0, dst, addr, ldtype, ldhint);
}

inline void Assembler::ld2(PredicateRegister qp, Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M1 inst(ldtype | 1, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ld2(                      Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld2(PR0, dst, addr, ldtype, ldhint);
}

inline void Assembler::ld4(PredicateRegister qp, Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M1 inst(ldtype | 2, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ld4(                      Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld4(PR0, dst, addr, ldtype, ldhint);
}

inline void Assembler::ld8(PredicateRegister qp, Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M1 inst(ldtype | 3, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ld8(                      Register dst, Register addr,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld8(PR0, dst, addr, ldtype, ldhint);
}


inline void Assembler::ld8fill(PredicateRegister qp, Register dst, Register addr,
                               LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M1 inst(0x1B, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ld8fill(                      Register dst, Register addr,
                               LoadStore_Hint ldhint) {
  ld8fill(PR0, dst, addr, ldhint);
}


inline void Assembler::ld1(PredicateRegister qp, Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M2 inst(ldtype | 0, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld1(                      Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld1(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ld2(PredicateRegister qp, Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M2 inst(ldtype | 1, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld2(                      Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld2(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ld4(PredicateRegister qp, Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M2 inst(ldtype | 2, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld4(                      Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld4(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ld8(PredicateRegister qp, Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M2 inst(ldtype | 3, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld8(                      Register dst, Register addr, Register adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld8(PR0, dst, addr, adjust, ldtype, ldhint);
}


inline void Assembler::ld8fill(PredicateRegister qp, Register dst, Register addr, Register adjust,
                               LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M2 inst(0x1B, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld8fill(                      Register dst, Register addr, Register adjust,
                               LoadStore_Hint ldhint) {
  ld8fill(PR0, dst, addr, adjust, ldhint);
}


inline void Assembler::ld1(PredicateRegister qp, Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_load(ldhint);
  M3 inst(ldtype | 0, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld1(                      Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld1(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ld2(PredicateRegister qp, Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_load(ldhint);
  M3 inst(ldtype | 1, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld2(                      Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld2(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ld4(PredicateRegister qp, Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_load(ldhint);
  M3 inst(ldtype | 2, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld4(                      Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld4(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ld8(PredicateRegister qp, Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_load(ldhint);
  M3 inst(ldtype | 3, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld8(                      Register dst, Register addr, int adjust,
                           LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld8(PR0, dst, addr, adjust, ldtype, ldhint);
}


inline void Assembler::ld8fill(PredicateRegister qp, Register dst, Register addr, int adjust,
                               LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_load(ldhint);
  M3 inst(0x1B, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ld8fill(                      Register dst, Register addr, int adjust,
                               LoadStore_Hint ldhint) {
  ld8fill(PR0, dst, addr, adjust, ldhint);
}


inline void Assembler::st1(PredicateRegister qp, Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_store(sttype, sthint);
  M4 inst(sttype | 0, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::st1(                      Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st1(PR0, addr, src, sttype, sthint);
}

inline void Assembler::st2(PredicateRegister qp, Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_store(sttype, sthint);
  M4 inst(sttype | 1, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::st2(                      Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st2(PR0, addr, src, sttype, sthint);
}

inline void Assembler::st4(PredicateRegister qp, Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_store(sttype, sthint);
  M4 inst(sttype | 2, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::st4(                      Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st4(PR0, addr, src, sttype, sthint);
}

inline void Assembler::st8(PredicateRegister qp, Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_store(sttype, sthint);
  M4 inst(sttype | 3, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::st8(                      Register addr, Register src,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st8(PR0, addr, src, sttype, sthint);
}


inline void Assembler::st8spill(PredicateRegister qp, Register addr, Register src,
                                LoadStore_Hint sthint) {
  verify_store(normal, sthint);
  M4 inst(0xB, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::st8spill(                      Register addr, Register src,
                                LoadStore_Hint sthint) {
  st8spill(PR0, addr, src, sthint);
}


inline void Assembler::st1(PredicateRegister qp, Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(sttype, sthint);
  M5 inst(0xB, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::st1(                      Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st1(PR0, addr, src, adjust, sttype, sthint);
}

inline void Assembler::st2(PredicateRegister qp, Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(sttype, sthint);
  M5 inst(sttype | 1, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::st2(                      Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st2(PR0, addr, src, adjust, sttype, sthint);
}

inline void Assembler::st4(PredicateRegister qp, Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(sttype, sthint);
  M5 inst(sttype | 2, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::st4(                      Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st4(PR0, addr, src, adjust, sttype, sthint);
}

inline void Assembler::st8(PredicateRegister qp, Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(sttype, sthint);
  M5 inst(sttype | 3, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::st8(                      Register addr, Register src, int adjust,
                           LoadStore_Type sttype, LoadStore_Hint sthint) {
  st8(PR0, addr, src, adjust, sttype, sthint);
}


inline void Assembler::st8spill(PredicateRegister qp, Register addr, Register src, int adjust,
                                LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(normal, sthint);
  M5 inst(0xB, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::st8spill(                      Register addr, Register src, int adjust,
                                LoadStore_Hint sthint) {
  st8spill(PR0, addr, src, adjust, sthint);
}


inline void Assembler::ldfs(PredicateRegister qp, FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M6 inst(ldtype | 2, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ldfs(                      FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfs(PR0, dst, addr, ldtype, ldhint);
}

inline void Assembler::ldfd(PredicateRegister qp, FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M6 inst(ldtype | 3, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ldfd(                      FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfd(PR0, dst, addr, ldtype, ldhint);
}

inline void Assembler::ldf8(PredicateRegister qp, FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M6 inst(ldtype | 1, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ldf8(                      FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldf8(PR0, dst, addr, ldtype, ldhint);
}

inline void Assembler::ldfe(PredicateRegister qp, FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M6 inst(ldtype | 0, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ldfe(                      FloatRegister dst, Register addr,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfe(PR0, dst, addr, ldtype, ldhint);
}


inline void Assembler::ldffill(PredicateRegister qp, FloatRegister dst, Register addr,
                               LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M6 inst(0x1B, ldhint, dst, addr, qp);
  emit(inst);
}
inline void Assembler::ldffill(                      FloatRegister dst, Register addr,
                               LoadStore_Hint ldhint) {
  ldffill(PR0, dst, addr, ldhint);
}


inline void Assembler::ldfs(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M7 inst(ldtype | 2, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldfs(                      FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfs(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldfd(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M7 inst(ldtype | 3, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldfd(                      FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfd(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldf8(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M7 inst(ldtype | 1, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldf8(                      FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldf8(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldfe(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  verify_load(ldhint);
  M7 inst(ldtype | 0, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldfe(                      FloatRegister dst, Register addr, Register adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfe(PR0, dst, addr, adjust, ldtype, ldhint);
}


inline void Assembler::ldffill(PredicateRegister qp, FloatRegister dst, Register addr, Register adjust,
                               LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M7 inst(0x1B, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldffill(                   FloatRegister dst, Register addr, Register adjust,
                               LoadStore_Hint ldhint) {
  ldffill(PR0, dst, addr, adjust, ldhint);
}


inline void Assembler::ldfs(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_fload(ldtype);
  verify_load(ldhint);
  M8 inst(ldtype | 2, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldfs(                      FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfs(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldfd(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_fload(ldtype);
  verify_load(ldhint);
  M8 inst(ldtype | 3, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldfd(                      FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfd(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldf8(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_fload(ldtype);
  verify_load(ldhint);
  M8 inst(ldtype | 1, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldf8(                      FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldf8(PR0, dst, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldfe(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_fload(ldtype);
  verify_load(ldhint);
  M8 inst(ldtype | 0, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldfe(                      FloatRegister dst, Register addr, int adjust,
                            LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfe(PR0, dst, addr, adjust, ldtype, ldhint);
}


inline void Assembler::ldffill(PredicateRegister qp, FloatRegister dst, Register addr, int adjust,
                               LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  verify_load(ldhint);
  M8 inst(0x1B, ldhint, dst, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::ldffill(                      FloatRegister dst, Register addr, int adjust,
                               LoadStore_Hint ldhint) {
  ldffill(PR0, dst, addr, adjust, ldhint);
}


inline void Assembler::stfs(PredicateRegister qp, Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  verify_store(normal, sthint);
  M9 inst(2, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::stfs(                      Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  stfs(PR0, addr, src, sthint);
}

inline void Assembler::stfd(PredicateRegister qp, Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  verify_store(normal, sthint);
  M9 inst(3, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::stfd(                      Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  stfd(PR0, addr, src, sthint);
}

inline void Assembler::stf8(PredicateRegister qp, Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  verify_store(normal, sthint);
  M9 inst(1, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::stf8(                      Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  stf8(PR0, addr, src, sthint);
}

inline void Assembler::stfe(PredicateRegister qp, Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  verify_store(normal, sthint);
  M9 inst(0, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::stfe(                      Register addr, FloatRegister src,
                            LoadStore_Hint sthint) {
  stfe(PR0, addr, src, sthint);
}


inline void Assembler::stfspill(PredicateRegister qp, Register addr, FloatRegister src,
                                LoadStore_Hint sthint) {
  verify_store(normal, sthint);
  M9 inst(0x1B, sthint, addr, src, qp);
  emit(inst);
}
inline void Assembler::stfspill(                      Register addr, FloatRegister src,
                                LoadStore_Hint sthint) {
  stfspill(PR0, addr, src, sthint);
}


inline void Assembler::stfs(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(normal, sthint);
  M10 inst(2, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::stfs(                      Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  stfs(PR0, addr, src, adjust, sthint);
}

inline void Assembler::stfd(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(normal, sthint);
  M10 inst(3, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::stfd(                      Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  stfd(PR0, addr, src, adjust, sthint);
}

inline void Assembler::stf8(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(normal, sthint);
  M10 inst(1, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::stf8(                      Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  stf8(PR0, addr, src, adjust, sthint);
}

inline void Assembler::stfe(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(normal, sthint);
  M10 inst(0, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::stfe(                      Register addr, FloatRegister src, int adjust,
                            LoadStore_Hint sthint) {
  stfe(PR0, addr, src, adjust, sthint);
}


inline void Assembler::stfspill(PredicateRegister qp, Register addr, FloatRegister src, int adjust,
                                LoadStore_Hint sthint) {
  verify_signed_range(adjust, 9);
  verify_store(normal, sthint);
  M10 inst(0x1B, sthint, addr, src, adjust, qp);
  emit(inst);
}
inline void Assembler::stfspill(                      Register addr, FloatRegister src, int adjust,
                                LoadStore_Hint sthint) {
  stfspill(PR0, addr, src, adjust, sthint);
}


inline void Assembler::ldfps(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  M11 inst(ldtype | 2, ldhint, dstlo, dsthi, addr, qp);
  emit(inst);
}
inline void Assembler::ldfps(                      FloatRegister dstlo, FloatRegister dsthi, Register addr,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfps(PR0, dstlo, dsthi, addr, ldtype, ldhint);
}

inline void Assembler::ldfpd(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  M11 inst(ldtype | 3, ldhint, dstlo, dsthi, addr, qp);
  emit(inst);
}
inline void Assembler::ldfpd(                      FloatRegister dstlo, FloatRegister dsthi, Register addr,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfpd(PR0, dstlo, dsthi, addr, ldtype, ldhint);
}

inline void Assembler::ldfp8(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  verify_fload(ldtype);
  M11 inst(ldtype | 1, ldhint, dstlo, dsthi, addr, qp);
  emit(inst);
}
inline void Assembler::ldfp8(                      FloatRegister dstlo, FloatRegister dsthi, Register addr,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfp8(PR0, dstlo, dsthi, addr, ldtype, ldhint);
}


inline void Assembler::ldfps(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  assert(adjust == 8, "bad increment");
  verify_fload(ldtype);
  M12 inst(ldtype | 2, ldhint, dstlo, dsthi, addr, qp);
  emit(inst);
}
inline void Assembler::ldfps(                      FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfps(PR0, dstlo, dsthi, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldfpd(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  assert(adjust == 16, "bad increment");
  verify_fload(ldtype);
  M12 inst(ldtype | 3, ldhint, dstlo, dsthi, addr, qp);
  emit(inst);
}
inline void Assembler::ldfpd(                      FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfpd(PR0, dstlo, dsthi, addr, adjust, ldtype, ldhint);
}

inline void Assembler::ldfp8(PredicateRegister qp, FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  assert(adjust == 16, "bad increment");
  verify_fload(ldtype);
  M12 inst(ldtype | 1, ldhint, dstlo, dsthi, addr, qp);
  emit(inst);
}
inline void Assembler::ldfp8(                      FloatRegister dstlo, FloatRegister dsthi, Register addr, int adjust,
                             LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ldfp8(PR0, dstlo, dsthi, addr, adjust, ldtype, ldhint);
}


inline void Assembler::lfetch(PredicateRegister qp, Register addr,
                              LinePrefetch_Type lftype, LoadStore_Hint ldhint) {
  M13 inst(lftype, ldhint, addr, qp);
  emit(inst);
}
inline void Assembler::lfetch(                      Register addr,
                              LinePrefetch_Type lftype, LoadStore_Hint ldhint) {
  lfetch(PR0, addr, lftype, ldhint);
}

inline void Assembler::lfetch(PredicateRegister qp, Register addr, Register adjust,
                              LinePrefetch_Type lftype, LoadStore_Hint ldhint) {
  M14 inst(lftype, ldhint, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::lfetch(                      Register addr, Register adjust,
                              LinePrefetch_Type lftype, LoadStore_Hint ldhint) {
  lfetch(PR0, addr, adjust, lftype, ldhint);
}

inline void Assembler::lfetch(PredicateRegister qp, Register addr, int adjust,
                              LinePrefetch_Type lftype, LoadStore_Hint ldhint) {
  verify_signed_range(adjust, 9);
  M15 inst(lftype, ldhint, addr, adjust, qp);
  emit(inst);
}
inline void Assembler::lfetch(                      Register addr, int adjust,
                              LinePrefetch_Type lftype, LoadStore_Hint ldhint) {
  lfetch(PR0, addr, adjust, lftype, ldhint);
}


inline void Assembler::cmpxchg1(PredicateRegister qp, Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(sem | 0, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::cmpxchg1(                      Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  cmpxchg1(PR0, dst, addr, src, sem, ldhint);
}

inline void Assembler::cmpxchg2(PredicateRegister qp, Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(sem | 1, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::cmpxchg2(                      Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  cmpxchg2(PR0, dst, addr, src, sem, ldhint);
}

inline void Assembler::cmpxchg4(PredicateRegister qp, Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(sem | 2, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::cmpxchg4(                      Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  cmpxchg4(PR0, dst, addr, src, sem, ldhint);
}

inline void Assembler::cmpxchg8(PredicateRegister qp, Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(sem | 3, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::cmpxchg8(                      Register dst, Register addr, Register src,
                                Semaphore_Completer sem, LoadStore_Hint ldhint) {
  cmpxchg8(PR0, dst, addr, src, sem, ldhint);
}


inline void Assembler::xchg1(PredicateRegister qp, Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(0x8 | 0, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::xchg1(                      Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  xchg1(PR0, dst, addr, src, ldhint);
}

inline void Assembler::xchg2(PredicateRegister qp, Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(0x8 | 1, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::xchg2(                      Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  xchg2(PR0, dst, addr, src, ldhint);
}

inline void Assembler::xchg4(PredicateRegister qp, Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(0x8 | 2, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::xchg4(                      Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  xchg4(PR0, dst, addr, src, ldhint);
}

inline void Assembler::xchg8(PredicateRegister qp, Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  verify_load(ldhint);
  M16 inst(0x8 | 3, ldhint, dst, addr, src, qp);
  emit(inst);
}
inline void Assembler::xchg8(                      Register dst, Register addr, Register src,
                             LoadStore_Hint ldhint) {
  xchg8(PR0, dst, addr, src, ldhint);
}


inline void Assembler::fetchadd4(PredicateRegister qp, Register dst, Register addr, int inc,
                                 Semaphore_Completer sem, LoadStore_Hint ldhint) {
  verify_fetchadd(inc);
  verify_load(ldhint);
  M17 inst(sem | 2, ldhint, dst, addr, inc, qp);
  emit(inst);
}
inline void Assembler::fetchadd4(                      Register dst, Register addr, int inc,
                                 Semaphore_Completer sem, LoadStore_Hint ldhint) {
  fetchadd4(PR0, dst, addr, inc, sem, ldhint);
}

inline void Assembler::fetchadd8(PredicateRegister qp, Register dst, Register addr, int inc,
                                 Semaphore_Completer sem, LoadStore_Hint ldhint) {
  verify_fetchadd(inc);
  verify_load(ldhint);
  M17 inst(sem | 3, ldhint, dst, addr, inc, qp);
  emit(inst);
}
inline void Assembler::fetchadd8(                      Register dst, Register addr, int inc,
                                 Semaphore_Completer sem, LoadStore_Hint ldhint) {
  fetchadd8(PR0, dst, addr, inc, sem, ldhint);
}


inline void Assembler::setfsig(PredicateRegister qp, FloatRegister dst, Register src) {
  M18 inst(0x1C, dst, src, qp);
  emit(inst);
}
inline void Assembler::setfsig(                      FloatRegister dst, Register src) {
  setfsig(PR0, dst, src);
}

inline void Assembler::setfexp(PredicateRegister qp, FloatRegister dst, Register src) {
  M18 inst(0x1D, dst, src, qp);
  emit(inst);
}
inline void Assembler::setfexp(                      FloatRegister dst, Register src) {
  setfexp(PR0, dst, src);
}

inline void Assembler::setfs(PredicateRegister qp, FloatRegister dst, Register src) {
  M18 inst(0x1E, dst, src, qp);
  emit(inst);
}
inline void Assembler::setfs(                      FloatRegister dst, Register src) {
  setfs(PR0, dst, src);
}

inline void Assembler::setfd(PredicateRegister qp, FloatRegister dst, Register src) {
  M18 inst(0x1F, dst, src, qp);
  emit(inst);
}
inline void Assembler::setfd(                      FloatRegister dst, Register src) {
  setfd(PR0, dst, src);
}
 

inline void Assembler::getfsig(PredicateRegister qp, Register dst, FloatRegister src) {
  M19 inst(0x1C, dst, src, qp);
  emit(inst);
}
inline void Assembler::getfsig(                      Register dst, FloatRegister src) {
  getfsig(PR0, dst, src);
}

inline void Assembler::getfexp(PredicateRegister qp, Register dst, FloatRegister src) {
  M19 inst(0x1D, dst, src, qp);
  emit(inst);
}
inline void Assembler::getfexp(                      Register dst, FloatRegister src) {
  getfexp(PR0, dst, src);
}

inline void Assembler::getfs(PredicateRegister qp, Register dst, FloatRegister src) {
  M19 inst(0x1E, dst, src, qp);
  emit(inst);
}
inline void Assembler::getfs(                      Register dst, FloatRegister src) {
  getfs(PR0, dst, src);
}

inline void Assembler::getfd(PredicateRegister qp, Register dst, FloatRegister src) {
  M19 inst(0x1F, dst, src, qp);
  emit(inst);
}
inline void Assembler::getfd(                      Register dst, FloatRegister src) {
  getfd(PR0, dst, src);
}


inline void Assembler::chksm(PredicateRegister qp, Register src, Label& target) {
  M20 inst(src, 0, qp);
  emit(inst, &target);
}
inline void Assembler::chksm(                      Register src, Label& target) {
  chksm(PR0, src, target);
}

inline void Assembler::chks(PredicateRegister qp, FloatRegister src, Label& target) {
  M21 inst(src, 0, qp);
  emit(inst, &target);
}
inline void Assembler::chks(                      FloatRegister src, Label& target) {
  chks(PR0, src, target);
}

inline void Assembler::chka(PredicateRegister qp, Register src, Cache_Hint hint, Label& target) {
  M22 inst(hint, src, 0, qp);
  emit(inst, &target);
}
inline void Assembler::chka(                      Register src, Cache_Hint hint, Label& target) {
  chka(PR0, src, hint, target);
}

inline void Assembler::chka(PredicateRegister qp, FloatRegister src, Cache_Hint hint, Label& target) {
  M23 inst(hint, src, 0, qp);
  emit(inst, &target);
}
inline void Assembler::chka(                      FloatRegister src, Cache_Hint hint, Label& target) {
  chka(PR0, src, hint, target);
}


inline void Assembler::invala(PredicateRegister qp) {
  M24 inst(0, 1, qp);
  emit(inst);
}

inline void Assembler::fwb(PredicateRegister qp) {
  M24 inst(0, 2, qp);
  emit(inst);
}

inline void Assembler::mf(PredicateRegister qp) {
  M24 inst(2, 2, qp);
  emit(inst);
}

inline void Assembler::mfa(PredicateRegister qp) {
  M24 inst(3, 2, qp);
  emit(inst);
}

inline void Assembler::srlzd(PredicateRegister qp) {
  M24 inst(0, 3, qp);
  emit(inst);
  flush_bundle();
}

inline void Assembler::srlzi(PredicateRegister qp) {
  M24 inst(1, 3, qp);
  emit(inst);
  flush_bundle();
}

inline void Assembler::synci(PredicateRegister qp) {
  M24 inst(3, 3, qp);
  emit(inst);
  flush_bundle();
}


inline void Assembler::flushrs(PredicateRegister qp) {
  flush_bundle();
  M25 inst(0xC, qp);
  emit(inst);
  flush_bundle();
}

inline void Assembler::loadrs(PredicateRegister qp) {
  flush_bundle();
  M25 inst(0xA, qp);
  emit(inst);
  flush_bundle();
}


inline void Assembler::invalae(PredicateRegister qp, Register src) {
  M26 inst(src, qp);
  emit(inst);
}
inline void Assembler::invalae(                      Register src) {
  invalae(PR0, src);
}

inline void Assembler::invalae(PredicateRegister qp, FloatRegister src) {
  M27 inst(src, qp);
  emit(inst);
}
inline void Assembler::invalae(                      FloatRegister src) {
  invalae(PR0, src);
}


inline void Assembler::fc(PredicateRegister qp, Register addr) {
  M28 inst(0x30, addr, qp);
  emit(inst);
}
inline void Assembler::fc(                      Register addr) {
  fc(PR0, addr);
}

inline void Assembler::ptce(PredicateRegister qp, Register addr) {
  M28 inst(0x34, addr, qp);
  emit(inst);
}
inline void Assembler::ptce(                      Register addr) {
  ptce(PR0, addr);
}


inline void Assembler::movm(PredicateRegister qp, ApplicationRegister dst, Register src) {
  M29 inst(dst, src, qp);
  emit(inst);
}
inline void Assembler::movm(                      ApplicationRegister dst, Register src) {
  movm(PR0, dst, src);
}

inline void Assembler::movm(PredicateRegister qp, ApplicationRegister dst, int imm8) {
  verify_signed_range(imm8, 8);
  M30 inst(dst, imm8, qp);
  emit(inst);
}
inline void Assembler::movm(                      ApplicationRegister dst, int imm8) {
  movm(PR0, dst, imm8);
}

inline void Assembler::movm(PredicateRegister qp, Register dst, ApplicationRegister src) {
  M31 inst(dst, src, qp);
  emit(inst);
}
inline void Assembler::movm(                      Register dst, ApplicationRegister src) {
  movm(PR0, dst, src);
}


inline void Assembler::alloc(Register saved_pfs,
                             uint inputs, uint locals, uint outputs, uint rotates) {
  verify_alloc(inputs, locals, outputs, rotates);
  M34 inst(saved_pfs, inputs, locals, outputs, rotates);
  eog();  // alloc must be first instruction in instruction group
  emit(inst);
  eog();  // Only present way to do this is to make it _only_ instruction in group
}


inline void Assembler::mov_to_psrum(PredicateRegister qp, Register src) {
  M35 inst(0x29, src, qp);
  emit(inst);
}
inline void Assembler::mov_to_psrum(                      Register src) {
  mov_to_psrum(PR0, src);
}

inline void Assembler::mov_from_psrum(PredicateRegister qp, Register dst) {
  M36 inst(0x21, dst, qp);
  emit(inst);
}
inline void Assembler::mov_from_psrum(                      Register dst) {
  mov_from_psrum(PR0, dst);
}


inline void Assembler::breakm(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  M37 inst(0, imm21, qp);
  emit(inst);
}
inline void Assembler::breakm(                      uint imm21) {
  breakm(PR0, imm21);
}

inline void Assembler::nopm(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  M37 inst(1, imm21, qp);
  emit(inst);
}
inline void Assembler::nopm(                      uint imm21) {
  nopm(PR0, imm21);
}


inline void Assembler::br(PredicateRegister qp, address target, relocInfo::relocType rtype,
                          Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B1 inst(0, bwh, ph, dh, 0, qp);
  emit(inst, target, rtype);
}
inline void Assembler::br(                      address target, relocInfo::relocType rtype,
                          Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  br(PR0, target, rtype, bwh, ph, dh);
}

inline void Assembler::br(PredicateRegister qp, Label& target,
                          Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B1 inst(0, bwh, ph, dh, 0, qp);
  emit(inst, &target);
}
inline void Assembler::br(                      Label& target,
                          Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  br(PR0, target, bwh, ph, dh);
}


inline void Assembler::wexit(PredicateRegister qp, Label& target,
                             Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B1 inst(2, bwh, ph, dh, 0, qp);
  emit(inst, &target);
}
inline void Assembler::wexit(                      Label& target,
                             Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  wexit(PR0, target, bwh, ph, dh);
}

inline void Assembler::wtop(PredicateRegister qp, Label& target,
                            Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B1 inst(3, bwh, ph, dh, 0, qp);
  emit(inst, &target);
}
inline void Assembler::wtop(                      Label& target,
                            Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  wtop(PR0, target, bwh, ph, dh);
}

inline void Assembler::cloop(Label& target,
                             Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B2 inst(5, bwh, ph, dh, 0);
  emit(inst, &target);
}

inline void Assembler::cexit(Label& target,
                             Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B2 inst(6, bwh, ph, dh, 0);
  emit(inst, &target);
}

inline void Assembler::ctop(Label& target,
                            Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B2 inst(7, bwh, ph, dh, 0);
  emit(inst, &target);
}


inline void Assembler::call(PredicateRegister qp, address target, relocInfo::relocType rtype,
                            BranchRegister link, Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B3 inst(bwh, ph, dh, link, 0, qp);
  emit(inst, target, rtype);
  flush_bundle();
}
inline void Assembler::call(                      address target, relocInfo::relocType rtype,
                            BranchRegister link, Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  call(PR0, target, rtype, link, bwh, ph, dh);
}

inline void Assembler::call(PredicateRegister qp, Label& target,
                            BranchRegister link, Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B3 inst(bwh, ph, dh, link, 0, qp);
  emit(inst, &target);
  flush_bundle();
}
inline void Assembler::call(                      Label& target,
                            BranchRegister link, Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  call(PR0, target, link, bwh, ph, dh);
}


inline void Assembler::br(PredicateRegister qp, BranchRegister target,
                          Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B4 inst(0x20, 0, bwh, ph, dh, target, qp);
  emit(inst);
}
inline void Assembler::br(                      BranchRegister target,
                          Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  br(PR0, target, bwh, ph, dh);
}

inline void Assembler::ret(PredicateRegister qp, BranchRegister target,
                           Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B4 inst(0x21, 4, bwh, ph, dh, target, qp);
  emit(inst);
  flush_bundle();
}
inline void Assembler::ret(                      BranchRegister target,
                           Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  ret(PR0, target, bwh, ph, dh);
}


inline void Assembler::call(PredicateRegister qp, BranchRegister target,
                            BranchRegister link, Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  B5 inst(bwh, ph, dh, link, target, qp);
  emit(inst);
  flush_bundle();
}
inline void Assembler::call(                      BranchRegister target,
                            BranchRegister link, Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  call(PR0, target, link, bwh, ph, dh);
}


inline void Assembler::cover() {
  B8 inst(0x02);
  emit(inst);
}

inline void Assembler::clrrrb() {
  B8 inst(0x04);
  emit(inst);
}

inline void Assembler::clrrrbpr() {
  B8 inst(0x05);
  emit(inst);
}

inline void Assembler::epc() {
  B8 inst(0x10);
  emit(inst);
}


inline void Assembler::breakb(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  B9 inst(0, imm21, qp);
  emit(inst);
}
inline void Assembler::breakb(                      uint imm21) {
  breakb(PR0, imm21);
}

inline void Assembler::nopb(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  B9 inst(2, imm21, qp);
  emit(inst);
}
inline void Assembler::nopb(                      uint imm21) {
  nopb(PR0, imm21);
}


inline void Assembler::fma(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                           FloatStatusField sf) {
  F1 inst(0x8, 0, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fma(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                           FloatStatusField sf) {
  fma(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fmas(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  F1 inst(0x8, 1, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fmas(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  fmas(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fmad(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  F1 inst(0x9, 0, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fmad(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  fmad(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fpma(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  F1 inst(0x9, 1, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fpma(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  fpma(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fms(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                           FloatStatusField sf) {
  F1 inst(0xA, 0, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fms(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                           FloatStatusField sf) {
  fms(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fmss(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  F1 inst(0xA, 1, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fmss(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  fmss(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fmsd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  F1 inst(0xB, 0, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fmsd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  fmsd(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fpms(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  F1 inst(0xB, 1, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fpms(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  fpms(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fnma(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  F1 inst(0xC, 0, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fnma(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                            FloatStatusField sf) {
  fnma(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fnmas(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                             FloatStatusField sf) {
  F1 inst(0xC, 1, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fnmas(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                             FloatStatusField sf) {
  fnmas(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fnmad(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                             FloatStatusField sf) {
  F1 inst(0xD, 0, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fnmad(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                             FloatStatusField sf) {
  fnmad(PR0, dst, src1, src2, src3, sf);
}

inline void Assembler::fpnma(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                             FloatStatusField sf) {
  F1 inst(0xD, 1, sf, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fpnma(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3,
                             FloatStatusField sf) {
  fpnma(PR0, dst, src1, src2, src3, sf);
}


inline void Assembler::xmal(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  F2 inst(0, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::xmal(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  xmal(PR0, dst, src1, src2, src3);
}

inline void Assembler::xmah(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  F2 inst(3, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::xmah(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  xmah(PR0, dst, src1, src2, src3);
}

inline void Assembler::xmahu(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  F2 inst(2, dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::xmahu(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  xmahu(PR0, dst, src1, src2, src3);
}


inline void Assembler::fselect(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  F3 inst(dst, src1, src2, src3, qp);
  emit(inst);
}
inline void Assembler::fselect(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  fselect(PR0, dst, src1, src2, src3);
}


inline void Assembler::frcpa(PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                             FloatStatusField sf) {
  F6 inst(0x0, sf, dst, p, src1, src2, qp);
  emit(inst);
}
inline void Assembler::frcpa(                      FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                             FloatStatusField sf) {
  frcpa(PR0, dst, p, src1, src2, sf);
}

inline void Assembler::fprcpa(PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                              FloatStatusField sf) {
  F6 inst(0x1, sf, dst, p, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fprcpa(                      FloatRegister dst, PredicateRegister p, FloatRegister src1, FloatRegister src2,
                              FloatStatusField sf) {
  fprcpa(PR0, dst, p, src1, src2, sf);
}


inline void Assembler::frsqrta(PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src,
                               FloatStatusField sf) {
  F7 inst(0x0, sf, dst, p, src, qp);
  emit(inst);
}
inline void Assembler::frsqrta(                      FloatRegister dst, PredicateRegister p, FloatRegister src,
                               FloatStatusField sf) {
  frsqrta(PR0, dst, p, src, sf);
}

inline void Assembler::fprsqrta(PredicateRegister qp, FloatRegister dst, PredicateRegister p, FloatRegister src,
                                FloatStatusField sf) {
  F7 inst(0x1, sf, dst, p, src, qp);
  emit(inst);
}
inline void Assembler::fprsqrta(                      FloatRegister dst, PredicateRegister p, FloatRegister src,
                                FloatStatusField sf) {
  fprsqrta(PR0, dst, p, src, sf);
}


inline void Assembler::fmin(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                            FloatStatusField sf) {
  F8 inst(0x0, 0x14, sf, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmin(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                            FloatStatusField sf) {
  fmin(PR0, dst, src1, src2, sf);
}

inline void Assembler::fmax(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                            FloatStatusField sf) {
  F8 inst(0x0, 0x15, sf, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmax(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                            FloatStatusField sf) {
  fmax(PR0, dst, src1, src2, sf);
}

inline void Assembler::famin(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                             FloatStatusField sf) {
  F8 inst(0x0, 0x16, sf, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::famin(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                             FloatStatusField sf) {
  famin(PR0, dst, src1, src2, sf);
}

inline void Assembler::famax(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                             FloatStatusField sf) {
  F8 inst(0x0, 0x17, sf, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::famax(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                             FloatStatusField sf) {
  famax(PR0, dst, src1, src2, sf);
}


inline void Assembler::fmerges(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x10, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmerges(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fmerges(PR0, dst, src1, src2);
}

inline void Assembler::fmergens(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x11, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmergens(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fmergens(PR0, dst, src1, src2);
}

inline void Assembler::fmergese(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x12, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmergese(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fmergese(PR0, dst, src1, src2);
}

inline void Assembler::fmixlr(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x39, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmixlr(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fmixlr(PR0, dst, src1, src2);
}

inline void Assembler::fmixr(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x3A, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmixr(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fmixr(PR0, dst, src1, src2);
}

inline void Assembler::fmixl(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x3B, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fmixl(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fmixl(PR0, dst, src1, src2);
}

inline void Assembler::fsxtr(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x3C, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fsxtr(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fsxtr(PR0, dst, src1, src2);
}

inline void Assembler::fsxtl(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x3D, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fsxtl(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fsxtl(PR0, dst, src1, src2);
}

inline void Assembler::fpack(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x28, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fpack(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fpack(PR0, dst, src1, src2);
}

inline void Assembler::fswap(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x34, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fswap(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fswap(PR0, dst, src1, src2);
}

inline void Assembler::fswapnl(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x35, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fswapnl(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fswapnl(PR0, dst, src1, src2);
}

inline void Assembler::fswapnr(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x36, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fswapnr(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fswapnr(PR0, dst, src1, src2);
}

inline void Assembler::fand(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x2C, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fand(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fand(PR0, dst, src1, src2);
}

inline void Assembler::fandcm(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x2D, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fandcm(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fandcm(PR0, dst, src1, src2);
}

inline void Assembler::for3(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x2E, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::for3(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  for3(PR0, dst, src1, src2);
}

inline void Assembler::fxor(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x0, 0x2F, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fxor(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fxor(PR0, dst, src1, src2);
}

inline void Assembler::fpmerges(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x1, 0x10, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fpmerges(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fpmerges(PR0, dst, src1, src2);
}

inline void Assembler::fpmergens(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x1, 0x11, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fpmergens(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fpmergens(PR0, dst, src1, src2);
}

inline void Assembler::fpmergese(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  F9 inst(0x1, 0x12, dst, src1, src2, qp);
  emit(inst);
}
inline void Assembler::fpmergese(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  fpmergese(PR0, dst, src1, src2);
}


inline void Assembler::fcvtfx(PredicateRegister qp, FloatRegister dst, FloatRegister src, bool truncate,
                              FloatStatusField sf) {
  F10 inst(0x0, 0, truncate, sf, dst, src, qp);
  emit(inst);
}
inline void Assembler::fcvtfx(                      FloatRegister dst, FloatRegister src, bool truncate,
                              FloatStatusField sf) {
  fcvtfx(PR0, dst, src, truncate, sf);
}

inline void Assembler::fcvtfxu(PredicateRegister qp, FloatRegister dst, FloatRegister src, bool truncate,
                               FloatStatusField sf) {
  F10 inst(0x0, 1, truncate, sf, dst, src, qp);
  emit(inst);
}
inline void Assembler::fcvtfxu(                      FloatRegister dst, FloatRegister src, bool truncate,
                               FloatStatusField sf) {
  fcvtfxu(PR0, dst, src, truncate, sf);
}


inline void Assembler::fcvtxf(PredicateRegister qp, FloatRegister dst, FloatRegister src) {
  F11 inst(dst, src, qp);
  emit(inst);
}
inline void Assembler::fcvtxf(                      FloatRegister dst, FloatRegister src) {
  fcvtxf(PR0, dst, src);
}


inline void Assembler::fsetc(PredicateRegister qp, uint amask, uint omask, FloatStatusField sf) {
  F12 inst(sf, amask, omask, qp);
  emit(inst);
}
inline void Assembler::fsetc(uint amask, uint omask, FloatStatusField sf) {
  fsetc(PR0, amask, omask, sf);
}


inline void Assembler::fclrf(PredicateRegister qp, FloatStatusField sf) {
  F13 inst(sf, qp);
  emit(inst);
}
inline void Assembler::fclrf(FloatStatusField sf) {
  fclrf(PR0, sf);
}

inline void Assembler::fchkf(PredicateRegister qp, Label& target, FloatStatusField sf) {
  F14 inst(sf, 0, qp);
  emit(inst, &target);
}
inline void Assembler::fchkf(Label& target, FloatStatusField sf) {
  fchkf(PR0, target, sf);
}


inline void Assembler::breakf(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  F15 inst(0, imm21, qp);
  emit(inst);
}
inline void Assembler::breakf(                      uint imm21) {
  breakf(PR0, imm21);
}

inline void Assembler::nopf(PredicateRegister qp, uint imm21) {
  verify_unsigned_range(imm21, 21);
  F15 inst(1, imm21, qp);
  emit(inst);
}
inline void Assembler::nopf(                      uint imm21) {
  nopf(PR0, imm21);
}


inline void Assembler::breakx(PredicateRegister qp, uint64_t imm62) {
  verify_unsigned_range(imm62, 62);
  X1 inst(0, imm62, qp);
  emit(inst);
}
inline void Assembler::breakx(                      uint64_t imm62) {
  breakx(PR0, imm62);
}

inline void Assembler::nopx(PredicateRegister qp, uint64_t imm62) {
  verify_unsigned_range(imm62, 62);
  X1 inst(1, imm62, qp);
  emit(inst);
}
inline void Assembler::nopx(                      uint64_t imm62) {
  nopx(PR0, imm62);
}


inline void Assembler::movl(PredicateRegister qp, Register dst, int64_t imm64) {
  X2 inst(dst, imm64, qp);
  emit(inst);
}
inline void Assembler::movl(                      Register dst, int64_t imm64) {
  movl(PR0, dst, imm64);
}

inline void Assembler::mova(PredicateRegister qp, Register dst, address addr, relocInfo::relocType rtype) {
  X2 inst(dst, (int64_t)addr, qp);
  emit(inst, addr, rtype);
}
inline void Assembler::mova(                      Register dst, address addr, relocInfo::relocType rtype) {
  mova(PR0, dst, addr, rtype);
}


inline void Assembler::brl(PredicateRegister qp, Label& target,
                           Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  X3 inst(0, bwh, ph, dh, 0, qp);
  emit(inst, &target);
}
inline void Assembler::brl(                      Label& target,
                           Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  brl(PR0, target, bwh, ph, dh);
}


inline void Assembler::calll(PredicateRegister qp, Label& target, BranchRegister link,
                             Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  X4 inst(bwh, ph, dh, link, 0, qp);
  emit(inst, &target);
}
inline void Assembler::calll(                      Label& target, BranchRegister link,
                             Branch_Hint bwh, Prefetch_Hint ph, Cache_Hint dh) {
  calll(PR0, target, link, bwh, ph, dh);
}


//=============================================================================
// MacroAssembler
//=============================================================================

inline void MacroAssembler::add(PredicateRegister qp, Register dst, Register src1, Register src2) {
  Assembler::adda(qp, dst, src1, src2);
}
inline void MacroAssembler::add(                      Register dst, Register src1, Register src2) {
  add(PR0, dst, src1, src2);
}

inline void MacroAssembler::add(PredicateRegister qp, Register dst, int imm, Register src) {
  if (in_signed_range(imm, 14)) {
    adds(qp, dst, imm, src);
  } else {
    addl(qp, dst, imm, src);
  }
}
inline void MacroAssembler::add(Register dst, int imm, Register src) {
  add(PR0, dst, imm, src);
}


inline void MacroAssembler::add(PredicateRegister qp, Register dst, Register src, int imm) {
  add(qp, dst, imm, src);
}
inline void MacroAssembler::add(                      Register dst, Register src, int imm) {
  add(PR0, dst, src, imm);
}


inline void MacroAssembler::sub(PredicateRegister qp, Register dst, Register src1, Register src2) {
  Assembler::sub(qp,  dst, src1, src2);
}
inline void MacroAssembler::sub(                      Register dst, Register src1, Register src2) {
  Assembler::sub(PR0, dst, src1, src2);
}

inline void MacroAssembler::sub(PredicateRegister qp, Register dst, Register src,  int imm) {
  add(qp, dst, -imm, src);
}

inline void MacroAssembler::sub(                      Register dst, Register src,  int imm) {
  add(PR0, dst, -imm, src);
}


inline void MacroAssembler::and3(PredicateRegister qp, Register dst, Register src1, Register src2) {
  Assembler::and3(qp, dst, src1, src2);
}
inline void MacroAssembler::and3(                      Register dst, Register src1, Register src2) {
  and3(PR0, dst, src1, src2);
}

inline void MacroAssembler::and3(PredicateRegister qp, Register dst, int imm, Register src) {
  Assembler::and3(qp, dst, imm, src);
}
inline void MacroAssembler::and3(                      Register dst, int imm, Register src) {
  and3(PR0, dst, imm, src);
}

inline void MacroAssembler::and3(PredicateRegister qp, Register dst, Register src, int imm) {
  Assembler::and3(qp, dst, imm, src);
}
inline void MacroAssembler::and3(                      Register dst, Register src, int imm) {
  and3(PR0, dst, src, imm);
}


inline void MacroAssembler::andcm(PredicateRegister qp, Register dst, Register src1, Register src2) {
  Assembler::andcm(qp, dst, src1, src2);
}
inline void MacroAssembler::andcm(                      Register dst, Register src1, Register src2) {
  andcm(PR0, dst, src1, src2);
}

inline void MacroAssembler::andcm(PredicateRegister qp, Register dst, int imm, Register src) {
  Assembler::andcm(qp, dst, imm, src);
}
inline void MacroAssembler::andcm(                      Register dst, int imm, Register src) {
  andcm(PR0, dst, imm, src);
}

inline void MacroAssembler::andcm(PredicateRegister qp, Register dst, Register src, int imm) {
  Assembler::andcm(qp, dst, imm, src);
}
inline void MacroAssembler::andcm(                      Register dst, Register src, int imm) {
  andcm(PR0, dst, src, imm);
}


inline void MacroAssembler::or3(PredicateRegister qp, Register dst, Register src1, Register src2) {
  Assembler::or3(qp, dst, src1, src2);
}
inline void MacroAssembler::or3(                      Register dst, Register src1, Register src2) {
  or3(PR0, dst, src1, src2);
}

inline void MacroAssembler::or3(PredicateRegister qp, Register dst, int imm, Register src) {
  Assembler::or3(qp, dst, imm, src);
}
inline void MacroAssembler::or3(                      Register dst, int imm, Register src) {
  or3(PR0, dst, imm, src);
}

inline void MacroAssembler::or3(PredicateRegister qp, Register dst, Register src, int imm) {
  Assembler::or3(qp, dst, imm, src);
}
inline void MacroAssembler::or3(                      Register dst, Register src, int imm) {
  or3(PR0, dst, src, imm);
}


inline void MacroAssembler::xor3(PredicateRegister qp, Register dst, Register src1, Register src2) {
  Assembler::xor3(qp, dst, src1, src2);
}
inline void MacroAssembler::xor3(                      Register dst, Register src1, Register src2) {
  xor3(PR0, dst, src1, src2);
}

inline void MacroAssembler::xor3(PredicateRegister qp, Register dst, int imm, Register src) {
  Assembler::xor3(qp, dst, imm, src);
}
inline void MacroAssembler::xor3(                      Register dst, int imm, Register src) {
  xor3(PR0, dst, imm, src);
}

inline void MacroAssembler::xor3(PredicateRegister qp, Register dst, Register src, int imm) {
  Assembler::xor3(qp, dst, imm, src);
}
inline void MacroAssembler::xor3(                      Register dst, Register src, int imm) {
  xor3(PR0, dst, src, imm);
}

inline void MacroAssembler::sub(PredicateRegister qp, Register dst, int imm,       Register src) {
  Assembler::sub(qp, dst, imm, src);
}

inline void MacroAssembler::sub(                      Register dst, int imm,       Register src) {
  Assembler::sub(PR0, dst, imm, src);
}

inline void MacroAssembler::mov(PredicateRegister qp, Register dst, Register src) {
  if (dst == src) return;
  adds(qp, dst, 0, src);
}
inline void MacroAssembler::mov(                      Register dst, Register src) {
  mov(PR0, dst, src);
}

inline void MacroAssembler::mov(PredicateRegister qp, Register dst, int imm) {
  if (in_signed_range(imm, 14)) {
    adds(qp, dst, imm, GR0);
  } else if (dst->encoding() < 4 && in_signed_range(imm, 22)) {
    addl(qp, dst, imm, GR0);
  } else {
    movl(qp, dst, imm);
  }
}
inline void MacroAssembler::mov(                      Register dst, int imm) {
  mov(PR0, dst, imm);
}


inline void MacroAssembler::mov(PredicateRegister qp, ApplicationRegister dst, Register src) {
  if (dst == AR_PFS || dst == AR_LC || dst == AR_EC) {
    movi(qp, dst, src);
  } else {
    movm(qp, dst, src);
  }
}
inline void MacroAssembler::mov(                      ApplicationRegister dst, Register src) {
  mov(PR0, dst, src);
}

inline void MacroAssembler::mov(PredicateRegister qp, ApplicationRegister dst, int imm8) {
  if (dst == AR_PFS || dst == AR_LC || dst == AR_EC) {
    movi(qp, dst, imm8);
  } else {
    movm(qp, dst, imm8);
  }
}
inline void MacroAssembler::mov(                      ApplicationRegister dst, int imm8) {
  mov(PR0, dst, imm8);
}

inline void MacroAssembler::mov(PredicateRegister qp, Register dst, ApplicationRegister src) {
  if (src == AR_PFS || src == AR_LC || src == AR_EC) {
    movi(qp, dst, src);
  } else {
    movm(qp, dst, src);
  }
}
inline void MacroAssembler::mov(                      Register dst, ApplicationRegister src) {
  mov(PR0, dst, src);
}


inline void MacroAssembler::mov(PredicateRegister qp, BranchRegister dst, Register src, uint tag13,
                                Branch_Hint mwh, Importance_Hint ih) {
  Assembler::movi(qp, dst, src, tag13, mwh, ih);
}
inline void MacroAssembler::mov(                      BranchRegister dst, Register src, uint tag13,
                                Branch_Hint mwh, Importance_Hint ih) {
  mov(PR0, dst, src, tag13, mwh, ih);
}

inline void MacroAssembler::mov(PredicateRegister qp, BranchRegister dst, Register src, Label& tag,
                                Branch_Hint mwh, Importance_Hint ih) {
  Assembler::movi(qp, dst, src, tag, mwh, ih);
}
inline void MacroAssembler::mov(                      BranchRegister dst, Register src, Label& tag,
                                Branch_Hint mwh, Importance_Hint ih) {
  mov(PR0, dst, src, tag, mwh, ih);
}


inline void MacroAssembler::mov(PredicateRegister qp, Register dst, BranchRegister src) {
  Assembler::movi(qp, dst, src);
}
inline void MacroAssembler::mov(                      Register dst, BranchRegister src) {
  mov(PR0, dst, src);
}


inline void MacroAssembler::clr(PredicateRegister qp, Register dst) {
  mov(qp, dst, 0);
}
inline void MacroAssembler::clr(                      Register dst) {
  clr(PR0, dst);
}


inline void MacroAssembler::shru(PredicateRegister qp, Register dst, Register src, uint count) {
  extru(qp, dst, src, count, 64-count);
}

inline void MacroAssembler::shru(                      Register dst, Register src, uint count) {
  shru(PR0, dst, src, count);
}

inline void MacroAssembler::shru(PredicateRegister qp, Register dst, Register src, Register count) {
  Assembler::shru(qp, dst, src, count);
}

inline void MacroAssembler::shru(                      Register dst, Register src, Register count) {
  shru(PR0, dst, src, count);
}

inline void MacroAssembler::shr(PredicateRegister qp, Register dst, Register src, uint count) {
  extr(qp, dst, src, count, 64-count);
}

inline void MacroAssembler::shr(                      Register dst, Register src, uint count) {
  shr(PR0, dst, src, count);
}

inline void MacroAssembler::shr(PredicateRegister qp, Register dst, Register src, Register count) {
  Assembler::shr(qp, dst, src, count);
}

inline void MacroAssembler::shr(                      Register dst, Register src, Register count) {
  shr(PR0, dst, src, count);
}

inline void MacroAssembler::shl(PredicateRegister qp, Register dst, Register src, uint count) {
  depz(qp, dst, src, count, 64-count);
}

inline void MacroAssembler::shl(                      Register dst, Register src, uint count) {
  shl(PR0, dst, src, count);
}

inline void MacroAssembler::shl(PredicateRegister qp, Register dst, Register src, Register count) {
  Assembler::shl(qp, dst, src, count);
}

inline void MacroAssembler::shl(                      Register dst, Register src, Register count) {
  shl(PR0, dst, src, count);
}


inline void MacroAssembler::shr4u(PredicateRegister qp, Register dst, Register src, Register count) {
  Assembler::zxt4(qp, dst, src);
  Assembler::shru(qp, dst, dst, count);
}
inline void MacroAssembler::shr4u(                      Register dst, Register src, Register count) {
  shr4u(PR0, dst, src, count);
}

inline void MacroAssembler::shr4u(PredicateRegister qp, Register dst, Register src, uint count) {
  extru(qp, dst, src, count, 32-count);
}
inline void MacroAssembler::shr4u(                      Register dst, Register src, uint count) {
  shr4u(PR0, dst, src, count);
}

inline void MacroAssembler::shr4(PredicateRegister qp, Register dst, Register src, Register count) {
  Assembler::sxt4(qp, dst, src);
  Assembler::shr (qp, dst, dst, count);
}
inline void MacroAssembler::shr4(                      Register dst, Register src, Register count) {
  shr4(PR0, dst, src, count);
}

inline void MacroAssembler::shr4(PredicateRegister qp, Register dst, Register src, uint count) {
  extr(qp, dst, src, count, 32-count);
}
inline void MacroAssembler::shr4(                      Register dst, Register src, uint count) {
  shr4(PR0, dst, src, count);
}


inline void MacroAssembler::ld1s(PredicateRegister qp, Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld1(qp, dst, addr, ldtype, ldhint);
  sxt1(qp, dst, dst);
}
inline void MacroAssembler::ld1s(                      Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld1s(PR0, dst, addr, ldtype, ldhint);
}

inline void MacroAssembler::ld2s(PredicateRegister qp, Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld2(qp, dst, addr, ldtype, ldhint);
  sxt2(qp, dst, dst);
}
inline void MacroAssembler::ld2s(                      Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld2s(PR0, dst, addr, ldtype, ldhint);
}

inline void MacroAssembler::ld4s(PredicateRegister qp, Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld4(qp, dst, addr, ldtype, ldhint);
  sxt4(qp, dst, dst);
}
inline void MacroAssembler::ld4s(                      Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld4s(PR0, dst, addr, ldtype, ldhint);
}

inline void MacroAssembler::ld8s(PredicateRegister qp, Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld8(qp, dst, addr, ldtype, ldhint);
}
inline void MacroAssembler::ld8s(                      Register dst, Register addr,
                                 LoadStore_Type ldtype, LoadStore_Hint ldhint) {
  ld8s(PR0, dst, addr, ldtype, ldhint);
}


inline void MacroAssembler::fabs(PredicateRegister qp, FloatRegister dst, FloatRegister src) {
  fmerges(qp, dst, FR0, src);
}
inline void MacroAssembler::fabs(                      FloatRegister dst, FloatRegister src) {
  fabs(PR0, dst, src);
}


inline void MacroAssembler::fadd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                 FloatStatusField sf) {
  fma(qp, dst, src1, FR1, src2, sf);
}
inline void MacroAssembler::fadd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                 FloatStatusField sf) {
  fadd(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::fadds(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmas(qp, dst, src1, FR1, src2, sf);
}
inline void MacroAssembler::fadds(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fadds(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::faddd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmad(qp, dst, src1, FR1, src2, sf);
}
inline void MacroAssembler::faddd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  faddd(PR0, dst, src1, src2, sf);
}


inline void MacroAssembler::fsub(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                 FloatStatusField sf) {
  fms(qp, dst, src1, FR1, src2, sf);
}
inline void MacroAssembler::fsub(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                 FloatStatusField sf) {
  fsub(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::fsubs(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmss(qp, dst, src1, FR1, src2, sf);
}
inline void MacroAssembler::fsubs(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fsubs(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::fsubd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmsd(qp, dst, src1, FR1, src2, sf);
}
inline void MacroAssembler::fsubd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fsubd(PR0, dst, src1, src2, sf);
}


inline void MacroAssembler::fcvtxufs(PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fmas(qp, dst, src, FR1, FR0, sf);
}
inline void MacroAssembler::fcvtxufs(                      FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fcvtxufs(PR0, dst, src, sf);
}

inline void MacroAssembler::fcvtxufd(PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fmad(qp, dst, src, FR1, FR0, sf);
}
inline void MacroAssembler::fcvtxufd(                      FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fcvtxufd(PR0, dst, src, sf);
}


inline void MacroAssembler::fmpy(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                 FloatStatusField sf) {
  fma(qp, dst, src1, src2, FR0, sf);
}
inline void MacroAssembler::fmpy(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                 FloatStatusField sf) {
  fmpy(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::fmpys(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmas(qp, dst, src1, src2, FR0, sf);
}
inline void MacroAssembler::fmpys(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmpys(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::fmpyd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmad(qp, dst, src1, src2, FR0, sf);
}
inline void MacroAssembler::fmpyd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fmpyd(PR0, dst, src1, src2, sf);
}


inline void MacroAssembler::fnmpy(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fnma(qp, dst, src1, src2, FR0, sf);
}
inline void MacroAssembler::fnmpy(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                  FloatStatusField sf) {
  fnmpy(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::fnmpys(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                   FloatStatusField sf) {
  fnmas(qp, dst, src1, src2, FR0, sf);
}
inline void MacroAssembler::fnmpys(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                   FloatStatusField sf) {
  fnmpys(PR0, dst, src1, src2, sf);
}

inline void MacroAssembler::fnmpyd(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                   FloatStatusField sf) {
  fnmad(qp, dst, src1, src2, FR0, sf);
}
inline void MacroAssembler::fnmpyd(                      FloatRegister dst, FloatRegister src1, FloatRegister src2,
                                   FloatStatusField sf) {
  fnmpyd(PR0, dst, src1, src2, sf);
}


inline void MacroAssembler::fneg(PredicateRegister qp, FloatRegister dst, FloatRegister src) {
  fmergens(qp, dst, src, src);
}
inline void MacroAssembler::fneg(                      FloatRegister dst, FloatRegister src) {
  fneg(PR0, dst, src);
}

inline void MacroAssembler::fnegabs(PredicateRegister qp, FloatRegister dst, FloatRegister src) {
  fmergens(qp, dst, FR0, src);
}
inline void MacroAssembler::fnegabs(                      FloatRegister dst, FloatRegister src) {
  fnegabs(PR0, dst, src);
}


inline void MacroAssembler::fnorm(PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fma(qp, dst, src, FR1, FR0, sf);
}
inline void MacroAssembler::fnorm(                      FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fnorm(PR0, dst, src, sf);
}

inline void MacroAssembler::fnorms(PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fmas(qp, dst, src, FR1, FR0, sf);
}
inline void MacroAssembler::fnorms(                      FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fnorms(PR0, dst, src, sf);
}

inline void MacroAssembler::fnormd(PredicateRegister qp, FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fmad(qp, dst, src, FR1, FR0, sf);
}
inline void MacroAssembler::fnormd(                      FloatRegister dst, FloatRegister src, FloatStatusField sf) {
  fnormd(PR0, dst, src, sf);
}


inline void MacroAssembler::mov(PredicateRegister qp, FloatRegister dst, FloatRegister src) {
  if (dst == src) return;
  fmerges(qp, dst, src, src);
}
inline void MacroAssembler::mov(                      FloatRegister dst, FloatRegister src) {
  mov(PR0, dst, src);
}


inline void MacroAssembler::xmalu(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  xmal(qp, dst, src1, src2, src3);
}
inline void MacroAssembler::xmalu(                      FloatRegister dst, FloatRegister src1, FloatRegister src2, FloatRegister src3) {
  xmalu(PR0, dst, src1, src2, src3);
}
  
inline void MacroAssembler::xmpy(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmal(qp, dst, src1, src2, FR0);
}
inline void MacroAssembler::xmpy(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmpy(PR0, dst, src1, src2);
}
  
inline void MacroAssembler::xmpyl(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmal(qp, dst, src1, src2, FR0);
}
inline void MacroAssembler::xmpyl(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmpyl(PR0, dst, src1, src2);
}
  
inline void MacroAssembler::xmpylu(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmal(qp, dst, src1, src2, FR0);
}
inline void MacroAssembler::xmpylu(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmpylu(PR0, dst, src1, src2);
}
  
inline void MacroAssembler::xmpyh(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmah(qp, dst, src1, src2, FR0);
}
inline void MacroAssembler::xmpyh(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmpyh(PR0, dst, src1, src2);
}
  
inline void MacroAssembler::xmpyhu(PredicateRegister qp, FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmahu(qp, dst, src1, src2, FR0);
}
inline void MacroAssembler::xmpyhu(                      FloatRegister dst, FloatRegister src1, FloatRegister src2) {
  xmpyhu(PR0, dst, src1, src2);
}
