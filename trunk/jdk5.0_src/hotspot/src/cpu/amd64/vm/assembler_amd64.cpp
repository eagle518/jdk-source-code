#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)assembler_amd64.cpp	1.23 04/03/19 12:30:50 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

#include "incls/_precompiled.incl"
#include "incls/_assembler_amd64.cpp.incl"

// Implementation of Address
Address::Address(address target, relocInfo::relocType rtype)
{
  _base = noreg;
  _index = noreg;
  _scale = no_scale;
  _disp = 0;
  _target = target;
  switch (rtype) {
  case relocInfo::external_word_type:
    _rspec = external_word_Relocation::spec(target);
    break;
  case relocInfo::internal_word_type:
    _rspec = internal_word_Relocation::spec(target);
    break;
  case relocInfo::opt_virtual_call_type:
    _rspec = opt_virtual_call_Relocation::spec();
    break;
  case relocInfo::static_call_type:
    _rspec = static_call_Relocation::spec();
    break;
  case relocInfo::runtime_call_type:
    _rspec = runtime_call_Relocation::spec();
    break;
  case relocInfo::none:
    break;
  default:
    ShouldNotReachHere();
    break;
  }
}

// Implementation of Displacement

// Using me, bind instruction to be backpatched (addr in unbound label
// L) to position in pos argument, using assembler masm.
void Displacement::bind(Label &L, int pos, AbstractAssembler* masm) 
{
  int fixup_pos = L.pos();
  int imm32 = 0;
  switch (type()) {
  case call:
    assert(masm->byte_at(fixup_pos - 1) == 0xE8, "call expected");
    imm32 = pos - (fixup_pos + sizeof(int));
    break;
  case absolute_jump:
    assert(masm->byte_at(fixup_pos - 1) == 0xE9, "jmp expected");
    imm32 = pos - (fixup_pos + sizeof(int));
    break;
  case conditional_jump:
    assert(masm->byte_at(fixup_pos - 2) == 0x0F, "jcc expected");
    assert(masm->byte_at(fixup_pos - 1) == (0x80 | info()), "jcc expected");
    imm32 = pos - (fixup_pos + sizeof(int));
    break;
  default:
    ShouldNotReachHere();
  }
  masm->long_at_put(fixup_pos, imm32);
  next(L);
}

// Implementation of Assembler
int AbstractAssembler::code_fill_byte() 
{
  return (u_char)'\xF4'; // hlt
}

void Assembler::emit_data(jint data, 
                          relocInfo::relocType rtype, 
                          int format) 
{
  if (rtype == relocInfo::none) {
    emit_long(data);
  } else {
    emit_data(data, Relocation::spec_simple(rtype), format);
  }
}

void Assembler::emit_data(jint data,
                          RelocationHolder const& rspec,
                          int format)
{
  assert(imm64_operand == 0, "default format must be imm64 in this file");
  assert(imm64_operand != format, "must not be imm64");
  assert(_inst_mark != NULL, "must be inside InstructionMark");
  // Do not use AbstractAssembler::relocate, which is not intended for
  // embedded words.  Instead, relocate to the enclosing instruction.
  code()->relocate(_inst_mark, rspec, format);
#ifdef ASSERT
  check_relocation(rspec, format);
#endif
  emit_long(data);
}

void Assembler::emit_data64(jlong data, 
                            relocInfo::relocType rtype, 
                            int format) 
{
  if (rtype == relocInfo::none) {
    emit_long64(data);
  } else {
    emit_data64(data, Relocation::spec_simple(rtype), format);
  }
}

void Assembler::emit_data64(jlong data,
                            RelocationHolder const& rspec,
                            int format)
{
  assert(imm64_operand == 0, "default format must be imm64 in this file");
  assert(imm64_operand == format, "must be imm64");
  assert(_inst_mark != NULL, "must be inside InstructionMark");
  // Do not use AbstractAssembler::relocate, which is not intended for
  // embedded words.  Instead, relocate to the enclosing instruction.
  code()->relocate(_inst_mark, rspec, format);
#ifdef ASSERT
  check_relocation(rspec, format);
#endif
  emit_long64(data);
}

void Assembler::emit_arith_b(int op1, int op2, Register dst, int imm8) 
{
  assert(isByte(op1) && isByte(op2), "wrong opcode");
  assert(isByte(imm8), "not a byte");
  assert((op1 & 0x01) == 0, "should be 8bit operation");
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    dstenc -= 8;
  }
  emit_byte(op1);
  emit_byte(op2 | dstenc);
  emit_byte(imm8);
}

void Assembler::emit_arith(int op1, int op2, Register dst, int imm32) 
{
  assert(isByte(op1) && isByte(op2), "wrong opcode");
  assert((op1 & 0x01) == 1, "should be 32bit operation");
  assert((op1 & 0x02) == 0, "sign-extension bit should not be set");
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    dstenc -= 8;
  }
  if (is8bit(imm32)) {
    emit_byte(op1 | 0x02); // set sign bit
    emit_byte(op2 | dstenc);
    emit_byte(imm32 & 0xFF);
  } else {
    emit_byte(op1);
    emit_byte(op2 | dstenc);
    emit_long(imm32);
  }
}

// immediate-to-memory forms
void Assembler::emit_arith_operand(int op1, 
                                   Register rm, Address adr,
                                   int imm32)
{
  assert((op1 & 0x01) == 1, "should be 32bit operation");
  assert((op1 & 0x02) == 0, "sign-extension bit should not be set");
  if (is8bit(imm32)) {
    emit_byte(op1 | 0x02); // set sign bit
    adr.fix_rip_relative_offset(1);
    emit_operand(rm, adr);
    emit_byte(imm32 & 0xFF);
  } else {
    emit_byte(op1);
    adr.fix_rip_relative_offset(4);
    emit_operand(rm, adr);
    emit_long(imm32);
  }
}


void Assembler::emit_arith(int op1, int op2, Register dst, Register src) 
{
  assert(isByte(op1) && isByte(op2), "wrong opcode");
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc >= 8) {
    dstenc -= 8;
  }
  if (srcenc >= 8) {
    srcenc -= 8;
  }
  emit_byte(op1);
  emit_byte(op2 | dstenc << 3 | srcenc);
}

void Assembler::emit_operand(Register reg, Register base, Register index,
                             Address::ScaleFactor scale, int disp,
                             address target,
                             RelocationHolder const& rspec) 
{
  relocInfo::relocType rtype = (relocInfo::relocType) rspec.type();
  int regenc = reg->encoding();
  if (regenc >= 8) { 
    regenc -= 8; 
  }
  if (base->is_valid()) {
    if (index->is_valid()) {
      assert(scale != Address::no_scale, "inconsistent address");
      int indexenc = index->encoding();
      if (indexenc >= 8) {
        indexenc -= 8;
      }
      int baseenc = base->encoding();
      if (baseenc >= 8) {
        baseenc -= 8;
      }
      // [base + index*scale + disp]
      if (disp == 0 && rtype == relocInfo::none  &&
          base != rbp && base != r13) {
        // [base + index*scale]
        // [00 reg 100][ss index base]
        assert(index != rsp, "illegal addressing mode");
        emit_byte(0x04 | regenc << 3);
        emit_byte(scale << 6 | indexenc << 3 | baseenc);
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [base + index*scale + imm8]
        // [01 reg 100][ss index base] imm8
	assert(index != rsp, "illegal addressing mode");
        emit_byte(0x44 | regenc << 3);
        emit_byte(scale << 6 | indexenc << 3 | baseenc);
        emit_byte(disp & 0xFF);
      } else {
        // [base + index*scale + disp32]
        // [10 reg 100][ss index base] disp32
	assert(index != rsp, "illegal addressing mode");
	emit_byte(0x84 | regenc << 3);
        emit_byte(scale << 6 | indexenc << 3 | baseenc);
	emit_data(disp, rspec, disp32_operand);
      }
    } else if (base == rsp || base == r12) {
      // [rsp + disp]
      if (disp == 0 && rtype == relocInfo::none) {
        // [rsp]
        // [00 reg 100][00 100 100]
        emit_byte(0x04 | regenc << 3);
        emit_byte(0x24);
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [rsp + imm8]
        // [01 reg 100][00 100 100] disp8
        emit_byte(0x44 | regenc << 3);
        emit_byte(0x24);
        emit_byte(disp & 0xFF);
      } else {
        // [rsp + imm32]
        // [10 reg 100][00 100 100] disp32
	emit_byte(0x84 | regenc << 3);
        emit_byte(0x24);
	emit_data(disp, rspec, disp32_operand);
      }
    } else {
      // [base + disp]
      assert(base != rsp && base != r12, "illegal addressing mode");
      int baseenc = base->encoding();
      if (baseenc >= 8) {
        baseenc -= 8;
      }
      if (disp == 0 && rtype == relocInfo::none &&
          base != rbp && base != r13) {
        // [base]
        // [00 reg base]
        emit_byte(0x00 | regenc << 3 | baseenc);
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [base + disp8]
        // [01 reg base] disp8
        emit_byte(0x40 | regenc << 3 | baseenc);
        emit_byte(disp & 0xFF);
      } else {
        // [base + disp32]
        // [10 reg base] disp32
        emit_byte(0x80 | regenc << 3 | baseenc);
        emit_data(disp, rspec, disp32_operand);
      }
    }
  } else {
    if (index->is_valid()) {
      assert(scale != Address::no_scale, "inconsistent address");
      int indexenc = index->encoding();
      if (indexenc >= 8) {
        indexenc -= 8;
      }
      // [index*scale + disp]
      // [00 reg 100][ss index 101] disp32
      assert(index != rsp, "illegal addressing mode");
      emit_byte(0x04 | regenc << 3);
      emit_byte(scale << 6 | indexenc << 3 | 0x05);
      emit_data(disp, rspec, disp32_operand);
    } else {
      // [disp] RIP-RELATIVE
      // [00 reg 101] disp32
      emit_byte(0x05 | regenc << 3);
      intptr_t disp = (intptr_t) target - ((intptr_t) _code_pos + sizeof(int));
      assert(disp == (intptr_t)(int32_t)disp,
             "must be 32bit offset (RIP relative address)");
      emit_data((int) disp, rspec, disp32_operand);
    }
  }
}

void Assembler::emit_operand(FloatRegister reg, Register base, Register index,
                             Address::ScaleFactor scale, int disp,
                             address target,
                             RelocationHolder const& rspec) 
{
  relocInfo::relocType rtype = (relocInfo::relocType) rspec.type();
  int regenc = reg->encoding();
  if (regenc >= 8) { 
    regenc -= 8; 
  }
  if (base->is_valid()) {
    if (index->is_valid()) {
      assert(scale != Address::no_scale, "inconsistent address");
      int indexenc = index->encoding();
      if (indexenc >= 8) {
        indexenc -= 8;
      }
      int baseenc = base->encoding();
      if (baseenc >= 8) {
        baseenc -= 8;
      }
      // [base + index*scale + disp]
      if (disp == 0 && rtype == relocInfo::none  &&
          base != rbp && base != r13) {
        // [base + index*scale]
        // [00 reg 100][ss index base]
        assert(index != rsp, "illegal addressing mode");
        emit_byte(0x04 | regenc << 3);
        emit_byte(scale << 6 | indexenc << 3 | baseenc);
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [base + index*scale + disp8]
        // [01 reg 100][ss index base] disp8
	assert(index != rsp, "illegal addressing mode");
        emit_byte(0x44 | regenc << 3);
        emit_byte(scale << 6 | indexenc << 3 | baseenc);
        emit_byte(disp & 0xFF);
      } else {
        // [base + index*scale + disp32]
        // [10 reg 100][ss index base] disp32
	assert(index != rsp, "illegal addressing mode");
	emit_byte(0x84 | regenc << 3);
        emit_byte(scale << 6 | indexenc << 3 | baseenc);
	emit_data(disp, rspec, disp32_operand);
      }
    } else if (base == rsp || base == r12) {
      // [rsp + disp]
      if (disp == 0 && rtype == relocInfo::none) {
        // [rsp]
        // [00 reg 100][00 100 100]
        emit_byte(0x04 | regenc << 3);
        emit_byte(0x24);
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [rsp + imm8]
        // [01 reg 100][00 100 100] disp8
        emit_byte(0x44 | regenc << 3);
        emit_byte(0x24);
        emit_byte(disp & 0xFF);
      } else {
        // [rsp + imm32]
        // [10 reg 100][00 100 100] disp32
	emit_byte(0x84 | regenc << 3);
        emit_byte(0x24);
	emit_data(disp, rspec, disp32_operand);
      }
    } else {
      // [base + disp]
      assert(base != rsp && base != r12, "illegal addressing mode");
      int baseenc = base->encoding();
      if (baseenc >= 8) {
        baseenc -= 8;
      }
      if (disp == 0 && rtype == relocInfo::none &&
          base != rbp && base != r13) {
        // [base]
        // [00 reg base]
        emit_byte(0x00 | regenc << 3 | baseenc);
      } else if (is8bit(disp) && rtype == relocInfo::none) {
        // [base + imm8]
        // [01 reg base] disp8
        emit_byte(0x40 | regenc << 3 | baseenc);
        emit_byte(disp & 0xFF);
      } else {
        // [base + imm32]
        // [10 reg base] disp32
        emit_byte(0x80 | regenc << 3 | baseenc);
        emit_data(disp, rspec, disp32_operand);
      }
    }
  } else {
    if (index->is_valid()) {
      assert(scale != Address::no_scale, "inconsistent address");
      int indexenc = index->encoding();
      if (indexenc >= 8) {
        indexenc -= 8;
      }
      // [index*scale + disp]
      // [00 reg 100][ss index 101] disp32
      assert(index != rsp, "illegal addressing mode");
      emit_byte(0x04 | regenc << 3);
      emit_byte(scale << 6 | indexenc << 3 | 0x05);
      emit_data(disp, rspec, disp32_operand);
    } else {
      // [disp] RIP-RELATIVE
      // [00 reg 101] disp32
      emit_byte(0x05 | regenc << 3);
      intptr_t disp = (intptr_t) target - ((intptr_t) _code_pos + sizeof(int));
      assert(disp == (intptr_t)(int32_t)disp,
             "must be 32bit offset (RIP relative address)");
      emit_data((int) disp, rspec, disp32_operand);
    }
  }
}

// Secret local extension to Assembler::WhichOperand:
#define end_pc_operand (_WhichOperand_limit)

address Assembler::locate_operand(address inst, WhichOperand which) {
  // Decode the given instruction, and return the address of
  // an embedded 32-bit operand word.

  // If "which" is disp32_operand, selects the displacement portion
  // of an effective address specifier.
  // If "which" is imm64_operand, selects the trailing immediate constant.
  // If "which" is call32_operand, selects the displacement of a call or jump.
  // Caller is responsible for ensuring that there is such an operand,
  // and that it is 32/64 bits wide.

  // If "which" is end_pc_operand, find the end of the instruction.

  address ip = inst;
  bool is_64bit = false;

  debug_only(bool has_disp32 = false);
  int tail_size = 0; // other random bytes (#32, #16, etc.) at end of insn

  again_after_prefix:
  switch (0xFF & *ip++) {

  // These convenience macros generate groups of "case" labels for the switch.
#define REP4(x) (x)+0: case (x)+1: case (x)+2: case (x)+3
#define REP8(x) (x)+0: case (x)+1: case (x)+2: case (x)+3: \
             case (x)+4: case (x)+5: case (x)+6: case (x)+7
#define REP16(x) REP8((x)+0): \
              case REP8((x)+8)

  case CS_segment:
  case SS_segment:
  case DS_segment:
  case ES_segment:
  case FS_segment:
  case GS_segment:
    assert(0, "shouldn't have that prefix");
    assert(ip == inst + 1 || ip == inst + 2, "only two prefixes allowed");
    goto again_after_prefix;

  case 0x67:
  case REX:
  case REX_B:
  case REX_X:
  case REX_XB:
  case REX_R:
  case REX_RB:
  case REX_RX:
  case REX_RXB:
//     assert(ip == inst + 1, "only one prefix allowed");
    goto again_after_prefix;
    
  case REX_W:
  case REX_WB:
  case REX_WX:
  case REX_WXB:
  case REX_WR:
  case REX_WRB:
  case REX_WRX:
  case REX_WRXB:
    is_64bit = true;
//     assert(ip == inst + 1, "only one prefix allowed");
    goto again_after_prefix;

  case 0xFF: // pushq a; decl a; incl a; call a; jmp a
  case 0x88: // movb a, r
  case 0x89: // movl a, r
  case 0x8A: // movb r, a
  case 0x8B: // movl r, a
  case 0x8F: // popl a
    break;

  case 0x68: // pushq #32
    if (which == end_pc_operand) {
      return ip + 4;
    }
    assert(0, "pushq has no disp32 or imm64");
    ShouldNotReachHere();

  case 0x66: // movw ... (size prefix)
    again_after_size_prefix2:
    switch (0xFF & *ip++) {
    case REX:
    case REX_B:
    case REX_X:
    case REX_XB:
    case REX_R:
    case REX_RB:
    case REX_RX:
    case REX_RXB:
    case REX_W:
    case REX_WB:
    case REX_WX:
    case REX_WXB:
    case REX_WR:
    case REX_WRB:
    case REX_WRX:
    case REX_WRXB:
      goto again_after_size_prefix2;
    case 0x8B: // movw r, a
    case 0x89: // movw a, r
      break;
    case 0xC7: // movw a, #16
      tail_size = 2;  // the imm16
      break;
    case 0x0F: // several SSE/SSE2 variants
      ip--;    // reparse the 0x0F
      goto again_after_prefix;
    default:
      ShouldNotReachHere();
    }
    break;

  case REP8(0xB8): // movl/q r, #32/#64(oop?)
    if (which == end_pc_operand)  return ip + (is_64bit ? 8 : 4);
    assert(which == imm64_operand && is_64bit, "");
    return ip;

  case 0x69: // imul r, a, #32
  case 0xC7: // movl a, #32(oop?)
    tail_size = 4;
    debug_only(has_disp32 = true); // has both kinds of operands!
    break;

  case 0x0F: // movx..., etc.
    switch (0xFF & *ip++) {
    case 0x2E: // ucomiss
    case 0x2F: // comiss
    case 0x57: // xorps
    case 0x54: // andps
    case 0x12: // movlpd
    case 0x6E: // movd
    case 0x7E: // movd
    case 0xAE: // ldmxcsr   a
    case 0xD6: // movq
      debug_only(has_disp32 = true); // has both kinds of operands!
      break;
    case 0xAD: // shrd r, a, %cl
    case 0xAF: // imul r, a
    case 0xBE: // movsbl r, a
    case 0xBF: // movswl r, a
    case 0xB6: // movzbl r, a
    case 0xB7: // movzwl r, a
    case REP16(0x40): // cmovl cc, r, a
    case 0xB0: // cmpxchgb
    case 0xB1: // cmpxchg
    case 0xC1: // xaddl
    case 0xC7: // cmpxchg8
    case REP16(0x90): // setcc a
      // fall out of the switch to decode the address
      break;
    case 0xAC: // shrd r, a, #8
      tail_size = 1;  // the imm8
      break;
    case REP16(0x80): // jcc rdisp32
      if (which == end_pc_operand)  return ip + 4;
      assert(which == call32_operand, "jcc has no disp32 or imm64");
      return ip;
    default:
      ShouldNotReachHere();
    }
    break;

  case 0x81: // addl a, #32; addl r, #32
    // also: orl, adcl, sbbl, andl, subl, xorl, cmpl
    tail_size = 4;
    debug_only(has_disp32 = true); // has both kinds of operands!
    break;

  case 0x83: // addl a, #8; addl r, #8
    // also: orl, adcl, sbbl, andl, subl, xorl, cmpl
    tail_size = 1;
    break;

  case 0x9B:
    switch (0xFF & *ip++) {
    case 0xD9: // fnstcw a
      break;
    default:
      ShouldNotReachHere();
    }
    break;

  case REP4(0x00): // addb a, r; addl a, r; addb r, a; addl r, a
  case REP4(0x10): // adc...
  case REP4(0x20): // and...
  case REP4(0x30): // xor...
  case REP4(0x08): // or...
  case REP4(0x18): // sbb...
  case REP4(0x28): // sub...
  case 0xF7: // mull a
  case 0x87: // xchg r, a
    break;
  case REP4(0x38): // cmp...
  case 0x8D: // lea r, a
  case 0x85: // test r, a
    debug_only(has_disp32 = true); // has both kinds of operands!
    break;

  case 0xC6: // movb a, #8 
  case 0x80: // cmpb a, #8
  case 0x6B: // imul r, a, #8
    tail_size = 1; // the imm8
    break;

  case 0xE8: // call rdisp32
  case 0xE9: // jmp  rdisp32
    if (which == end_pc_operand)  return ip + 4;
    assert(which == call32_operand, "call has no disp32 or imm32");
    return ip;

  case 0xD1: // sal a, 1; sar a, 1; shl a, 1; shr a, 1
  case 0xD3: // sal a, %cl; sar a, %cl; shl a, %cl; shr a, %cl
  case 0xD9: // fld_s a; fst_s a; fstp_s a; fldcw a
  case 0xDD: // fld_d a; fst_d a; fstp_d a
  case 0xDB: // fild_s a; fistp_s a; fld_x a; fstp_x a
  case 0xDF: // fild_d a; fistp_d a
  case 0xD8: // fadd_s a; fsubr_s a; fmul_s a; fdivr_s a; fcomp_s a
  case 0xDC: // fadd_d a; fsubr_d a; fmul_d a; fdivr_d a; fcomp_d a
  case 0xDE: // faddp_d a; fsubrp_d a; fmulp_d a; fdivrp_d a; fcompp_d a
    break;

  case 0xF3:                    // For SSE
  case 0xF2:                    // For SSE2
    switch (0xFF & *ip++) {
    case REX:
    case REX_B:
    case REX_X:
    case REX_XB:
    case REX_R:
    case REX_RB:
    case REX_RX:
    case REX_RXB:
    case REX_W:
    case REX_WB:
    case REX_WX:
    case REX_WXB:
    case REX_WR:
    case REX_WRB:
    case REX_WRX:
    case REX_WRXB:
      ip++;
    default:
      ip++;
    }
    debug_only(has_disp32 = true); // has both kinds of operands!
    break;

  default:
    ShouldNotReachHere();

#undef REP8
#undef REP16
  }

  assert(which != call32_operand, "instruction is not a call, jmp, or jcc");
  assert(which != imm64_operand, "instruction is not a movq reg, imm64");
  assert(which != disp32_operand || has_disp32,
         "instruction has no disp32 field");

  // parse the output of emit_operand
  int op2 = 0xFF & *ip++;
  int base = op2 & 0x07;
  int op3 = -1;
  const int b100 = 4;
  const int b101 = 5;
  if (base == b100 && (op2 >> 6) != 3) {
    op3 = 0xFF & *ip++;
    base = op3 & 0x07;   // refetch the base
  }
  // now ip points at the disp (if any)

  switch (op2 >> 6) {
  case 0:
    // [00 reg  100][ss index base]
    // [00 reg  100][00   100  esp]
    // [00 reg base]
    // [00 reg  100][ss index  101][disp32]
    // [00 reg  101]               [disp32]
    
    if (base == b101) {
      if (which == disp32_operand)
	return ip;		// caller wants the disp32
      ip += 4;			// skip the disp32
    }
    break;

  case 1:
    // [01 reg  100][ss index base][disp8]
    // [01 reg  100][00   100  esp][disp8]
    // [01 reg base]               [disp8]
    ip += 1;			// skip the disp8
    break;

  case 2:
    // [10 reg  100][ss index base][disp32]
    // [10 reg  100][00   100  esp][disp32]
    // [10 reg base]               [disp32]
    if (which == disp32_operand)
      return ip;		// caller wants the disp32
    ip += 4;			// skip the disp32
    break;

  case 3:
    // [11 reg base]  (not a memory addressing mode)
    break;
  }

  if (which == end_pc_operand) {
    return ip + tail_size;
  }

  assert(0, "fix locate_operand");
  return ip;
}

address Assembler::locate_next_instruction(address inst)
{
  // Secretly share code with locate_operand:
  return locate_operand(inst, end_pc_operand);
}

#ifdef ASSERT
void Assembler::check_relocation(RelocationHolder const& rspec, int format) 
{
  address inst = _inst_mark;
  assert(inst != NULL && inst < pc(),
         "must point to beginning of instruction");
  address opnd;

  Relocation* r = rspec.reloc();
  if (r->type() == relocInfo::none) {
    return;
  } else if (r->is_call()) {
    assert(format == disp32_operand, "cannot specify a nonzero format");
    opnd = locate_operand(inst, call32_operand);
  } else if (r->is_data()) {
    assert(format == imm64_operand || format == disp32_operand, "format ok");
    opnd = locate_operand(inst, (WhichOperand) format);
  } else {
    assert(format == 0, "cannot specify a format");
    return;
  }
  assert(opnd == pc(), "must put operand where relocs can find it");
}
#endif

void Assembler::emit_operand(Register reg, Address adr) 
{
  emit_operand(reg, adr._base, adr._index, adr._scale, adr._disp,
               adr._target,
               adr._rspec);
}

void Assembler::emit_operand(FloatRegister reg, Address adr) 
{
  emit_operand(reg, adr._base, adr._index, adr._scale, adr._disp,
               adr._target,
               adr._rspec);
}

void Assembler::emit_farith(int b1, int b2, int i) 
{
  assert(isByte(b1) && isByte(b2), "wrong opcode");
  assert(0 <= i &&  i < 8, "illegal stack offset");
  emit_byte(b1);
  emit_byte(b2 + i);
}

// pushad is invalid, use this instead.
// NOTE: Kills flags!!
void Assembler::pushaq() 
{
  // we have to store original rsp.  ABI says that 128 bytes
  // below rsp are local scratch.
  movq(Address(rsp, -5 * wordSize), rsp);

  subq(rsp, 16 * wordSize);

  movq(Address(rsp, 15 * wordSize), rax);
  movq(Address(rsp, 14 * wordSize), rcx);
  movq(Address(rsp, 13 * wordSize), rdx);
  movq(Address(rsp, 12 * wordSize), rbx);
  // skip rsp
  movq(Address(rsp, 10 * wordSize), rbp);
  movq(Address(rsp, 9 * wordSize), rsi);
  movq(Address(rsp, 8 * wordSize), rdi);
  movq(Address(rsp, 7 * wordSize), r8);
  movq(Address(rsp, 6 * wordSize), r9);
  movq(Address(rsp, 5 * wordSize), r10);
  movq(Address(rsp, 4 * wordSize), r11);
  movq(Address(rsp, 3 * wordSize), r12);
  movq(Address(rsp, 2 * wordSize), r13);
  movq(Address(rsp, wordSize), r14);
  movq(Address(rsp), r15);
}

// popad is invalid, use this instead
// NOTE: Kills flags!!
void Assembler::popaq() 
{
  movq(r15, Address(rsp));
  movq(r14, Address(rsp, wordSize));
  movq(r13, Address(rsp, 2 * wordSize));
  movq(r12, Address(rsp, 3 * wordSize));
  movq(r11, Address(rsp, 4 * wordSize));
  movq(r10, Address(rsp, 5 * wordSize));
  movq(r9,  Address(rsp, 6 * wordSize));
  movq(r8,  Address(rsp, 7 * wordSize));
  movq(rdi, Address(rsp, 8 * wordSize));
  movq(rsi, Address(rsp, 9 * wordSize));
  movq(rbp, Address(rsp, 10 * wordSize));
  // skip rsp
  movq(rbx, Address(rsp, 12 * wordSize));
  movq(rdx, Address(rsp, 13 * wordSize));
  movq(rcx, Address(rsp, 14 * wordSize));
  movq(rax, Address(rsp, 15 * wordSize));

  addq(rsp, 16 * wordSize);
}

void Assembler::pushfq() 
{
  emit_byte(0x9C);
}

void Assembler::popfq() 
{
  emit_byte(0x9D);
}

void Assembler::pushq(int imm32) 
{
  emit_byte(0x68);
  emit_long(imm32);  
}

void Assembler::pushq(Register src) 
{
  int srcenc = src->encoding();
  if (srcenc >= 8) {
    prefix(REX_B);
    srcenc -= 8;
  }
  emit_byte(0x50 | srcenc);
}

void Assembler::pushq(Address src) 
{
  InstructionMark im(this);
  if (src.base_needs_rex()) {
    if (src.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (src.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xFF);
  emit_operand(rsi, src);  
}

void Assembler::popq(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0x58 | dstenc);
}

void Assembler::popq(Address dst)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0x8F);
  emit_operand(rax, dst);
}

void Assembler::prefix(Prefix p)
{
  a_byte(p);
}

void Assembler::movb(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      } else if (dst->encoding() >= 4) {
        prefix(REX);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x8A);
  emit_operand(dst, src);
}

void Assembler::movb(Address dst, int imm8)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xC6);
  dst.fix_rip_relative_offset(1);
  emit_operand(rax, dst);
  emit_byte(imm8);
}

void Assembler::movb(Address dst, Register src)
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      } else if (src->encoding() >= 4) {
        prefix(REX);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x88);
  emit_operand(src, dst);
}

void Assembler::movw(Address dst, int imm16)
{
  InstructionMark im(this);
  emit_byte(0x66); // switch to 16-bit mode
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xC7);
  dst.fix_rip_relative_offset(2);
  emit_operand(rax, dst);
  emit_word(imm16);
}
  
void Assembler::movw(Register dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0x66);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x8B);
  emit_operand(dst, src);
}

void Assembler::movw(Address dst, Register src)
{
  InstructionMark im(this);
  emit_byte(0x66);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x89);
  emit_operand(src, dst);
}

// Uses zero extension.
void Assembler::movl(Register dst, int imm32)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xB8 | dstenc);
  emit_long(imm32);
}

void Assembler::movl(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x8B);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x8B);
  emit_operand(dst, src);
}

void Assembler::movl(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xC7);
  dst.fix_rip_relative_offset(4);
  emit_operand(rax, dst);
  emit_long(imm32);
}

void Assembler::movl(Address dst, Register src)
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x89);
  emit_operand(src, dst);
}

void Assembler::movq(Register dst, int64_t imm64)
{
  InstructionMark im(this);
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xB8 | dstenc);
  emit_long64(imm64);
}

void Assembler::movq(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();

  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x8B);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movq(Register dst, jobject obj)
{
  ShouldNotReachHere();
  InstructionMark im(this);
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xB8 | dstenc);
  emit_data64((jlong) obj, relocInfo::oop_type);
}

void Assembler::movq(Register dst, Address src)
{
  InstructionMark im(this);
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    dstenc -= 8;
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  if (is_reachable(src)) {
    emit_byte(0x8B);
    emit_operand(dst, src);
  }
  else {
    emit_byte(0xB8 | dstenc);
// [RGV] using relocInfo::none for now to work around assert. 
    emit_data64((jlong) src._target, relocInfo::none );
  }
}

void Assembler::movq(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_byte(0xC7);
  dst.fix_rip_relative_offset(4);
  emit_operand(rax, dst);
  emit_long(imm32);
}

void Assembler::movq(Address dst, Register src)
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x89);
  emit_operand(src, dst);
}

void Assembler::movsbl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xBE);
  emit_operand(dst, src);
}

void Assembler::movsbl(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    } else if (srcenc >= 4) {
      prefix(REX);
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xBE);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movswl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xBF);
  emit_operand(dst, src);
}
  
void Assembler::movswl(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xBF);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movslq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x63);
  emit_operand(dst, src);
}
  
void Assembler::movslq(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x63);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}
  
void Assembler::movzbl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xB6);
  emit_operand(dst, src);
}

void Assembler::movzbl(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    } else if (srcenc >= 4) {
      prefix(REX);
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xB6);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movzwl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xB7);
  emit_operand(dst, src);
}
 
void Assembler::movzwl(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xB7);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movss(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x10);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movss(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF3);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x10);
  emit_operand(dst, src);
}

void Assembler::movss(Address dst, FloatRegister src)
{
  InstructionMark im(this);
  emit_byte(0xF3);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x11);
  emit_operand(src, dst);
}

void Assembler::movsd(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x10);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

// Optimization guide recommends using movlpd instead of movsd for this case
void Assembler::movlpd(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0x66);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x12);
  emit_operand(dst, src);
}

void Assembler::movsd(Address dst, FloatRegister src)
{
  InstructionMark im(this);
  emit_byte(0xF2);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x11);
  emit_operand(src, dst);
}

void Assembler::movdl(FloatRegister dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0x66);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x6E);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movdl(Register dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0x66);
  if (srcenc < 8) {
    if (dstenc >= 8) {
      prefix(REX_B);
      dstenc -= 8;
    }
  } else {
    if (dstenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      dstenc -= 8;
    }
    srcenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x7E);
  emit_byte(0xC0 | srcenc << 3 | dstenc);
}

void Assembler::movdq(FloatRegister dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0x66);
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x6E);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::movdq(Register dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0x66);
  if (srcenc < 8) {
    if (dstenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      dstenc -= 8;
    }
  } else {
    if (dstenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      dstenc -= 8;
    }
    srcenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x7E);
  emit_byte(0xC0 | srcenc << 3 | dstenc);
}

void Assembler::cmovl(Condition cc, Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x40 | cc);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cmovl(Condition cc, Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x40 | cc);
  emit_operand(dst, src);
}

void Assembler::cmovq(Condition cc, Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x40 | cc);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cmovq(Condition cc, Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x40 | cc);
  emit_operand(dst, src);
}

void Assembler::adcl(Register dst, int imm32)
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xD0, dst, imm32);
}

void Assembler::adcl(Register dst, Address src) 
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x13);
  emit_operand(dst, src);
}

void Assembler::adcl(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x13, 0xC0, dst, src);
}

void Assembler::adcq(Register dst, int imm32)
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xD0, dst, imm32);
}

void Assembler::adcq(Register dst, Address src) 
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x13);
  emit_operand(dst, src);
}

void Assembler::adcq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x13, 0xC0, dst, src);
}

void Assembler::addl(Address dst, int imm32) 
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_arith_operand(0x81, rax, dst,imm32);
}

void Assembler::addl(Address dst, Register src) 
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x01);
  emit_operand(src, dst);
}

void Assembler::addl(Register dst, int imm32) 
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xC0, dst, imm32);
}

void Assembler::addl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x03);
  emit_operand(dst, src);
}

void Assembler::addl(Register dst, Register src) 
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x03, 0xC0, dst, src);
}

void Assembler::addq(Address dst, int imm32) 
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_arith_operand(0x81, rax, dst,imm32);
}

void Assembler::addq(Address dst, Register src) 
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x01);
  emit_operand(src, dst);
}

void Assembler::addq(Register dst, int imm32) 
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xC0, dst, imm32);
}

void Assembler::addq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x03);
  emit_operand(dst, src);
}

void Assembler::addq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x03, 0xC0, dst, src);
}

void Assembler::andl(Register dst, int imm32)
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xE0, dst, imm32);
}

void Assembler::andl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x23);
  emit_operand(dst, src);
}

void Assembler::andl(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x23, 0xC0, dst, src);
}

void Assembler::andq(Register dst, int imm32)
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xE0, dst, imm32);
}

void Assembler::andq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x23);
  emit_operand(dst, src);
}

void Assembler::andq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x23, 0xC0, dst, src);
}

void Assembler::cmpb(Address dst, int imm8)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0x80);
  dst.fix_rip_relative_offset(1);
  emit_operand(rdi, dst);
  emit_byte(imm8);
}

void Assembler::cmpl(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0x81);
  dst.fix_rip_relative_offset(4);
  emit_operand(rdi, dst);
  emit_long(imm32);
}

void Assembler::cmpl(Register dst, int imm32)
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xF8, dst, imm32);
}

void Assembler::cmpl(Register dst, Register src) 
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x3B, 0xC0, dst, src);
}

void Assembler::cmpl(Register dst, Address src) 
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x3B);
  emit_operand(dst, src);
}

void Assembler::cmpq(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_byte(0x81);
  dst.fix_rip_relative_offset(4);
  emit_operand(rdi, dst);
  emit_long(imm32);
}

void Assembler::cmpq(Register dst, int imm32)
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xF8, dst, imm32);
}

void Assembler::cmpq(Register dst, Register src) 
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x3B, 0xC0, dst, src);
}

void Assembler::cmpq(Register dst, Address  src) 
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x3B);
  emit_operand(dst, src);
}

void Assembler::ucomiss(FloatRegister dst, FloatRegister src) 
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2E);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::ucomisd(FloatRegister dst, FloatRegister src) 
{
  emit_byte(0x66);
  ucomiss(dst, src);
}

void Assembler::decb(Register dst) 
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  } else if (dstenc >= 4) {
    prefix(REX);
  }
  emit_byte(0xFE);
  emit_byte(0xC8 | dstenc);
}

void Assembler::decl(Register dst)
{
  // Use two-byte form (one-byte from is a REX prefix in 64-bit mode)
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xFF);
  emit_byte(0xC8 | dstenc);
}

void Assembler::decl(Address dst)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xFF);
  emit_operand(rcx, dst);
}

void Assembler::decq(Register dst) 
{
  // Use two-byte form (one-byte from is a REX prefix in 64-bit mode)
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xFF);
  emit_byte(0xC8 | dstenc);
}

void Assembler::decq(Address dst)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_byte(0xFF);
  emit_operand(rcx, dst);
}

void Assembler::idivl(Register src)
{
  int srcenc = src->encoding();
  if (srcenc >= 8) {
    prefix(REX_B);
    srcenc -= 8;
  }
  emit_byte(0xF7);
  emit_byte(0xF8 | srcenc);
}

void Assembler::idivq(Register src)
{
  int srcenc = src->encoding();
  if (srcenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    srcenc -= 8;
  }
  emit_byte(0xF7);
  emit_byte(0xF8 | srcenc);
}

void Assembler::cdql()
{
  emit_byte(0x99);
}

void Assembler::cdqq()
{
  prefix(REX_W);
  emit_byte(0x99);
}

void Assembler::imull(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xAF);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::imull(Register dst, Register src, int value)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  if (is8bit(value)) {
    emit_byte(0x6B);
    emit_byte(0xC0 | dstenc << 3 | srcenc);
    emit_byte(value);
  } else {
    emit_byte(0x69);
    emit_byte(0xC0 | dstenc << 3 | srcenc);
    emit_long(value);
  }
}

void Assembler::imulq(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xAF);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::imulq(Register dst, Register src, int value)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  if (is8bit(value)) {
    emit_byte(0x6B);
    emit_byte(0xC0 | dstenc << 3 | srcenc);
    emit_byte(value);
  } else {
    emit_byte(0x69);
    emit_byte(0xC0 | dstenc << 3 | srcenc);
    emit_long(value);
  }
}

void Assembler::incl(Register dst)
{
  // Use two-byte form (one-byte from is a REX prefix in 64-bit mode)
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xFF);
  emit_byte(0xC0 | dstenc);
}

void Assembler::incl(Address dst)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xFF);
  emit_operand(rax, dst);
}

void Assembler::incq(Register dst)
{
  // Use two-byte form (one-byte from is a REX prefix in 64-bit mode)
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xFF);
  emit_byte(0xC0 | dstenc);
}

void Assembler::incq(Address dst)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_byte(0xFF);
  emit_operand(rax, dst);
}

void Assembler::leal(Register dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0x67); // addr32
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x8D);
  emit_operand(dst, src);
}

void Assembler::leaq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x8D);
  emit_operand(dst, src);
}

void Assembler::mull(Address src)
{
  InstructionMark im(this);
  emit_byte(0xF7);
  emit_operand(rsp, src);
}

void Assembler::mull(Register src)
{
  emit_byte(0xF7);
  emit_byte(0xE0 | src->encoding());
}

void Assembler::negl(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xF7);
  emit_byte(0xD8 | dstenc);
}

void Assembler::negq(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xF7);
  emit_byte(0xD8 | dstenc);
}

void Assembler::notl(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xF7);
  emit_byte(0xD0 | dstenc);
}

void Assembler::notq(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xF7);
  emit_byte(0xD0 | dstenc);
}

void Assembler::orl(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0x81);
  dst.fix_rip_relative_offset(4);
  emit_operand(rcx, dst);
  emit_long(imm32);
}

void Assembler::orl(Register dst, int imm32)
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xC8, dst, imm32);
}

void Assembler::orl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0B);
  emit_operand(dst, src);
}

void Assembler::orl(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x0B, 0xC0, dst, src);
}

void Assembler::orq(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_byte(0x81);
  dst.fix_rip_relative_offset(4);
  emit_operand(rcx, dst);
  emit_long(imm32);
}

void Assembler::orq(Register dst, int imm32)
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xC8, dst, imm32);
}

void Assembler::orq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x0B);
  emit_operand(dst, src);
}

void Assembler::orq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x0B, 0xC0, dst, src);
}

void Assembler::rcll(Register dst, int imm8)
{
  assert(isShiftCount(imm8), "illegal shift count");
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  if (imm8 == 1) {
    emit_byte(0xD1);
    emit_byte(0xD0 | dstenc);
  } else {
    emit_byte(0xC1);
    emit_byte(0xD0 | dstenc);
    emit_byte(imm8);
  }
}

void Assembler::rclq(Register dst, int imm8)
{
  assert(isShiftCount(imm8 >> 1), "illegal shift count");
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    emit_byte(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  if (imm8 == 1) {
    emit_byte(0xD1);
    emit_byte(0xD0 | dstenc);
  } else {
    emit_byte(0xC1);
    emit_byte(0xD0 | dstenc);
    emit_byte(imm8);
  }
}

void Assembler::sarl(Register dst, int imm8)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  assert(isShiftCount(imm8), "illegal shift count");
  if (imm8 == 1) {
    emit_byte(0xD1);
    emit_byte(0xF8 | dstenc);
  } else {
    emit_byte(0xC1);
    emit_byte(0xF8 | dstenc);
    emit_byte(imm8);
  }
}

void Assembler::sarl(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xD3);
  emit_byte(0xF8 | dstenc);
}

void Assembler::sarq(Register dst, int imm8)
{
  assert(isShiftCount(imm8 >> 1), "illegal shift count");
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  if (imm8 == 1) {
    emit_byte(0xD1);
    emit_byte(0xF8 | dstenc);
  } else {
    emit_byte(0xC1);
    emit_byte(0xF8 | dstenc);
    emit_byte(imm8);
  }
}

void Assembler::sarq(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xD3);
  emit_byte(0xF8 | dstenc);
}

void Assembler::sbbl(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_arith_operand(0x81, rbx, dst, imm32);
}

void Assembler::sbbl(Register dst, int imm32)
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xD8, dst, imm32);
}

void Assembler::sbbl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x1B);
  emit_operand(dst, src);
}

void Assembler::sbbl(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x1B, 0xC0, dst, src);
}

void Assembler::sbbq(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_arith_operand(0x81, rbx, dst, imm32);
}

void Assembler::sbbq(Register dst, int imm32)
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xD8, dst, imm32);
}

void Assembler::sbbq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x1B);
  emit_operand(dst, src);
}

void Assembler::sbbq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x1B, 0xC0, dst, src);
}

void Assembler::shll(Register dst, int imm8)
{
  assert(isShiftCount(imm8), "illegal shift count");
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  if (imm8 == 1 ) {
    emit_byte(0xD1);
    emit_byte(0xE0 | dstenc);
  } else {
    emit_byte(0xC1);
    emit_byte(0xE0 | dstenc);
    emit_byte(imm8);
  }
}

void Assembler::shll(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xD3);
  emit_byte(0xE0 | dstenc);
}

void Assembler::shlq(Register dst, int imm8)
{
  assert(isShiftCount(imm8 >> 1), "illegal shift count");
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  if (imm8 == 1) {
    emit_byte(0xD1);
    emit_byte(0xE0 | dstenc);
  } else {
    emit_byte(0xC1);
    emit_byte(0xE0 | dstenc);
    emit_byte(imm8);
  }
}

void Assembler::shlq(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xD3);
  emit_byte(0xE0 | dstenc);
}

void Assembler::shrl(Register dst, int imm8)
{
  assert(isShiftCount(imm8), "illegal shift count");
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xC1);
  emit_byte(0xE8 | dstenc);
  emit_byte(imm8);
}

void Assembler::shrl(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  emit_byte(0xD3);
  emit_byte(0xE8 | dstenc);
}

void Assembler::shrq(Register dst, int imm8)
{
  assert(isShiftCount(imm8 >> 1), "illegal shift count");
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xC1);
  emit_byte(0xE8 | dstenc);
  emit_byte(imm8);
}

void Assembler::shrq(Register dst)
{
  int dstenc = dst->encoding();
  if (dstenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    dstenc -= 8;
  }
  emit_byte(0xD3);
  emit_byte(0xE8 | dstenc);
}
  
void Assembler::subl(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  if (is8bit(imm32)) {
    emit_byte(0x83);
    dst.fix_rip_relative_offset(1);
    emit_operand(rbp, dst);
    emit_byte(imm32 & 0xFF);
  } else {
    emit_byte(0x81);
    dst.fix_rip_relative_offset(4);
    emit_operand(rbp, dst);
    emit_long(imm32);
  }
}

void Assembler::subl(Register dst, int imm32)
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xE8, dst, imm32);
}

void Assembler::subl(Address dst, Register src)
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x29);
  emit_operand(src, dst);
}

void Assembler::subl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x2B);
  emit_operand(dst, src);
}

void Assembler::subl(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x2B, 0xC0, dst, src);
}

void Assembler::subq(Address dst, int imm32)
{
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  if (is8bit(imm32)) {
    emit_byte(0x83);
    dst.fix_rip_relative_offset(1);
    emit_operand(rbp, dst);
    emit_byte(imm32 & 0xFF);
  } else {
    emit_byte(0x81);
    dst.fix_rip_relative_offset(4);
    emit_operand(rbp, dst);
    emit_long(imm32);
  }
}

void Assembler::subq(Register dst, int imm32)
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xE8, dst, imm32);
}

void Assembler::subq(Address dst, Register src)
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x29);
  emit_operand(src, dst);
}

void Assembler::subq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x2B);
  emit_operand(dst, src);
}

void Assembler::subq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x2B, 0xC0, dst, src);
}

void Assembler::testb(Register dst, int imm8)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
  } else if (dstenc >= 4) {
    prefix(REX);
  }
  emit_arith_b(0xF6, 0xC0, dst, imm8);
}

void Assembler::testl(Register dst, int imm32)
{
  // not using emit_arith because test
  // doesn't support sign-extension of
  // 8bit operands
  int dstenc = dst->encoding();
  if (dstenc == 0) {
    emit_byte(0xA9);
  } else {
    if (dstenc >= 8) {
      prefix(REX_B);
      dstenc -= 8;
    }
    emit_byte(0xF7);
    emit_byte(0xC0 | dstenc);
  }
  emit_long(imm32);
}

void Assembler::testl(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x85, 0xC0, dst, src);
}

void Assembler::testq(Register dst, int imm32)
{
  // not using emit_arith because test
  // doesn't support sign-extension of
  // 8bit operands
  int dstenc = dst->encoding();
  if (dstenc == 0) {
    prefix(REX_W);
    emit_byte(0xA9);
  } else { 
    if (dstenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      dstenc -= 8;
    }
    emit_byte(0xF7);
    emit_byte(0xC0 | dstenc);
  }
  emit_long(imm32);
}

void Assembler::testq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x85, 0xC0, dst, src);
}

void Assembler::xaddl(Address dst, Register src)
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xC1);
  emit_operand(src, dst);
}

void Assembler::xaddq(Address dst, Register src)
{
  InstructionMark im(this);
  if (src->encoding() < 8) {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (dst.base_needs_rex()) {
      if (dst.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (dst.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xC1);
  emit_operand(src, dst);
}

void Assembler::xorl(Register dst, int imm32)
{
  if (dst->encoding() >= 8) {
    prefix(REX_B);
  }
  emit_arith(0x81, 0xF0, dst, imm32);
}

void Assembler::xorl(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() >= 8) {
      prefix(REX_B);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
    }
  }
  emit_arith(0x33, 0xC0, dst, src);
}

void Assembler::xorl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x33);
  emit_operand(dst, src);
}

void Assembler::xorq(Register dst, int imm32)
{
  if (dst->encoding() < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
  }
  emit_arith(0x81, 0xF0, dst, imm32);
}

void Assembler::xorq(Register dst, Register src)
{
  if (dst->encoding() < 8) {
    if (src->encoding() < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src->encoding() < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
    }
  }
  emit_arith(0x33, 0xC0, dst, src);
}

void Assembler::xorq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x33);
  emit_operand(dst, src);
}

void Assembler::bswapl(Register reg)
{
  int regenc = reg->encoding();
  if (regenc >= 8) {
    prefix(REX_B);
    regenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xC8 | regenc);
}

void Assembler::bswapq(Register reg)
{
  int regenc = reg->encoding();
  if (regenc < 8) {
    prefix(REX_W);
  } else {
    prefix(REX_WB);
    regenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0xC8 | regenc);
}

void Assembler::lock()
{
  emit_byte(0xF0);
}

void Assembler::xchgl(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x87);
  emit_operand(dst, src);
}

void Assembler::xchgl(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x87);
  emit_byte(0xc0 | dstenc << 3 | srcenc);
}

void Assembler::xchgq(Register dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x87);
  emit_operand(dst, src);
}

void Assembler::xchgq(Register dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x87);
  emit_byte(0xc0 | dstenc << 3 | srcenc);
}

void Assembler::cmpxchgl(Register reg, Address adr)
{
  InstructionMark im(this);
  if (reg->encoding() < 8) {
    if (adr.base_needs_rex()) {
      if (adr.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (adr.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (adr.base_needs_rex()) {
      if (adr.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (adr.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xB1);
  emit_operand(reg, adr);
}

void Assembler::cmpxchgq(Register reg, Address adr)
{
  InstructionMark im(this);
  if (reg->encoding() < 8) {
    if (adr.base_needs_rex()) {
      if (adr.index_needs_rex()) {
        prefix(REX_WXB);
      } else {
        prefix(REX_WB);
      }
    } else {
      if (adr.index_needs_rex()) {
        prefix(REX_WX);
      } else {
        prefix(REX_W);
      }
    }
  } else {
    if (adr.base_needs_rex()) {
      if (adr.index_needs_rex()) {
        prefix(REX_WRXB);
      } else {
        prefix(REX_WRB);
      }
    } else {
      if (adr.index_needs_rex()) {
        prefix(REX_WRX);
      } else {
        prefix(REX_WR);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0xB1);
  emit_operand(reg, adr);
}

void Assembler::hlt()
{
  emit_byte(0xF4);
}

void Assembler::nop()
{
  emit_byte(0x90);
}

void Assembler::ret(int imm16)
{
  if (imm16 == 0) {
    emit_byte(0xC3);
  } else {
    emit_byte(0xC2);
    emit_word(imm16);
  }
}

// copies data from [rsi] to [rdi] using rcx double words (m64)
void Assembler::rep_move()
{
  // REP
  emit_byte(0xF3);
  // MOVSQ
  prefix(REX_W);
  emit_byte(0xA5);
}

// sets rcx double words (m64) with rax value at [rdi]
void Assembler::rep_set()
{
  // REP
  emit_byte(0xF3);
  // STOSQ
  prefix(REX_W);
  emit_byte(0xAB);
}

// scans rcx double words (m64) at [rdi] for occurance of rax
void Assembler::repne_scan()
{
  // REPNE/REPNZ
  emit_byte(0xF2);
  // SCASQ
  prefix(REX_W);
  emit_byte(0xAF);
}

void Assembler::setb(Condition cc, Register dst)
{
  assert(0 <= cc && cc < 16, "illegal cc");
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  } else if (dstenc >= 4) {
    prefix(REX);
  }
  emit_byte(0x0F);
  emit_byte(0x90 | cc);
  emit_byte(0xC0 | dstenc);
}

void Assembler::clflush(Address adr)
{
  if (adr.base_needs_rex()) {
    if (adr.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (adr.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0x0F);
  emit_byte(0xAE);
  emit_operand(rdi, adr);
}

void Assembler::call(Label& L, relocInfo::relocType rtype)
{
  if (L.is_bound()) {
    const int long_size = 5;
    int offs = L.pos() - offset();
    assert(offs <= 0, "assembler error");
    InstructionMark im(this);
    // 1110 1000 #32-bit disp
    emit_byte(0xE8);
    emit_data(offs - long_size, rtype, disp32_operand);
  } else {
    InstructionMark im(this);
    // 1110 1000 #32-bit disp
    emit_byte(0xE8);
    // copied from emit_disp(L,type,info)
    Displacement disp(L, Displacement::call, 0);
    L.link_to(offset());
    emit_data(int(disp.data()), rtype, disp32_operand);
  }
}

void Assembler::call(address entry, relocInfo::relocType rtype)
{
  assert(rtype != relocInfo::virtual_call_type,
         "must use virtual_call_Relocation::spec");
  assert(entry != NULL, "call most probably wrong");
  InstructionMark im(this);
  emit_byte(0xE8);
  intptr_t disp = (intptr_t) entry - ((intptr_t) _code_pos + sizeof(int));
  assert(disp == (intptr_t)(int32_t)disp, "must be 32bit offset (call1)");
  emit_data((int) disp, rtype, disp32_operand);
}

void Assembler::call(address entry, RelocationHolder const& rspec)
{
  assert(entry != NULL, "call most probably wrong");
  InstructionMark im(this);
  emit_byte(0xE8);
  intptr_t disp = (intptr_t) entry - ((intptr_t) _code_pos + sizeof(int));
  assert(disp == (intptr_t)(int32_t)disp, "must be 32bit offset (call2)");
  emit_data((int) disp, rspec, disp32_operand);
}

void Assembler::call(Register dst, relocInfo::relocType rtype)
{
  int dstenc = dst->encoding();
  if (dstenc >= 8) {
    prefix(REX_B);
    dstenc -= 8;
  }
  relocate(rtype);
  emit_byte(0xFF);
  emit_byte(0xD0 | dstenc);
}

void Assembler::call(Address adr)
{
  InstructionMark im(this);
  if (adr.base_needs_rex()) {
    if (adr.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (adr.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xFF);
  emit_operand(rdx, adr);
}

void Assembler::jmp(address entry, relocInfo::relocType rtype)
{
  assert(entry != NULL, "jmp most probably wrong");
  InstructionMark im(this);
  emit_byte(0xE9);
  intptr_t disp = (intptr_t) entry - ((intptr_t) _code_pos + sizeof(int));
  assert(disp == (intptr_t)(int32_t)disp, "must be 32bit offset (call3)");
  emit_data((int) disp, rtype, disp32_operand);
}

void Assembler::jmp(Register reg, relocInfo::relocType rtype)
{
  int regenc = reg->encoding();
  if (regenc >= 8) {
    prefix(REX_B);
    regenc -= 8;
  }
  relocate(rtype);
  emit_byte(0xFF);
  emit_byte(0xE0 | regenc);
}

void Assembler::jmp(Address adr)
{
  InstructionMark im(this);
  if (adr.base_needs_rex()) {
    if (adr.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (adr.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0xFF);
  emit_operand(rsp, adr);
}

void Assembler::jmp(Label& L, relocInfo::relocType rtype)
{
  relocate(rtype);

  if (L.is_bound()) {
    const int short_size = 2;
    const int long_size  = 5;
    int offs = L.pos() - offset();
    assert(offs <= 0, "assembler error");
    if (isByte(offs - short_size)) {
      // 1110 1011 #8-bit disp
      emit_byte(0xEB);
      emit_byte((offs - short_size) & 0xFF);
    } else {
      // 1110 1001 #32-bit disp
      emit_byte(0xE9);
      emit_long(offs - long_size);
    }
  } else {
    // 1110 1001 #32-bit disp
    emit_byte(0xE9);
    emit_disp(L, Displacement::absolute_jump, 0);
  }
}

void Assembler::jcc(Condition cc, Label& L, relocInfo::relocType rtype)
{
  relocate(rtype);
  assert((0 <= cc) && (cc < 16), "illegal cc");
  if (L.is_bound()) {
    const int short_size = 2;
    const int long_size  = 6;
    int offs = L.pos() - offset();
    assert(offs <= 0, "assembler error");
    if (isByte(offs - short_size)) {
      // 0111 tttn #8-bit disp
      emit_byte(0x70 | cc);
      emit_byte((offs - short_size) & 0xFF);
    } else {
      // 0000 1111 1000 tttn #32-bit disp
      emit_byte(0x0F);
      emit_byte(0x80 | cc);
      emit_long(offs - long_size);
    }
  } else {
    // 0000 1111 1000 tttn #32-bit disp
    // Note: could eliminate cond. jumps to this jump if condition
    //       is the same however, seems to be rather unlikely case.
    emit_byte(0x0F);
    emit_byte(0x80 | cc);
    emit_disp(L, Displacement::conditional_jump, cc);
  }
}

void Assembler::jcc(Condition cc, address dst, relocInfo::relocType rtype)
{
  assert((0 <= cc) && (cc < 16), "illegal cc");
  assert(dst != NULL, "jcc most probably wrong");
  // 0000 1111 1000 tttn #32-bit disp
  InstructionMark im(this);
  emit_byte(0x0F);
  emit_byte(0x80 | cc);
  intptr_t disp = (intptr_t) dst - ((intptr_t) _code_pos + sizeof(int));
  assert(disp == (intptr_t)(int32_t)disp, "must be 32bit offset (call4)");
  emit_data((int) disp, rtype, disp32_operand);
}


// FP instructions

void Assembler::fxsave(Address dst)
{
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_byte(0x0F);
  emit_byte(0xAE);
  emit_operand(as_Register(0), dst);
}

void Assembler::fxrstor(Address src)
{
  if (src.base_needs_rex()) {
    if (src.index_needs_rex()) {
      prefix(REX_WXB);
    } else {
      prefix(REX_WB);
    }
  } else {
    if (src.index_needs_rex()) {
      prefix(REX_WX);
    } else {
      prefix(REX_W);
    }
  }
  emit_byte(0x0F);
  emit_byte(0xAE);
  emit_operand(as_Register(1), src);
}

void Assembler::ldmxcsr(Address src)
{ 
  InstructionMark im(this);
  if (src.base_needs_rex()) {
    if (src.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (src.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0x0F);
  emit_byte(0xAE);
  emit_operand(as_Register(2), src);
}

void Assembler::stmxcsr(Address dst)
{ 
  InstructionMark im(this);
  if (dst.base_needs_rex()) {
    if (dst.index_needs_rex()) {
      prefix(REX_XB);
    } else {
      prefix(REX_B);
    }
  } else {
    if (dst.index_needs_rex()) {
      prefix(REX_X);
    }
  }
  emit_byte(0x0F);
  emit_byte(0xAE);
  emit_operand(as_Register(3), dst);
}

void Assembler::addss(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x58);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::addss(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF3);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x58);
  emit_operand(dst, src);
}

void Assembler::subss(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x5C);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::subss(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF3);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x5C);
  emit_operand(dst, src);
}

void Assembler::mulss(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x59);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::mulss(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF3);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x59);
  emit_operand(dst, src);
}

void Assembler::divss(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x5E);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::divss(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF3);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x5E);
  emit_operand(dst, src);
}

void Assembler::addsd(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x58);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::addsd(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF2);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x58);
  emit_operand(dst, src);
}

void Assembler::subsd(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x5C);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::subsd(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF2);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x5C);
  emit_operand(dst, src);
}

void Assembler::mulsd(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x59);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::mulsd(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF2);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x59);
  emit_operand(dst, src);
}

void Assembler::divsd(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x5E);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::divsd(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF2);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x5E);
  emit_operand(dst, src);
}

void Assembler::sqrtsd(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x51);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::sqrtsd(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0xF2);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x51);
  emit_operand(dst, src);
}

void Assembler::xorps(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x57);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::xorps(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x57);
  emit_operand(dst, src);
}

void Assembler::xorpd(FloatRegister dst, FloatRegister src)
{
  emit_byte(0x66);
  xorps(dst, src);
}

void Assembler::xorpd(FloatRegister dst, Address src)
{
  InstructionMark im(this);
  emit_byte(0x66);
  if (dst->encoding() < 8) {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_XB);
      } else {
        prefix(REX_B);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_X);
      }
    }
  } else {
    if (src.base_needs_rex()) {
      if (src.index_needs_rex()) {
        prefix(REX_RXB);
      } else {
        prefix(REX_RB);
      }
    } else {
      if (src.index_needs_rex()) {
        prefix(REX_RX);
      } else {
        prefix(REX_R);
      }
    }
  }
  emit_byte(0x0F);
  emit_byte(0x57);
  emit_operand(dst, src);
}

void Assembler::cvtsi2ssl(FloatRegister dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2A);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvtsi2ssq(FloatRegister dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2A);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvtsi2sdl(FloatRegister dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2A);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvtsi2sdq(FloatRegister dst, Register src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2A);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvttss2sil(Register dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2C);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvttss2siq(Register dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2C);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvttsd2sil(Register dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2C);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvttsd2siq(Register dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc < 8) {
      prefix(REX_W);
    } else {
      prefix(REX_WB);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_WR);
    } else {
      prefix(REX_WRB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x2C);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvtss2sd(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF3);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x5A);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

void Assembler::cvtsd2ss(FloatRegister dst, FloatRegister src)
{
  int dstenc = dst->encoding();
  int srcenc = src->encoding();
  emit_byte(0xF2);
  if (dstenc < 8) {
    if (srcenc >= 8) {
      prefix(REX_B);
      srcenc -= 8;
    }
  } else {
    if (srcenc < 8) {
      prefix(REX_R);
    } else {
      prefix(REX_RB);
      srcenc -= 8;
    }
    dstenc -= 8;
  }
  emit_byte(0x0F);
  emit_byte(0x5A);
  emit_byte(0xC0 | dstenc << 3 | srcenc);
}

// Implementation of MacroAssembler

void MacroAssembler::fat_nop()
{
  // A 5 byte nop that is safe for patching (see patch_verified_entry)
  // Recommened sequence from 'Software Optimization Guide for the AMD
  // Hammer Processor'
  emit_byte(0x66);
  emit_byte(0x66);
  emit_byte(0x90);
  emit_byte(0x66);
  emit_byte(0x90);
}

void MacroAssembler::null_check(Register reg, int offset)
{
  if (needs_explicit_null_check(offset)) {
    // provoke OS NULL exception if reg = NULL by
    // accessing M[reg] w/o changing any (non-CC) registers
    cmpq(rax, Address(reg));
    // Note: should probably use testl(rax, Address(reg));
    //       may be shorter code (however, this version of
    //       testl needs to be implemented first)
  } else {
    // nothing to do, (later) access of M[reg + offset]
    // will provoke OS NULL exception if reg = NULL
  }
}

int MacroAssembler::load_unsigned_byte(Register dst, Address src)
{
  int off = offset();
  movzbl(dst, src);
  return off;
}

int MacroAssembler::load_unsigned_word(Register dst, Address src)
{
  int off = offset();
  movzwl(dst, src);
  return off;
}

int MacroAssembler::load_signed_byte(Register dst, Address src)
{
  int off = offset();
  movsbl(dst, src);
  return off;
}

int MacroAssembler::load_signed_word(Register dst, Address src)
{
  int off = offset();
  movswl(dst, src);
  return off;
}

void MacroAssembler::incrementl(Register reg, int value)
{
  if (value <  0) { decrementl(reg, -value); return; }
  if (value == 0) {                        ; return; }
  if (value == 1) { incl(reg)              ; return; }
  /* else */      { addl(reg, value)       ; return; }
}

void MacroAssembler::decrementl(Register reg, int value)
{
  if (value <  0) { incrementl(reg, -value); return; }
  if (value == 0) {                        ; return; }
  if (value == 1) { decl(reg)              ; return; }
  /* else */      { subl(reg, value)       ; return; }
}

void MacroAssembler::incrementq(Register reg, int value)
{
  if (value <  0) { decrementq(reg, -value); return; }
  if (value == 0) {                        ; return; }
  if (value == 1) { incq(reg)              ; return; }
  /* else */      { addq(reg, value)       ; return; }
}

void MacroAssembler::decrementq(Register reg, int value)
{
  if (value <  0) { incrementq(reg, -value); return; }
  if (value == 0) {                        ; return; }
  if (value == 1) { decq(reg)              ; return; }
  /* else */      { subq(reg, value)       ; return; }
}

void MacroAssembler::align(int modulus)
{
  while (offset() % modulus != 0) {
    nop();
  }
}

void MacroAssembler::enter()
{
  pushq(rbp);
  movq(rbp, rsp);
}

void MacroAssembler::leave()
{
  emit_byte(0xC9); // LEAVE
}


void MacroAssembler::set_last_Java_frame(Register java_thread,
                                         Register last_java_sp,
                                         Register last_java_fp,
                                         address  last_java_pc)
{
  // determine last_java_sp register
  if (!last_java_sp->is_valid()) {
    last_java_sp = rsp;
  }

  // last_java_fp is optional
  if (last_java_fp->is_valid()) {
    movq(Address(r15_thread, JavaThread::last_Java_fp_offset()),
         last_java_fp);
  }

  // last_java_pc is optional
  if (last_java_pc != NULL) {
    movq(Address(r15_thread,
                 JavaThread::frame_anchor_offset() +
                 JavaFrameAnchor::last_Java_pc_offset()),
         (int64_t) last_java_pc);  
  }

#ifdef ASSERT
  {
    Label L;
    cmpl(Address(r15_thread,
		 JavaThread::frame_anchor_offset() +
                 JavaFrameAnchor::flags_offset()),
         0);
    jcc(Assembler::equal, L);
    stop("MacroAssembler::set_last_Java_frame: flags not cleared");
    bind(L);
  }
#endif // ASSERT
  movq(Address(r15_thread, JavaThread::last_Java_sp_offset()), last_java_sp);
}

void MacroAssembler::reset_last_Java_frame(Register java_thread,
                                           bool clear_pc)
{
  // we must set sp to zero to clear frame
  movq(Address(r15_thread, JavaThread::last_Java_sp_offset()), 0);
  // must clear fp, so that compiled frames are not confused; it is
  // possible that we need it only for debugging
  movq(Address(r15_thread, JavaThread::last_Java_fp_offset()), 0);

  if (clear_pc) {
    movq(Address(r15_thread, JavaThread::last_Java_pc_offset()), 0);
  }

  movl(Address(r15_thread,
               JavaThread::frame_anchor_offset() +
               JavaFrameAnchor::flags_offset()),
       0);
}


// Implementation of call_VM versions

void MacroAssembler::call_VM_leaf_base(address entry_point, int num_args)
{
  Label L, E;

#ifdef _WIN64
    // Windows always allocates space for it's register args
    subq(rsp, (num_args+1)*wordSize);
#endif

  // Align stack if necessary
  testl(rsp, 15);
  jcc(Assembler::zero, L);

  subq(rsp, 8);
  call(entry_point, relocInfo::runtime_call_type);
  addq(rsp, 8);
  jmp(E);

  bind(L);
  call(entry_point, relocInfo::runtime_call_type);

  bind(E);

#ifdef _WIN64
    // restore stack pointer
    addq(rsp, (num_args+1)*wordSize);
#endif

}


void MacroAssembler::call_VM_base(Register oop_result,
                                  Register java_thread,
                                  Register last_java_sp,
                                  address entry_point,
                                  int num_args,
                                  bool check_exceptions)
{
  // determine last_java_sp register
  if (!last_java_sp->is_valid()) {
    last_java_sp = rsp;
  }

  // debugging support
  assert(num_args >= 0, "cannot have negative number of arguments");
  assert(r15_thread != oop_result,
         "cannot use the same register for java_thread & oop_result");
  assert(r15_thread != last_java_sp,
         "cannot use the same register for java_thread & last_java_sp");

  // set last Java frame before call
  assert(last_java_sp != rbp, 
         "this code doesn't work for last_java_sp == rbp, which currently "
         "can't portably work anyway since C2 doesn't save rbp");
  movq(Address(r15_thread, JavaThread::last_Java_fp_offset()), rbp);
#ifdef ASSERT
  {
    Label L;
    cmpl(Address(r15_thread,
                 JavaThread::frame_anchor_offset() +
                 JavaFrameAnchor::flags_offset()),
         0);
    jcc(Assembler::equal, L);
    stop("MacroAssembler::call_VM_base: flags not cleared");
    bind(L);
  }
#endif /* ASSERT */
  movq(Address(r15_thread, JavaThread::last_Java_sp_offset()), last_java_sp);

  {
    Label L, E;

    // Align stack if necessary
#ifdef _WIN64
    // Windows always allocates space for it's register args
    subq(rsp, (num_args+1)*wordSize);
#endif
    testl(rsp, 15);
    jcc(Assembler::zero, L);

    subq(rsp, 8);
    call(entry_point, relocInfo::runtime_call_type);
    addq(rsp, 8);
    jmp(E);

    bind(L);
    call(entry_point, relocInfo::runtime_call_type);

    bind(E);

#ifdef _WIN64
    // restore stack pointer
    addq(rsp, (num_args+1)*wordSize);
#endif
  }

#ifdef ASSERT
  pushq(rax);
  {
    Label L;
    get_thread(rax);
    cmpq(r15_thread, rax);
    jcc(Assembler::equal, L);
    stop("MacroAssembler::call_VM_base: register not callee saved?");
    bind(L);
  }
  popq(rax);
#endif

  // reset last Java frame
  // we must set sp to zero to clear frame
  movq(Address(r15_thread, JavaThread::last_Java_sp_offset()), 0);

  // must clear fp, so that compiled frames are not confused; it is possible
  // that we need it only for debugging
  movq(Address(r15_thread, JavaThread::last_Java_fp_offset()), 0);
  movl(Address(r15_thread,
	       JavaThread::frame_anchor_offset() +
               JavaFrameAnchor::flags_offset()),
       0);

  check_and_handle_popframe(noreg);

  if (check_exceptions) {
    cmpq(Address(r15_thread, Thread::pending_exception_offset()), (int) NULL);
    jcc(Assembler::notEqual, 
        StubRoutines::forward_exception_entry(), 
        relocInfo::runtime_call_type);
  }

  // get oop result if there is one and reset the value in the thread
  if (oop_result->is_valid()) {
    movq(oop_result, Address(r15_thread, JavaThread::vm_result_offset()));
    movq(Address(r15_thread, JavaThread::vm_result_offset()), (int) NULL);
    verify_oop(oop_result);
  }
}

void MacroAssembler::check_and_handle_popframe(Register java_thread) {}

void MacroAssembler::call_VM_helper(Register oop_result,
                                    address entry_point,
                                    int num_args,
                                    bool check_exceptions)
{
  // Java thread becomes first argument of C function
  movq(rarg0, r15_thread);

  // We've pushed one address, correct last_Java_sp
  leaq(rax, Address(rsp, wordSize));

  call_VM_base(oop_result, noreg, rax, entry_point, num_args,
               check_exceptions);
}


void MacroAssembler::call_VM(Register oop_result,
                             address entry_point,
                             bool check_exceptions) 
{
  Label C, E;
  Assembler::call(C, relocInfo::none);
  jmp(E);

  bind(C);
  call_VM_helper(oop_result, entry_point, 0, check_exceptions);
  ret(0);

  bind(E);
}


void MacroAssembler::call_VM(Register oop_result,
                             address entry_point,
                             Register arg_1,
                             bool check_exceptions) 
{
  assert(rax != arg_1, "smashed argument");
  assert(rarg0 != arg_1, "smashed argument");

  Label C, E;
  Assembler::call(C, relocInfo::none);
  jmp(E);

  bind(C);
  // rarg0 is reserved for thread
  if (rarg1 != arg_1) {
    movq(rarg1, arg_1);
  }
  call_VM_helper(oop_result, entry_point, 1, check_exceptions);
  ret(0);

  bind(E);
}

void MacroAssembler::call_VM(Register oop_result,
                             address entry_point,
                             Register arg_1,
                             Register arg_2,
                             bool check_exceptions) 
{
  assert(rax != arg_1, "smashed argument");
  assert(rax != arg_2, "smashed argument");
  assert(rarg0 != arg_1, "smashed argument");
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg1 != arg_2, "smashed argument");
  assert(rarg2 != arg_1, "smashed argument");

  Label C, E;
  Assembler::call(C, relocInfo::none);
  jmp(E);

  bind(C);
  // rarg0 is reserved for thread
  if (rarg1 != arg_1) {
    movq(rarg1, arg_1);
  }
  if (rarg2 != arg_2) {
    movq(rarg2, arg_2);
  }
  call_VM_helper(oop_result, entry_point, 2, check_exceptions);
  ret(0);

  bind(E);
}


void MacroAssembler::call_VM(Register oop_result,
                             address entry_point,
                             Register arg_1,
                             Register arg_2,
                             Register arg_3,
                             bool check_exceptions) 
{
  assert(rax != arg_1, "smashed argument");
  assert(rax != arg_2, "smashed argument");
  assert(rax != arg_3, "smashed argument");
  assert(rarg0 != arg_1, "smashed argument");
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg0 != arg_3, "smashed argument");
  assert(rarg1 != arg_2, "smashed argument");
  assert(rarg1 != arg_3, "smashed argument");
  assert(rarg2 != arg_1, "smashed argument");
  assert(rarg2 != arg_3, "smashed argument");
  assert(rarg3 != arg_1, "smashed argument");
  assert(rarg3 != arg_2, "smashed argument");

  Label C, E;
  Assembler::call(C, relocInfo::none);
  jmp(E);

  bind(C);
  // rarg0 is reserved for thread
  if (rarg1 != arg_1) {
    movq(rarg1, arg_1);
  }
  if (rarg2 != arg_2) {
    movq(rarg2, arg_2);
  }
  if (rarg3 != arg_3) {
    movq(rarg3, arg_3);
  }
  call_VM_helper(oop_result, entry_point, 3, check_exceptions);
  ret(0);

  bind(E);
}

void MacroAssembler::call_VM(Register oop_result,
                             Register last_java_sp, 
                             address entry_point,
                             int num_args,
                             bool check_exceptions) 
{
  call_VM_base(oop_result, noreg, last_java_sp, entry_point, num_args,
               check_exceptions);
}

void MacroAssembler::call_VM(Register oop_result, 
                             Register last_java_sp,
                             address entry_point, 
                             Register arg_1, 
                             bool check_exceptions) 
{
  assert(rarg0 != arg_1, "smashed argument");
  assert(rarg1 != last_java_sp, "smashed argument");
  // rarg0 is reserved for thread
  if (rarg1 != arg_1) {
    movq(rarg1, arg_1);
  }
  call_VM(oop_result, last_java_sp, entry_point, 1, check_exceptions);
}

void MacroAssembler::call_VM(Register oop_result, 
                             Register last_java_sp,
                             address entry_point,
                             Register arg_1,
                             Register arg_2,
                             bool check_exceptions)
{
  assert(rarg0 != arg_1, "smashed argument");
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg1 != arg_2, "smashed argument");
  assert(rarg1 != last_java_sp, "smashed argument");
  assert(rarg2 != arg_1, "smashed argument");
  assert(rarg2 != last_java_sp, "smashed argument");
  // rarg0 is reserved for thread
  if (rarg1 != arg_1) {
    movq(rarg1, arg_1);
  }
  if (rarg2 != arg_2) {
    movq(rarg2, arg_2);
  }
  call_VM(oop_result, last_java_sp, entry_point, 2, check_exceptions);
}


void MacroAssembler::call_VM(Register oop_result,
                             Register last_java_sp,
                             address entry_point,
                             Register arg_1,
                             Register arg_2,
                             Register arg_3,
                             bool check_exceptions)
{
  assert(rarg0 != arg_1, "smashed argument");
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg0 != arg_3, "smashed argument");
  assert(rarg1 != arg_2, "smashed argument");
  assert(rarg1 != arg_3, "smashed argument");
  assert(rarg1 != last_java_sp, "smashed argument");
  assert(rarg2 != arg_1, "smashed argument");
  assert(rarg2 != arg_3, "smashed argument");
  assert(rarg2 != last_java_sp, "smashed argument");
  assert(rarg3 != arg_1, "smashed argument");
  assert(rarg3 != arg_2, "smashed argument");
  assert(rarg3 != last_java_sp, "smashed argument");
  // rarg0 is reserved for thread
  if (rarg1 != arg_1) {
    movq(rarg1, arg_1);
  }
  if (rarg2 != arg_2) {
    movq(rarg2, arg_2);
  }
  if (rarg3 != arg_3) {
    movq(rarg2, arg_3);
  }
  call_VM(oop_result, last_java_sp, entry_point, 3, check_exceptions);
}

void MacroAssembler::call_VM_leaf(address entry_point, int num_args)
{
  call_VM_leaf_base(entry_point, num_args);
}

void MacroAssembler::call_VM_leaf(address entry_point, Register arg_1)
{
  if (rarg0 != arg_1) {
    movq(rarg0, arg_1);
  }
  call_VM_leaf(entry_point, 1);
}

void MacroAssembler::call_VM_leaf(address entry_point,
                                  Register arg_1, 
                                  Register arg_2) 
{
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg1 != arg_1, "smashed argument");
  if (rarg0 != arg_1) {
    movq(rarg0, arg_1);
  }
  if (rarg1 != arg_2) {
    movq(rarg1, arg_2);
  }
  call_VM_leaf(entry_point, 2);
}

void MacroAssembler::call_VM_leaf(address entry_point,
                                  Register arg_1,
                                  Register arg_2,
                                  Register arg_3)
{
  assert(rarg0 != arg_2, "smashed argument");
  assert(rarg0 != arg_3, "smashed argument");
  assert(rarg1 != arg_1, "smashed argument");
  assert(rarg1 != arg_3, "smashed argument");
  assert(rarg2 != arg_1, "smashed argument");
  assert(rarg2 != arg_2, "smashed argument");
  if (rarg0 != arg_1) {
    movq(rarg0, arg_1);
  }
  if (rarg1 != arg_2) {
    movq(rarg1, arg_2);
  }
  if (rarg2 != arg_3) {
    movq(rarg2, arg_3);
  }
  call_VM_leaf(entry_point, 3);
}


// Calls to C land
//
// When entering C land, the rbp & rsp of the last Java frame have to
// be recorded in the (thread-local) JavaThread object. When leaving C
// land, the last Java fp has to be reset to 0. This is required to
// allow proper stack traversal.
void MacroAssembler::store_check(Register obj) 
{
  // Does a store check for the oop in register obj. The content of
  // register obj is destroyed afterwards.
  store_check_part_1(obj);
  store_check_part_2(obj);
}

void MacroAssembler::store_check(Register obj, Address dst) 
{
  store_check(obj);
}

// split the store check operation so that other instructions can be
// scheduled inbetween
void MacroAssembler::store_check_part_1(Register obj) 
{
  BarrierSet* bs = Universe::heap()->barrier_set();
  assert(bs->kind() == BarrierSet::CardTableModRef, "Wrong barrier set kind");
  shrq(obj, CardTableModRefBS::card_shift);
}

void MacroAssembler::store_check_part_2(Register obj) 
{
  BarrierSet* bs = Universe::heap()->barrier_set();
  assert(bs->kind() == BarrierSet::CardTableModRef, "Wrong barrier set kind");
  CardTableModRefBS* ct = (CardTableModRefBS*)bs;
  assert(sizeof(*ct->byte_map_base) == sizeof(jbyte), "adjust this code");
  movq(r10, (int64_t) ct->byte_map_base);
  movb(Address(r10, obj, Address::times_1), 0);
}

void MacroAssembler::c2bool(Register x) 
{
  // implements x == 0 ? 0 : 1
  // note: must only look at least-significant byte of x
  //       since C-style booleans are stored in one byte
  //       only! (was bug)
  andl(x, 0xFF);
  setb(Assembler::notZero, x);
}

int MacroAssembler::corrected_idivl(Register reg) 
{
  // Full implementation of Java idiv and irem; checks for special
  // case as described in JVM spec., p.243 & p.271.  The function
  // returns the (pc) offset of the idivl instruction - may be needed
  // for implicit exceptions.
  //
  //         normal case                           special case
  //
  // input : eax: dividend                         min_int
  //         reg: divisor   (may not be eax/edx)   -1
  //
  // output: eax: quotient  (= eax idiv reg)       min_int
  //         edx: remainder (= eax irem reg)       0
  assert(reg != rax && reg != rdx, "reg cannot be rax or rdx register");
  const int min_int = 0x80000000;
  Label normal_case, special_case;
  
  // check for special case
  cmpl(rax, min_int);
  jcc(Assembler::notEqual, normal_case);
  xorl(rdx, rdx); // prepare edx for possible special case (where
                  // remainder = 0)
  cmpl(reg, -1);
  jcc(Assembler::equal, special_case);

  // handle normal case
  bind(normal_case);
  cdql();
  int idivl_offset = offset();
  idivl(reg);

  // normal and special case exit
  bind(special_case);

  return idivl_offset;
}

int MacroAssembler::corrected_idivq(Register reg) 
{
  // Full implementation of Java ldiv and lrem; checks for special
  // case as described in JVM spec., p.243 & p.271.  The function
  // returns the (pc) offset of the idivl instruction - may be needed
  // for implicit exceptions.
  //
  //         normal case                           special case
  //
  // input : rax: dividend                         min_long
  //         reg: divisor   (may not be eax/edx)   -1
  //
  // output: rax: quotient  (= rax idiv reg)       min_long
  //         rdx: remainder (= rax irem reg)       0
  assert(reg != rax && reg != rdx, "reg cannot be rax or rdx register");
  static const int64_t min_long = 0x8000000000000000;
  Label normal_case, special_case;
  
  // check for special case
  cmpq(rax, Address((address) &min_long, relocInfo::none));
  jcc(Assembler::notEqual, normal_case);
  xorl(rdx, rdx); // prepare rdx for possible special case (where
                  // remainder = 0)
  cmpq(reg, -1);
  jcc(Assembler::equal, special_case);

  // handle normal case
  bind(normal_case);
  cdqq();
  int idivq_offset = offset();
  idivq(reg);

  // normal and special case exit
  bind(special_case);

  return idivq_offset;
}

void MacroAssembler::push_IU_state() {
  pushfq();     // Push flags first because pushaq kills them
  subq(rsp, 8); // Make sure rsp stays 16-byte aligned
  pushaq();
}

void MacroAssembler::pop_IU_state() {
  popaq();
  addq(rsp, 8);
  popfq();
}

void MacroAssembler::push_FPU_state() {
  subq(rsp, FPUStateSizeInWords * wordSize);
  fxsave(Address(rsp));
}

void MacroAssembler::pop_FPU_state() {
  fxrstor(Address(rsp));
  addq(rsp, FPUStateSizeInWords * wordSize);
}

void MacroAssembler::push_CPU_state() {
  push_IU_state();
  push_FPU_state();
}

void MacroAssembler::pop_CPU_state() {
  pop_FPU_state();
  pop_IU_state();
}

void MacroAssembler::sign_extend_short(Register reg) 
{
  movswl(reg, reg);
}

void MacroAssembler::sign_extend_byte(Register reg) 
{
  movsbl(reg, reg);
}

void MacroAssembler::division_with_shift(Register reg, int shift_value) 
{
  assert (shift_value > 0, "illegal shift value");
  Label _is_positive;
  testl (reg, reg);
  jcc (Assembler::positive, _is_positive);
  int offset = (1 << shift_value) - 1 ;

  if (offset == 1) {
    incl(reg);
  } else {
    addl(reg, offset);
  }

  bind (_is_positive);
  sarl(reg, shift_value);
}

void MacroAssembler::round_to_l(Register reg, int modulus)
{
  addl(reg, modulus - 1);
  andl(reg, -modulus);
}

void MacroAssembler::round_to_q(Register reg, int modulus)
{
  addq(reg, modulus - 1);
  andq(reg, -modulus);
}

void MacroAssembler::verify_oop(Register reg, const char* s) 
{
  if (!VerifyOops) {
    return;
  }

  // Pass register number to verify_oop_subroutine
  char* b = new char[strlen(s) + 50];
  sprintf(b, "verify_oop: %s: %s", reg->name(), s);

  pushq(rax); // save rax, restored by receiver

  // pass args on stack, only touch rax
  pushq(reg);
  pushq(Address((address) b, relocInfo::none));

  // call indirectly to solve generation ordering problem
  movq(rax, (int64_t) StubRoutines::verify_oop_subroutine_entry_address());
  Assembler::call(rax); // no alignment requirement
  // everything popped by receiver
}

void MacroAssembler::stop(const char* msg) 
{
  address rip = pc();
  pushaq(); // get regs on stack
  movq(rarg0, (int64_t) msg);
  leaq(rarg1, Address(rip, relocInfo::none)); // get pc
  movq(rarg2, rsp); // pass pointer to regs array
  andq(rsp, -16); // align stack as required by ABI
  call(CAST_FROM_FN_PTR(address, MacroAssembler::debug),
       relocInfo::runtime_call_type);
  hlt();
}


void MacroAssembler::debug(char* msg, int64_t pc, int64_t regs[])
{
  // In order to get locks to work, we need to fake a in_VM state
  if (ShowMessageBoxOnError) {
    JavaThread* thread = JavaThread::current();
    JavaThreadState saved_state = thread->thread_state();
    thread->set_thread_state(_thread_in_vm);
    ttyLocker ttyl;
#ifndef PRODUCT
    if (CountBytecodes || TraceBytecodes || StopInterpreterAt) {
      BytecodeCounter::print();
    }
#endif
    // To see where a verify_oop failed, get $ebx+40/X for this frame.
    // XXX correct this offset for amd64
    // This is the value of eip which points to where verify_oop will return.
    if (os::message_box(msg, "Execution stopped, print registers?")) {
      tty->print_cr("rip = 0x%016lx", pc);
      tty->print_cr("rax = 0x%016lx", regs[15]);
      tty->print_cr("rbx = 0x%016lx", regs[12]);
      tty->print_cr("rcx = 0x%016lx", regs[14]);
      tty->print_cr("rdx = 0x%016lx", regs[13]);
      tty->print_cr("rdi = 0x%016lx", regs[8]);
      tty->print_cr("rsi = 0x%016lx", regs[9]);
      tty->print_cr("rbp = 0x%016lx", regs[10]);
      tty->print_cr("rsp = 0x%016lx", regs[11]);
      tty->print_cr("r8  = 0x%016lx", regs[7]);
      tty->print_cr("r9  = 0x%016lx", regs[6]);
      tty->print_cr("r10 = 0x%016lx", regs[5]);
      tty->print_cr("r11 = 0x%016lx", regs[4]);
      tty->print_cr("r12 = 0x%016lx", regs[3]);
      tty->print_cr("r13 = 0x%016lx", regs[2]);
      tty->print_cr("r14 = 0x%016lx", regs[1]);
      tty->print_cr("r15 = 0x%016lx", regs[0]);
      BREAKPOINT;
    }
    ThreadStateTransition::transition(thread, _thread_in_vm, saved_state);
  } else {
    ::tty->print_cr("=============== DEBUG MESSAGE: %s ================\n",
                    msg);
  }
}

void MacroAssembler::os_breakpoint() 
{
  // instead of directly emitting a breakpoint, call os:breakpoint for
  // better debugability
  // This shouldn't need alignment, it's an empty function
  call(CAST_FROM_FN_PTR(address, os::breakpoint), 
       relocInfo::runtime_call_type);
}

void MacroAssembler::verify_tlab()
{
#ifdef ASSERT
  if (UseTLAB) {
    Label next, ok;
    Register t1 = rarg1;
    
    pushq(t1);

    movq(t1, Address(r15_thread, in_bytes(JavaThread::tlab_top_offset())));
    cmpq(t1, Address(r15_thread, in_bytes(JavaThread::tlab_start_offset())));
    jcc(Assembler::aboveEqual, next);
    stop("assert(top >= start)");
    should_not_reach_here();

    bind(next);
    movq(t1, Address(r15_thread, in_bytes(JavaThread::tlab_end_offset())));
    cmpq(t1, Address(r15_thread, in_bytes(JavaThread::tlab_top_offset())));
    jcc(Assembler::aboveEqual, ok);
    stop("assert(top <= end)");
    should_not_reach_here();

    bind(ok);

    popq(t1);
  }
#endif 
}

// Defines obj, preserves var_size_in_bytes
void MacroAssembler::eden_allocate(Register obj,
                                   Register var_size_in_bytes,
                                   int con_size_in_bytes,
                                   Register t1,
                                   Label& slow_case)
{
  assert(obj == rax, "obj must be in rax for cmpxchg");
  assert_different_registers(obj, var_size_in_bytes, t1);
  Register end = t1;
  Label retry;
  bind(retry);
  movq(obj, Address((address) Universe::heap()->top_addr(), relocInfo::none));
  if (var_size_in_bytes == noreg) {
    leaq(end, Address(obj, con_size_in_bytes));
  } else {
    leaq(end, Address(obj, var_size_in_bytes, Address::times_1));
  }
  // if end < obj then we wrapped around => object too long => slow case
  cmpq(end, obj);
  jcc(Assembler::below, slow_case);
  cmpq(end, Address((address) Universe::heap()->end_addr(), relocInfo::none));
  jcc(Assembler::above, slow_case);
  // Compare obj with the top addr, and if still equal, store the new
  // top addr in end at the address of the top addr pointer. Sets ZF
  // if was equal, and clears it otherwise. Use lock prefix for
  // atomicity on MPs.
  if (os::is_MP()) {
    lock();
  }
  cmpxchgq(end, 
           Address((address) Universe::heap()->top_addr(), relocInfo::none));
  // if someone beat us on the allocation, try again, otherwise continue 
  jcc(Assembler::notEqual, retry);
}

// Defines obj, preserves var_size_in_bytes, okay for t2 == var_size_in_bytes.
void MacroAssembler::tlab_allocate(Register obj,
                                   Register var_size_in_bytes,
                                   int con_size_in_bytes,
                                   Register t1,
                                   Register t2,
                                   Label& slow_case)
{
  assert_different_registers(obj, t1, t2);
  assert_different_registers(obj, var_size_in_bytes, t1);
  Register end = t2;

  verify_tlab();

  movq(obj, Address(r15_thread, JavaThread::tlab_top_offset()));
  if (var_size_in_bytes == noreg) {
    leaq(end, Address(obj, con_size_in_bytes));
  } else {
    leaq(end, Address(obj, var_size_in_bytes, Address::times_1));
  }
  cmpq(end, Address(r15_thread, JavaThread::tlab_end_offset()));
  jcc(Assembler::above, slow_case);

  // update the tlab top pointer
  movq(Address(r15_thread, JavaThread::tlab_top_offset()), end);

  // recover var_size_in_bytes if necessary
  if (var_size_in_bytes == end) {
    subq(var_size_in_bytes, obj);
  }
  verify_tlab();
}

// Preserves rbx and rdx.
void MacroAssembler::tlab_refill(Label& retry,
                                 Label& try_eden,
                                 Label& slow_case)
{
  Register top = rax;
  Register t1 = rarg3;
  Register t2 = rarg1;
  Register t3 = r10;
  Register thread_reg = r15_thread;
  assert_different_registers(top, thread_reg, t1, t2, t3,
                             /* preserve: */ rbx, rdx);
  Label do_refill, discard_tlab;

  if (CMSIncrementalMode) {
    // No allocation in the shared eden.
    jmp(slow_case);
  }

  movq(top, Address(thread_reg, in_bytes(JavaThread::tlab_top_offset())));
  movq(t1, Address(thread_reg, in_bytes(JavaThread::tlab_end_offset())));

  // calculate amount of free space
  subq(t1, top);
  shrq(t1, LogHeapWordSize);

  // Retain tlab and allocate object in shared space if
  // the amount free in the tlab is too large to discard.
  cmpq(t1, Address(thread_reg, // size_t
                   in_bytes(JavaThread::tlab_refill_waste_limit_offset())));
  jcc(Assembler::lessEqual, discard_tlab);

  // Retain
  movq(t2, ThreadLocalAllocBuffer::refill_waste_limit_increment());
  addq(Address(thread_reg,  // size_t
               in_bytes(JavaThread::tlab_refill_waste_limit_offset())),
       t2);
  if (TLABStats) {
    // increment number of slow_allocations
    addl(Address(thread_reg, // unsigned int
                 in_bytes(JavaThread::tlab_slow_allocations_offset())),
         1);
  }  
  jmp(try_eden);
 
  bind(discard_tlab);
  if (TLABStats) {
    // increment number of refills
    addl(Address(thread_reg, // unsigned int
                 in_bytes(JavaThread::tlab_number_of_refills_offset())),
         1);
    // accumulate wastage -- t1 is amount free in tlab
    addl(Address(thread_reg, // unsigned int
                 in_bytes(JavaThread::tlab_fast_refill_waste_offset())),
         t1);
  }

  // if tlab is currently allocated (top or end != null) then
  // fill [top, end + alignment_reserve) with array object
  testq(top, top);
  jcc(Assembler::zero, do_refill);

  // set up the mark word
  movq(t3, (int64_t) markOopDesc::prototype()->copy_set_hash(0x2));
  movq(Address(top, oopDesc::mark_offset_in_bytes()), t3);
  // set the length to the remaining space
  subq(t1, typeArrayOopDesc::header_size(T_INT));
  addq(t1, (int)ThreadLocalAllocBuffer::alignment_reserve());
  shlq(t1, log2_intptr(HeapWordSize / sizeof(jint)));
  movq(Address(top, arrayOopDesc::length_offset_in_bytes()), t1);
  // set klass to intArrayKlass
  movq(t1,
       Address((address) Universe::intArrayKlassObj_addr(), relocInfo::none));
  movq(Address(top, oopDesc::klass_offset_in_bytes()), t1);

  // refill the tlab with an eden allocation
  bind(do_refill);
  movq(t1, Address(thread_reg, in_bytes(JavaThread::tlab_size_offset())));
  shlq(t1, LogHeapWordSize);
  // add object_size ??
  eden_allocate(top, t1, 0, t2, slow_case);

  // Check that t1 was preserved in eden_allocate.
#ifdef ASSERT
  if (UseTLAB) {
    Label ok;
    Register tsize = rarg1;
    assert_different_registers(tsize, thread_reg, t1);
    pushq(tsize);
    movq(tsize, Address(thread_reg, in_bytes(JavaThread::tlab_size_offset())));
    shlq(tsize, LogHeapWordSize);
    cmpq(t1, tsize);
    jcc(Assembler::equal, ok);
    stop("assert(t1 != tlab size)");
    should_not_reach_here();

    bind(ok);
    popq(tsize);
  }
#endif
  movq(Address(thread_reg, in_bytes(JavaThread::tlab_start_offset())), top);
  movq(Address(thread_reg, in_bytes(JavaThread::tlab_top_offset())), top);
  addq(top, t1);
  subq(top, (int)ThreadLocalAllocBuffer::alignment_reserve_in_bytes());
  movq(Address(thread_reg, in_bytes(JavaThread::tlab_end_offset())), top);
  verify_tlab();
  jmp(retry);
}
